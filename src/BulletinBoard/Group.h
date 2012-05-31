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

#ifndef BB_GROUP_H
#define BB_GROUP_H

#include <string>
#include <map>
#include <stdint.h>
#include "Semaphore.h"
#include "DataType.h"
#include "YamlDoc.h"

namespace BulletinBoard {

class Main;
class Signal;

class Group {
    public:
        Group(Main *main, double sampleTime);
        ~Group();

        const Signal* newSignal(const std::string &name,
                const DataType& datatype, size_t n);

        void save(YAML::Node&) const;
        bool empty() const;
        void copy(const Group *other);
        uint32_t checkSum() const;

        size_t getSignalSize() const;
        bool operator>(const Group& other) const;

        void *prepareIPC(size_t *counter,
                void *addr, int semid, int instance);

        typedef std::map<const Signal *, const void*> PdoMap;
        void setupTx(const PdoMap& txPdoData);
        void setAddr(const Signal* s, const void *) const;

        struct CopyList {
            void *dst;
            const void *src;
            size_t len;
        };
        size_t receive(const CopyList*) const;
        void transmit() const;

        Main * const main;
        const double sampleTime;

    private:
        Semaphore *sem;

        size_t signalSize;
        size_t *counter;

        typedef std::map<std::string, Signal*> SignalMap;
        SignalMap signals;

        CopyList *copy_list;
};

}

#endif // BB_GROUP_H
