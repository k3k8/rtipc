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

#include "Group.h"
#include "Signal.h"
#include "Main.h"
#include "Semaphore.h"
#include <cstring>
#include <mhash.h>

using namespace BulletinBoard;

/////////////////////////////////////////////////////////////////////////////
Group::Group (Main *main, double ts):
    main(main), sampleTime(ts)
{
    copy_list = 0;
    signalSize = 0;
    sem = 0;
    log_debug() << "New Group to main" << main << sampleTime << this;
}

/////////////////////////////////////////////////////////////////////////////
Group::~Group ()
{
    delete sem;
    delete copy_list;

    for (SignalMap::iterator it = signals.begin(); it != signals.end(); it++)
        delete it->second;
}

//////////////////////////////////////////////////////////////////////////////
const Signal* Group::newSignal (const std::string &name,
        const DataType& datatype, size_t n)
{
    Signal *s = new Signal(this, name, datatype, n);
    signals[name] = s;
    signalSize += s->size();

    log_debug() << "Added signal" << name << n << "to" << sampleTime << this;
    return s;
}

/////////////////////////////////////////////////////////////////////////////
void Group::save(YAML::Node& node) const
{
    YAML::Map d(node);
    d.append("Name", "GroupName");
    d.append("SampleTime", sampleTime);

    YAML::Sequence signals(d.appendNode("Signals", YAML::Node::Sequence));

    for (SignalMap::const_iterator it = this->signals.begin();
            it != this->signals.end(); ++it) {
        const Signal *s = it->second;
        YAML::Map map(signals.appendNode(YAML::Node::Map));

        map.append("Name", s->name, '"');
        map.append("DataType", s->dataType.c_str());
        map.append("Length", s->elementCount);
    }
}

/////////////////////////////////////////////////////////////////////////////
void Group::copy(const Group *other)
{
    for (SignalMap::const_iterator it = other->signals.begin();
            it != other->signals.end(); it++) {
        if (signals.find(it->first) == signals.end()) {
            const Signal *s = it->second;
            newSignal(s->name, s->dataType, s->elementCount);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
uint32_t Group::checkSum() const
{
    if (empty())
        return 0;

    MHASH td;

    td = mhash_init(MHASH_ADLER32);
    if (td == MHASH_FAILED)
        return -1;

    //mhash(td, &(*it)->instance, sizeof((*it)->instance));

    for (SignalMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        const Signal* s = it->second;
        mhash(td, &s->dataType, sizeof(s->dataType));
        mhash(td, s->name.c_str(), s->name.size());
        mhash(td, &s->elementCount, sizeof(s->elementCount));
    }

    uint32_t n;
    mhash_deinit(td, &n);
    
    return n;
}

/////////////////////////////////////////////////////////////////////////////
bool Group::empty () const
{
    return signals.empty();
}

/////////////////////////////////////////////////////////////////////////////
size_t Group::getSignalSize () const
{
    return signalSize;
}

//////////////////////////////////////////////////////////////////////////////
bool Group::operator> (const Group& other) const
{
    if (signals.size() > other.signals.size())
        return true;

    for (SignalMap::const_iterator it1 = signals.begin();
            it1 != signals.end(); it1++) {

        SignalMap::const_iterator it2 = other.signals.find(it1->first);

        if (it2 == other.signals.end()
                or *it1->second != *it2->second)
            return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
void *Group::prepareIPC (size_t *counter, void *addr, int semid, int instance)
{
    log_debug() << this;

    char *shmemAddr = reinterpret_cast<char*>(addr);
    sem = new Semaphore(semid, instance);

    log_debug() << sampleTime << sem;

    for (SignalMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        it->second->shmemAddr = shmemAddr;
        shmemAddr += it->second->size();
    }

    this->counter = counter;

    return shmemAddr;
}

/////////////////////////////////////////////////////////////////////////////
void Group::setAddr (const Signal *s, const void* addr) const
{
    if (s->srcAddr)
        *s->srcAddr = addr ? addr : s->src;
}

/////////////////////////////////////////////////////////////////////////////
void Group::setupTx (const PdoMap& txPdoData)
{
    if (empty())
        return;

    copy_list = new CopyList[signals.size() + 1];
    CopyList *cl = copy_list;

    for (SignalMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        Signal *s = it->second;
        PdoMap::const_iterator it2 = txPdoData.find(s);

        cl->len = s->size();
        cl->dst = s->shmemAddr;

        if (it2 != txPdoData.end()) {
            s->src = it2->second;
            s->srcAddr = &cl->src;

            cl->src = s->src;
            cl++;
        }
    }

    cl->src = 0;
}

//////////////////////////////////////////////////////////////////////////////
void Group::transmit () const
{
    //log_debug() << this << "cl=" << copy_list << "sem=" << sem;
    if (!copy_list)
        return;

    SemaphoreLock lock(sem);
    for (const CopyList *cl = copy_list; cl->src; cl++) {
        ::memcpy(cl->dst, cl->src, cl->len);
        //printf(".");
    }

    (*counter)++;
}

//////////////////////////////////////////////////////////////////////////////
size_t Group::receive (const CopyList* copy_list) const
{
    //log_debug() << this << "cl=" << copy_list << "sem=" << sem;

    SemaphoreLock lock(sem);
    for (const CopyList *cl = copy_list; cl->src; cl++) {
        ::memcpy(cl->dst, cl->src, cl->len);
        //printf("x");
    }

    //printf("\n");
    return *counter;
}
