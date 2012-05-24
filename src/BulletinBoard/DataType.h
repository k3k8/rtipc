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

#ifndef BB_DATATYPE_H
#define BB_DATATYPE_H

#include <string>
#include "DataType.h"
#include "include/rtipc.h"

namespace BulletinBoard {

class DataType {
    public:
        DataType(enum rtipc_datatype_t datatype);
        DataType(const std::string& dt);

        size_t size() const;
        const char* c_str() const;

        bool operator!= (const DataType& other) const;

    private:
        const size_t dtIdx;

        static size_t fromString(const char *dt);
        static size_t fromType(enum rtipc_datatype_t dt);
};

}

#endif // BB_DATATYPE_H
