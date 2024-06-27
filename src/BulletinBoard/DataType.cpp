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

#include "../Debug.h"

#include "DataType.h"
#include <stdint.h>
#include <cstring>
#include <stdexcept>

using namespace BulletinBoard;

static struct {
    const char *name;
    enum rtipc_datatype_t dt;
    size_t size;
} dt_list[] = {
    {"double",  rtipc_double_T, sizeof(double)},
    {"single",  rtipc_single_T, sizeof(float)},
    {"boolean", rtipc_boolean_T, sizeof(uint8_t)},
    {"uint8",   rtipc_uint8_T,  sizeof(uint8_t)},
    { "int8",   rtipc_sint8_T,  sizeof( int8_t)},
    {"uint16",  rtipc_uint16_T, sizeof(uint16_t)},
    { "int16",  rtipc_sint16_T, sizeof( int16_t)},
    {"uint32",  rtipc_uint32_T, sizeof(uint32_t)},
    { "int32",  rtipc_sint32_T, sizeof( int32_t)},
    {"uint64",  rtipc_uint64_T, sizeof(uint64_t)},
    { "int64",  rtipc_sint64_T, sizeof( int64_t)},
    {0,},
};

//////////////////////////////////////////////////////////////////////////////
DataType::DataType (enum rtipc_datatype_t datatype):
    dtIdx(fromType(datatype))
{
}

//////////////////////////////////////////////////////////////////////////////
DataType::DataType (const std::string& datatype):
    dtIdx(fromString(datatype.c_str()))
{
}

//////////////////////////////////////////////////////////////////////////////
size_t DataType::fromType (enum rtipc_datatype_t dt)
{
    for (size_t i = 0; dt_list[i].name; i++) {
        if (dt_list[i].dt == dt)
            return i;
    }

    throw std::runtime_error(
            std::string("Data type unknown"));
}

//////////////////////////////////////////////////////////////////////////////
size_t DataType::fromString (const char *dt)
{
    for (size_t i = 0; dt_list[i].name; i++) {
        if (!strcmp(dt, dt_list[i].name))
            return i;
    }

    throw std::runtime_error(
            std::string("Data type ").append(dt).append(" unknown"));
}

/////////////////////////////////////////////////////////////////////////////
bool DataType::operator!= (const DataType& other) const
{
    return other.dtIdx != dtIdx;
}

//////////////////////////////////////////////////////////////////////////////
const char* DataType::c_str () const
{
    return dt_list[dtIdx].name;
}

//////////////////////////////////////////////////////////////////////////////
size_t DataType::size () const
{
    return dt_list[dtIdx].size;
}
