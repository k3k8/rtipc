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

#ifndef BB_MAIN_H
#define BB_MAIN_H

#include <string>
#include <list>
#include <map>
#include <stdint.h>
#include "DataType.h"
#include "Group.h"

namespace BulletinBoard {

class Signal;

class Main {
    public:
        Main(const std::string& file = std::string());
        ~Main();

        void load(const std::string &file);
        bool save(const std::string &file);

        bool compatible(const Main& other);

        Group *addGroup(double sample_time);
        const Signal* newSignal(Group *group, const std::string &name,
                const DataType& datatype, size_t n);

        int openSharedMemory(bool exclusive,
                const std::string& lockFile = std::string());
        
        typedef std::map<std::string, const Signal*> SignalMap;
        const SignalMap& getSignalMap() const;

    protected:
        void setupTx(const Group::PdoMap& txPdoData);

        typedef std::list<Group*> Groups;
        Groups groups;

    private:

        std::string lockFile;

        int shmid;
        void *shmaddr;

        SignalMap signalMap;

        void clear();
        uint32_t checkSum() const;
        size_t groupCount() const;
};

}

#endif // BB_MAIN_H
