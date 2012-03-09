/*****************************************************************************
 *
 *  $Id$
 *
 *  Copyright 2010 Richard Hacker (lerichi at gmx dot net)
 *
 *  This file is part of the pdserv library.
 *
 *  The pdserv library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or (at
 *  your option) any later version.
 *
 *  The pdserv library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the pdserv library. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include "rtipc.h"
#include <stdint.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, const char *argv[])
{
    struct rtipc *rtipc = rtipc_create(argv[0], NULL);
    struct rtipc_group *group1 = rtipc_create_group(rtipc, 0.1);
    double sig01[6];
    uint16_t sig02[5];
    uint8_t sig03;

    assert(!rtipc_signal(group1, "SIG01", rtipc_double_T, sig01, 6));
    assert(!rtipc_signal(group1, "SIG02", rtipc_uint16_T, sig02, 5));
    assert(!rtipc_signal(group1, "SIG03", rtipc_boolean_T, &sig03, 1));

    assert(!rtipc_prepare(rtipc));

    while (1) {
        struct timespec time;

        //sleep(1);
        usleep(100000);
        clock_gettime(CLOCK_REALTIME, &time);

        rtipc_rx(group1);

        sig01[0] = !sig01[0];
        sig01[1] += 0.1;
        sig01[2] = time.tv_sec + time.tv_nsec * 1.0e-9;

        sig02[0]++;

        sig03 = !sig03;

        rtipc_tx(group1);
    }

    rtipc_exit(rtipc);

    return 0;
}
