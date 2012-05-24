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

#ifndef BB_SEM_H
#define BB_SEM_H

#include <stddef.h>
#include <sys/sem.h>

namespace BulletinBoard {

class Semaphore {
    public:
        Semaphore(int semid, size_t instance);
        void lock();
        void unlock();

    private:
        const int semid;

        struct sembuf sop;
};

class SemaphoreLock {
    public:
        SemaphoreLock(Semaphore *s): sem(s) {
            sem->lock();
        }
        ~SemaphoreLock() {
            sem->unlock();
        }
    private:
        Semaphore * const sem;
};

}

#endif // BB_SEM_H
