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

//#include "config.h"

#include "Debug.h"
#include "include/rtipc.h"

//////////////////////////////////////////////////////////////////////////////
struct rtipc* rtipc_create( const char *name, const char *cache_dir)
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////
struct rtipc_group* rtipc_create_group(
        struct rtipc* rtipc, double sample_time)
{
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////
int rtipc_signal( struct rtipc_group *group, const char *name,
        enum rtipc_datatype_t datatype, const void *addr, size_t n)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
int rtipc_prepare( struct rtipc* rtipc)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
int rtipc_find_signal( struct rtipc_group *group, const char *name,
        enum rtipc_datatype_t datatype, const void *addr,
        size_t n, unsigned char *connected)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_tx( struct rtipc_group *group)
{
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_rx( struct rtipc_group *group)
{
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_exit( struct rtipc* rtipc)
{
}
