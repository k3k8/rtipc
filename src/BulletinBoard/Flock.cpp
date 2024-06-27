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

#include "Flock.h"
#include <iostream>
#include <stdexcept>

#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


using namespace BulletinBoard;

/////////////////////////////////////////////////////////////////////////////
Flock::Flock (const std::string &file)
{
    struct flock f;
    int rv;

    fd = ::open(file.c_str(), O_WRONLY);

    if (fd < 0)
        throw std::runtime_error("Could not open lock file");

    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_start = 0;
    f.l_len = 1;

    while ((rv = fcntl(fd, F_SETLKW, &f)) == -1) {
        if (errno != EINTR)
            throw std::runtime_error("flock()");
    }
}

/////////////////////////////////////////////////////////////////////////////
Flock::~Flock ()
{
    ::close(fd);
}
