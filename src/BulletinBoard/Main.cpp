/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright 2012 Richard Hacker (lerichi at gmx dot net)
 *
 *  This file is part of the rtipc library.
 *
 *  The rtipc library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or (at
 *  your option) any later version.
 *
 *  The rtipc library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the rtipc library. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include "../Debug.h"

#include "Main.h"
#include "Signal.h"
#include "Group.h"
#include "Flock.h"
#include "Config.h"
#include <sstream>
#include <fstream>
#include <cerrno>
#include <iostream>
#include <vector>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

using namespace BulletinBoard;

//////////////////////////////////////////////////////////////////////////////
Main::Main (const std::string &file)
{
    shmaddr = 0;
    if (!file.empty())
        load(file);
}

//////////////////////////////////////////////////////////////////////////////
Main::~Main ()
{
    clear();
}


//////////////////////////////////////////////////////////////////////////////
void Main::load (const std::string& file)
{
    lockFile = file;

    Config conf(file);
    Config::Node root(conf);

    for (Config::Node::List groups(root); groups; ++groups) {

        Config::Node::Group group(*groups);

        Group *g = addGroup(group["SampleTime"]);

        for (Config::Node::List it(group["Signals"]); it; ++it) {

            Config::Node::Group sigSpec = *it;
            newSignal(g, sigSpec["Name"], DataType(sigSpec["DataType"]),
                    static_cast<unsigned int>(sigSpec["Length"]));
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
void Main::clear ()
{
    for (Groups::iterator it = groups.begin(); it != groups.end(); it++)
        delete *it;

    groups.clear();
}

//////////////////////////////////////////////////////////////////////////////
Group* Main::addGroup (double sampleTime)
{
    Group *g = new Group(this, sampleTime);
    groups.push_back(g);
    return g;
}

//////////////////////////////////////////////////////////////////////////////
const Signal* Main::newSignal (Group *group, const std::string &name,
        const DataType& datatype, size_t n)
{
    if (signalMap[name])
        return 0;

    signalMap[name] = group->newSignal(name, datatype, n);

    return signalMap[name];
}

/////////////////////////////////////////////////////////////////////////////
const Main::SignalMap& Main::getSignalMap() const
{
    return signalMap;
}

//////////////////////////////////////////////////////////////////////////////
uint32_t Main::checkSum () const
{
    uint32_t crc = 0;

    for (Groups::const_iterator it = groups.begin();
            it != groups.end(); it++)
        crc += (*it)->checkSum();

    return crc;
}

//////////////////////////////////////////////////////////////////////////////
int Main::openSharedMemory (bool exclusive, const std::string& lf)
{
    if (shmaddr)
        return 0;

    if (!lf.empty())
        this->lockFile = lf;

    Flock lock(this->lockFile.c_str());
    int semid;

    size_t signalSize = sizeof(uint32_t) + groups.size()*sizeof(size_t);

    // Calculate size of shared memory
    for (Main::Groups::const_iterator it = groups.begin();
            it != groups.end(); ++it) {
        log_debug("group");
        signalSize += (*it)->getSignalSize();
    }

    log_debug("signalSize=%zu", signalSize);
    key_t key = ftok(this->lockFile.c_str(),1);
    if (key == -1)
        return errno;

    // First try to get the shm without creating
    int shmflg = S_IRUSR | S_IWUSR;
    shmid = shmget(key, 0, shmflg);
    if (shmid == -1 and errno == ENOENT) {
        // Shared memory does not exist, create it
        shmflg |= IPC_CREAT;
        shmid = shmget(key, signalSize, shmflg);

        // Remove possible semaphores
        semid = semget(key, 0, S_IRUSR | S_IWUSR);
        if (semid != -1)
            if (semctl(semid, 0, IPC_RMID))
                return -errno;
    }

    if (shmid == -1)
        return -errno;

    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void*)-1)
        return -errno;

    uint32_t *checksum = reinterpret_cast<uint32_t*>(shmaddr);

    uint32_t cs = checkSum();
    if (shmflg & IPC_CREAT) {
        // Write checksum
        log_debug("New IPC segment was created");
        *checksum = cs;
    }
    else if (*checksum != cs ) {
        // Shared memory existed already. Check that the checksum is valid
        log_debug("Checksum invalid %u", cs);

        // Remove the shared memory segment
        shmdt(shmaddr);
        shmctl(shmid, IPC_RMID, NULL);

        // Remove possible semaphores
        semctl(key, 0, IPC_RMID);

        // Try to attach without creating. If the result is not an error,
        // the segment was not removed successfully.
        shmid = shmget(key, 0, shmflg);
        if (shmid != -1) {
            return -EEXIST;
        }
        log_debug("Successfully removed segment");

        // Now try to create the segment
        shmflg |= IPC_CREAT;
        shmid = shmget(key, signalSize, shmflg);
        if (shmid == -1) {
            return -errno;
        }
        log_debug("Successfully created segment again");

        shmaddr = shmat(shmid, NULL, 0);
        if (shmaddr == (void*)-1) {
            return -errno;
        }
        *checksum = cs;
    }

    log_debug("size = %s %zu %i %p",
            this->lockFile.c_str(), signalSize, key, shmaddr);

    int semflg = S_IRUSR | S_IWUSR;
    semid = semget(key, 0, semflg);

    struct semid_ds semid_ds;
    if (semid == -1 or shmflg & IPC_CREAT)
        semid_ds.sem_nsems = 0;
    else if (semctl(semid, 0, IPC_STAT, &semid_ds) == -1)
        return -errno;

    log_debug("size");
    unsigned short nsems = groups.size() + 1;
    struct sembuf sop;
    if (semid_ds.sem_nsems != nsems) {
        log_debug("size");
        semctl(semid, 0, IPC_RMID);

        log_debug("size");
        semid = semget(key, 0, semflg);
        log_debug("size %i", semid);
        if (semid != -1)
            return -errno;

        log_debug("size");
        semflg |= IPC_CREAT;
        semid = semget(key, nsems, semflg);
        if (semid == -1)
            return -errno;

        log_debug("size");
        sop.sem_op = 1;
        sop.sem_flg = 0;
        for (sop.sem_num = 0; sop.sem_num < nsems; sop.sem_num++)
            log_debug("semop %i", semop(semid, &sop, 1));
    }

    size_t *counter = reinterpret_cast<size_t*>(checksum + 1);
    void *addr = counter + groups.size();
    log_debug("SemId = %i", semid);
    int i = 0;
    for (Groups::iterator it = groups.begin(); it != groups.end(); it++)
        addr = (*it)->prepareIPC(counter++, addr, semid, i++);

    // Try getting a lock when exclusive is set
    sop.sem_op = -1;
    sop.sem_num = nsems - 1;
    sop.sem_flg = SEM_UNDO | IPC_NOWAIT;
    return exclusive and semop(semid, &sop, 1) ? errno : 0;
}

//////////////////////////////////////////////////////////////////////////////
size_t Main::groupCount () const
{
    size_t n = 0;

    for (Groups::const_iterator g = groups.begin();
            g != groups.end(); g++)
        n += !(*g)->empty();

    return n;
}

//////////////////////////////////////////////////////////////////////////////
bool Main::compatible (const Main& other)
{
    typedef std::map<const Group*, Group*> GroupMap;
    GroupMap groupMap;

    /* Return unequal if there are more groups than the other group */
    if (groupCount() > other.groupCount())
        return false;

    for (Groups::const_iterator g = groups.begin();
            g != groups.end(); g++) {

        // Find a signal that belongs to this group
        for (SignalMap::const_iterator sm = signalMap.begin();
                sm != signalMap.end(); sm++) {

            if (sm->second->group != *g)
                continue;

            // OK. Here is a signal
            SignalMap::const_iterator osm = other.signalMap.find(sm->first);

            // Find this signal in the other bulletin board
            if (osm == other.signalMap.end() or **g > *osm->second->group)
                // Signal does not exist in other bulletin board
                // or the other group is smaller than this group.
                // return incompatible
                return false;

            groupMap[osm->second->group] = *g;

            break; // Next group
        }
    }

    // Now go through the other groups and order our groups and
    // signals to match the other order
    groups.clear();
    for (Groups::const_iterator og = other.groups.begin();
            og != other.groups.end(); og++) {

        GroupMap::const_iterator it = groupMap.find(*og);

        if (it == groupMap.end()) {
            groups.push_back(new Group(this, (*og)->sampleTime));
        }
        else {
            groups.push_back(it->second);
        }

        (*groups.rbegin())->copy(*og);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
bool Main::save (const std::string &file)
{
    lockFile = file;

    yaml_document_t output_document;
    yaml_emitter_t emitter;
    //int key, value, node, group, signals;
    int root;
    FILE *f = fopen(file.c_str(), "w");

    std::ostringstream os;

    ::memset(&output_document, 0, sizeof(output_document));

    if (!yaml_document_initialize(&output_document, NULL, NULL, NULL, 0, 1))
        goto document_error;

    root = yaml_document_add_sequence(&output_document, NULL,
            YAML_BLOCK_SEQUENCE_STYLE);
    if (!root)
        goto document_error;

//    for (Groups::const_iterator it = groups.begin();
//            it != groups.end(); it++) {
//        const Group *g = *it;
//        //const Group::Signals& groupSignals = g->getSignals();
//        const Group::Signals groupSignals;
//
//        if (groupSignals.empty())
//            continue;
//
//        group = yaml_document_add_mapping(&output_document, NULL,
//                YAML_BLOCK_MAPPING_STYLE);
//        if (!yaml_document_append_sequence_item(&output_document, root, group))
//            goto document_error;
//
//        key = yaml_document_add_scalar(&output_document, NULL,
//                (yaml_char_t*)"GroupName", -1, YAML_PLAIN_SCALAR_STYLE);
//        value = yaml_document_add_scalar(&output_document, NULL,
//                (yaml_char_t*)"group", -1,
//                YAML_DOUBLE_QUOTED_SCALAR_STYLE);
//        if (!key or !value
//                or !yaml_document_append_mapping_pair(&output_document,
//                    group, key, value))
//            goto document_error;
//
//        os.str(std::string());
//        os << g->sampleTime;
//        key = yaml_document_add_scalar(&output_document, NULL,
//                (yaml_char_t*)"SampleTime", -1, YAML_PLAIN_SCALAR_STYLE);
//        value = yaml_document_add_scalar(&output_document, NULL,
//                (yaml_char_t*)os.str().c_str(), -1,
//                YAML_PLAIN_SCALAR_STYLE);
//        if (!key or !value
//                or !yaml_document_append_mapping_pair(&output_document,
//                    group, key, value))
//            goto document_error;
//
//        key = yaml_document_add_scalar(&output_document, NULL,
//                (yaml_char_t*)"Signals", -1, YAML_PLAIN_SCALAR_STYLE);
//        signals = yaml_document_add_sequence(&output_document, NULL,
//                YAML_BLOCK_SEQUENCE_STYLE);
//        if (!key or !signals
//                or !yaml_document_append_mapping_pair(&output_document,
//                    group, key, signals))
//            goto document_error;
//
//        for (Group::Signals::const_iterator it = groupSignals.begin();
//                it != groupSignals.end(); it++) {
//            const Signal *s = *it;
//
//            node = yaml_document_add_mapping(&output_document, NULL,
//                    YAML_BLOCK_MAPPING_STYLE);
//            if (!node)
//                goto document_error;
//            if (!yaml_document_append_sequence_item(&output_document, signals, node))
//                goto document_error;
//
//            key = yaml_document_add_scalar(&output_document, NULL,
//                    (yaml_char_t*)"Name", -1, YAML_PLAIN_SCALAR_STYLE);
//            value = yaml_document_add_scalar(&output_document, NULL,
//                    (yaml_char_t*)s->name.c_str(), -1,
//                    YAML_DOUBLE_QUOTED_SCALAR_STYLE);
//            if (!key or !value
//                    or !yaml_document_append_mapping_pair(&output_document,
//                        node, key, value))
//                goto document_error;
//
//            key = yaml_document_add_scalar(&output_document, NULL,
//                    (yaml_char_t*)"DataType", -1, YAML_PLAIN_SCALAR_STYLE);
//            value = yaml_document_add_scalar(&output_document, NULL,
//                    (yaml_char_t*)s->dataType.c_str(), -1, YAML_PLAIN_SCALAR_STYLE);
//            yaml_document_append_mapping_pair(&output_document,
//                    node, key, value);
//
//            os.str(std::string());
//            os << s->elementCount;
//            key = yaml_document_add_scalar(&output_document, NULL,
//                    (yaml_char_t*)"Length", -1, YAML_PLAIN_SCALAR_STYLE);
//            value = yaml_document_add_scalar(&output_document, NULL,
//                    (yaml_char_t*)os.str().c_str(), -1, YAML_PLAIN_SCALAR_STYLE);
//            yaml_document_append_mapping_pair(&output_document,
//                    node, key, value);
//
////            os.str(std::string());
////            os << s->memOffset;
////            key = yaml_document_add_scalar(&output_document, NULL,
////                    (yaml_char_t*)"Offset", -1, YAML_PLAIN_SCALAR_STYLE);
////            value = yaml_document_add_scalar(&output_document, NULL,
////                    (yaml_char_t*)os.str().c_str(), -1, YAML_PLAIN_SCALAR_STYLE);
////            yaml_document_append_mapping_pair(&output_document,
////                    node, key, value);
//        }
//    }

    ::memset(&emitter, 0, sizeof(emitter));

    if (!yaml_emitter_initialize(&emitter))
        goto emitter_error;
    yaml_emitter_set_output_file(&emitter, f);
    yaml_emitter_set_canonical(&emitter, 0);
    yaml_emitter_set_unicode(&emitter, 1);

    if (!yaml_emitter_open(&emitter))
        goto emitter_error;

    if (!yaml_emitter_dump(&emitter, &output_document))
        goto emitter_error;
    if (!yaml_emitter_close(&emitter))
        goto emitter_error;

    yaml_emitter_delete(&emitter);
    yaml_document_delete(&output_document);

    fclose(f);

    return false;

document_error:
emitter_error:
    log_debug("doc ero");
    return -1;
}
