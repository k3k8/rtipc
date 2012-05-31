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

#include "Debug.h"

#ifdef RTIPC_DEBUG

using namespace Debug;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int Log::level = 3;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Log::Space::Space(char c): space(c)
{
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Log::Log(const char *file, const char* func, int line, const Level& level)
{
    print = level <= Log::level;
    space = ' ';

    if (print) {
        switch (level) {
            case Critical:
                std::cout << "CRIT ";
                break;

            case Notice:
                std::cout << "NOTC ";
                break;

            case Debug:
                std::cout << "DBG  ";
                break;
        }

        std::cout
            << std::string(file + SRC_PATH_LENGTH)
            << ':' << func
            << '(' << line << "):";
    }
}

/////////////////////////////////////////////////////////////////////////////
Log::~Log()
{
    if (print)
        std::cout << std::endl;
}

/////////////////////////////////////////////////////////////////////////////
Log& Log::operator<< (const Space& s)
{
    space = s.space;
    return *this;
}

/////////////////////////////////////////////////////////////////////////////
void Log::setLevel(int n)
{
    level = n;
}

/////////////////////////////////////////////////////////////////////////////
bool Log::prefix()
{
    if (!print)
        return false;

    if (space)
        std::cout << space;
    space = ' ';

    return true;
}

#endif //RTIPC_DEBUG
