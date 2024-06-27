/*****************************************************************************
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

#include "Debug.h"

#include "Group.h"
#include "Main.h"
#include "RxPdo.h"
#include <cerrno>

using namespace RtIPC;

/////////////////////////////////////////////////////////////////////////////
Group::Group (Main *main, BulletinBoard::Group *group):
    main(main), bbGroup(group)
{
    copy_list = 0;
    disconnectedCount = 0;

    log_notice() << "New group" << this << "to main" << main
        << "Ts" << log_space('=') << bbGroup->sampleTime;
}

/////////////////////////////////////////////////////////////////////////////
Group::~Group ()
{
    for (RxPdoChunk::iterator it = rxPdoChunk.begin();
            it != rxPdoChunk.end(); it++) {
        delete (*it)->copy_list;
        delete *it;
    }

    for (RxPdoList::iterator it = rxPdo.begin(); it != rxPdo.end(); it++)
        delete *it;

    delete copy_list;

    log_notice() << "Finished group" << this;
}

/////////////////////////////////////////////////////////////////////////////
const BB::Signal* Group::addTxPdo (const std::string &name,
        const BB::DataType& datatype, const void *addr, size_t n)
{
    const BB::Signal *s = main->newSignal(bbGroup, name, datatype, n);

    if (!s)
        return 0;

    txPdoData[s] = addr;

    log_notice() << "Added TxPdo" << name
        << datatype.c_str() << log_space('[') << n << log_space(']')
        << " to" << this;
    return s;
}

////////////////////////////////////////////////////////////////////////////
const RxPdo* Group::addRxPdo (const std::string &name,
        const BB::DataType& datatype, void *addr, size_t n,
        unsigned char *connected)
{
    RxPdo *pdo = new RxPdo(this, name, datatype, n, addr, connected);
    rxPdo.push_back(pdo);

    pdo->shmemAddr = 0;
    pdo->copyListSrcPtr = 0;

    disconnectedCount++;

    log_notice() << "Added RxPdo" << name
        << pdo->dataType.c_str() << log_space('[') << n << log_space(']')
        << " to" << this;

    return pdo;
}

////////////////////////////////////////////////////////////////////////////
void Group::setupTx ()
{
    bbGroup->setupTx(txPdoData);
}

////////////////////////////////////////////////////////////////////////////
bool Group::setupRx (BB::Main *main)
{
    if (!main) {
        if (!disconnectedCount)
            return false;

        log_notice() << disconnectedCount
            << "signals not found in group" << this;

        // Finish setting up Rx. Make a local list of rxpdo's
        copy_list = new struct copy_list[disconnectedCount + 1];

        struct copy_list *cl = copy_list;
        for (RxPdoList::iterator it = rxPdo.begin(); it != rxPdo.end(); it++) {
            if (!(*it)->shmemAddr) {
                log_notice() << "    " << (*it)->name;

                cl->dst = (*it)->addr;
                cl->src = 0;
                cl->len = (*it)->size();

                (*it)->copyListSrcPtr = &cl->src;
            }
        }
        cl->dst = 0;

        return 0;
    }

    const BB::Main::SignalMap& signalMap = main->getSignalMap();

    typedef std::pair<RxPdo*, const BB::Signal*> RxPdoBuddy;
    typedef std::list<RxPdoBuddy> RxPdoBuddyList;
    typedef std::map<const BB::Group*, RxPdoBuddyList> RxPdoMap;
    RxPdoMap signals;

    for (RxPdoList::iterator it = rxPdo.begin(); it != rxPdo.end(); it++) {
        BB::Main::SignalMap::const_iterator it2 = signalMap.find((*it)->name);

        if (it2 != signalMap.end() and *it2->second == **it) {
            const BB::Signal *buddy = it2->second;
            signals[buddy->group].push_back(std::make_pair(*it,buddy));
        }
    }

    if (signals.empty())
        return false;

    log_debug() << "signals =" << signals.size() << bbGroup->sampleTime;

    main->openSharedMemory(false);

    for (RxPdoMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        size_t signalCount = it->second.size();
        ChunkData *chunkData = new ChunkData;
        unsigned char **connected = new unsigned char*[signalCount + 1];
        BB::Group::CopyList *copy_list =
            new BB::Group::CopyList[signalCount + 1];

        chunkData->group = it->first;
        chunkData->count = signalCount;
        chunkData->connected = connected;
        chunkData->copy_list = copy_list;
        chunkData->timeout = 2.0 * it->first->sampleTime / bbGroup->sampleTime;

        rxPdoChunk.push_back(chunkData);

        log_notice() << "Connecting" << signalCount
            << "signals in" << this
            << "in foreign group Ts" << log_space('=') << it->first->sampleTime;

        for (RxPdoBuddyList::const_iterator it2 = it->second.begin();
                it2 != it->second.end(); it2++) {

            log_notice() << "   " << it2->first->name;

            it2->first->copyListSrcPtr = &copy_list->src;
            it2->first->copyListConnectedPtr = connected;
            it2->first->shmemAddr = it2->second->shmemAddr;

            *connected = it2->first->connected;

            copy_list->src = it2->second->shmemAddr;
            copy_list->dst = it2->first->addr;
            copy_list->len = it2->first->size();
            log_debug()
                << it2->first->name
                << "src" << log_space('=') << copy_list->src
                << "dst" << log_space('=') << copy_list->dst
                << "len" << log_space('=') << copy_list->len;

            disconnectedCount--;
            connected++;
            copy_list++;
        }

        copy_list->src = 0;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
void Group::receive () const
{
    for (RxPdoChunk::const_iterator it = rxPdoChunk.begin();
            it != rxPdoChunk.end(); it++) {
        struct ChunkData *chunk = *it;
        size_t srcCounter = chunk->group->receive(chunk->copy_list);

        unsigned char connect;
        if (chunk->srcCounter == srcCounter) {
            connect = (int)(chunk->timer++ - srcCounter) < chunk->timeout;
        }
        else {
            chunk->srcCounter = srcCounter;
            chunk->timer = srcCounter;
            connect = 1;
        }

        unsigned char **connected = chunk->connected;
        size_t n = chunk->count;
        while (n--) {
            unsigned char *c = *connected++;
            if (c)
                *c = connect;
        }
    }

    if (!copy_list)
        return;

    for (struct copy_list *cl = copy_list; cl->dst; cl++) {
        if (cl->src)
            ::memcpy(cl->dst, cl->src, cl->len);
    }
}

////////////////////////////////////////////////////////////////////////////
void Group::setAddr (const RxPdo *pdo, const void *addr) const
{
    if (addr) {
        *pdo->copyListSrcPtr = addr;
        *pdo->connected = 1;

        if (pdo->copyListConnectedPtr)
            *pdo->copyListConnectedPtr = 0;
    }
    else {
        *pdo->copyListSrcPtr = pdo->shmemAddr;

        if (pdo->copyListConnectedPtr)
            *pdo->copyListConnectedPtr = pdo->connected;
        else
            *pdo->connected = 0;
    }
}
