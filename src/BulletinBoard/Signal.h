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

#ifndef BB_SIGNAL_H
#define BB_SIGNAL_H

#include <string>
#include "DataType.h"

namespace BulletinBoard {

class Group;

class Signal {
    public:
        Signal(Group *group, const std::string& name,
                const DataType& datatype, size_t n);

        Group * const group;
        const std::string name;
        const DataType dataType;
        const size_t elementCount;

        const void*  src;
        const void** srcAddr;

        char *shmemAddr;

        size_t size() const;

        bool operator!= (const Signal& other) const;
        bool operator== (const Signal& other) const {
            return !(*this != other);
        }
    private:
};

}

#endif // BB_SIGNAL_H
