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

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include "config.h"

#ifdef RTIPC_DEBUG

#define log_debug() \
    Debug::Log(__BASE_FILE__, __func__, __LINE__, Debug::Log::Debug)
#define log_notice() \
    Debug::Log(__BASE_FILE__, __func__, __LINE__, Debug::Log::Notice)
#define log_crit() \
    Debug::Log(__BASE_FILE__, __func__, __LINE__, Debug::Log::Critical)
#define log_space(c) Debug::Log::Space(c)

namespace Debug {

class Log {
    public:
        struct Space {
            Space(char c = 0);
            const char space;
        };

        enum Level {Critical, Notice, Debug};

        Log(const char *file, const char* func, int line, const Level& level);
        ~Log();

        Log& operator<< (const Space& s);

        template<class T>
            Log& operator<< (const T& d) {
                if (prefix())
                    std::cout << d;

                return *this;
            }

        static void setLevel(int n);

    private:
        static int level;
        bool print;
        char space;

        bool prefix();
};

}

#else   // RTIPC_DEBUG

namespace {

class NullLog {
    public:
        template<class T>
            NullLog& operator<< (const T&) {

                return *this;
            }

};

}
#define log_debug()  NullLog()
#define log_notice() NullLog()
#define log_crit() NullLog()
#define log_space(c...) NullLog()


#endif  // RTIPC_DEBUG

#endif // DEBUG_H
