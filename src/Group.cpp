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
}

/////////////////////////////////////////////////////////////////////////////
Group::~Group ()
{
}

/////////////////////////////////////////////////////////////////////////////
int Group::addTxPdo (const std::string &name,
        const BB::DataType& datatype, const void *addr, size_t n)
{
    const BB::Signal *s = main->newSignal(bbGroup, name, datatype, n);

    if (!s)
        return -EEXIST;

    txPdoData[s] = addr;

    return 0;
}

////////////////////////////////////////////////////////////////////////////
void Group::addRxPdo (const std::string &name,
        const BB::DataType& datatype, void *addr, size_t n,
        unsigned char *connected)
{
    rxPdo.push_back(new RxPdo(name, datatype, n, addr, connected));
}

////////////////////////////////////////////////////////////////////////////
void Group::setupTx ()
{
    bbGroup->setupTx(txPdoData);
}

////////////////////////////////////////////////////////////////////////////
bool Group::setupRx (BB::Main *main)
{
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

    log_debug("signals = %zu %f", signals.size(), bbGroup->sampleTime);

    main->openSharedMemory(false);

    for (RxPdoMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        ChunkData chunkData;
        BB::Group::CopyList *copy_list =
            new BB::Group::CopyList[it->second.size() + 1];

        chunkData.group = it->first;
        chunkData.copy_list = copy_list;

        rxPdoChunk.push_back(chunkData);

        for (RxPdoBuddyList::const_iterator it2 = it->second.begin();
                it2 != it->second.end(); it2++) {
            copy_list->src = it2->second->shmemAddr;
            copy_list->dst = it2->first->addr;
            copy_list->len = it2->first->size();
            log_debug("src=%p dst=%p len=%zu", copy_list->src,
                    copy_list->dst, copy_list->len);

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
        size_t counter = (*it).group->receive((*it).copy_list);

        counter = counter;
    }
}
