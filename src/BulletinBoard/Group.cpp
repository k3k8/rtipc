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

    return s;
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
    if (signals.empty())
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
    char *shmemAddr = reinterpret_cast<char*>(addr);
    sem = new Semaphore(semid, instance);

    for (SignalMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        it->second->shmemAddr = shmemAddr;
        shmemAddr += it->second->size();
    }

    this->counter = counter;

    return shmemAddr;
}

/////////////////////////////////////////////////////////////////////////////
void Group::setupTx (const PdoMap& txPdoData)
{
    copy_list = new CopyList[signals.size() + 1];
    CopyList *cl = copy_list;

    for (SignalMap::const_iterator it = signals.begin();
            it != signals.end(); it++) {
        const Signal *s = it->second;
        PdoMap::const_iterator it2 = txPdoData.find(s);

        cl->len = s->size();
        cl->dst = s->shmemAddr;

        if (it2 != txPdoData.end()) {
            cl->src = it2->second;
            cl++;
        }
    }

    cl->src = 0;
}

//////////////////////////////////////////////////////////////////////////////
void Group::transmit () const
{
    if (!copy_list)
        return;

    SemaphoreLock lock(sem);
    for (const CopyList *cl = copy_list; cl->src; cl++)
        ::memcpy(cl->dst, cl->src, cl->len);

    (*counter)++;
}

//////////////////////////////////////////////////////////////////////////////
size_t Group::receive (const CopyList* copy_list) const
{
    SemaphoreLock lock(sem);
    for (const CopyList *cl = copy_list; cl->src; cl++) {
        ::memcpy(cl->dst, cl->src, cl->len);
    }

    return *counter;
}
