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

#ifndef RXPDO_H
#define RXPDO_H

#include "BulletinBoard/Signal.h"

namespace BB = BulletinBoard;

namespace RtIPC {

class Group;

class RxPdo: public BB::Signal {
    public:
        RxPdo(const Group*, const std::string& name, 
                const BB::DataType& datatype, size_t n,
                void *addr, unsigned char *connected);

        const Group * const group;
        void * const addr;
        unsigned char * const connected;

        const void** srcAddr;
        unsigned char **connectedAddr;

    private:
};

}

#endif // RXPDO_H
