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

#ifndef MAIN_H
#define MAIN_H

#include "BulletinBoard/DataType.h"
#include "BulletinBoard/Main.h"

#include <string>
#include <set>
#include <map>
#include <list>

namespace BB = BulletinBoard;

namespace RtIPC {

class Group;
class RxPdo;

class Main: public BB::Main {
    public:
        Main(const std::string &name, const std::string &cache_dir);
        ~Main();

        const std::string name;

        Group *addGroup(double sample_time);

        int start();

    private:
        typedef std::list<Group*> Groups;
        Groups groups;

        std::string confDir;

        std::set<std::string> pdos;

        typedef std::list<BB::Main*> Applications;
        Applications applications;

        void verifyConfig(const std::string& confFile);
        bool setupRx(BB::Main *bb);

        //std::list<const RxPdo*> rxPdo;
};

}

#endif // MAIN_H
