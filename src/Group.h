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

#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <list>
#include "BulletinBoard/Group.h"

namespace BulletinBoard {
    class DataType;
    class Signal;
};

namespace BB = BulletinBoard;

namespace RtIPC {

class Main;
class RxPdo;

class Group {
    public:
        Group(Main *main, BB::Group *g);
        ~Group();

        Main * const main;
        BB::Group * const bbGroup;

        int addTxPdo(const std::string &name,
                const BB::DataType& datatype, const void *addr, size_t n);
        void addRxPdo(const std::string &name,
                const BB::DataType& datatype, void *addr, size_t n,
                unsigned char *connected);

        void setupTx();
        bool setupRx(BB::Main *bb);

        void receive() const;

    private:
        BB::Group::PdoMap txPdoData;

        typedef std::list<RxPdo *> RxPdoList;
        RxPdoList rxPdo;

        struct ChunkData {
            const BB::Group *group;
            BulletinBoard::Group::CopyList *copy_list;
        };

        typedef std::list<ChunkData> RxPdoChunk;
        RxPdoChunk rxPdoChunk;
};

}

#endif // GROUP_H
