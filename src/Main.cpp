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

#include "config.h"

#include "Debug.h"
#include "include/rtipc.h"

#include "Main.h"
#include "Group.h"
#include "BulletinBoard/Signal.h"
#include "BulletinBoard/YamlDoc.h"
#include "RxPdo.h"
#include "BulletinBoard/Main.h"
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cerrno>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

namespace BB = BulletinBoard;

using namespace RtIPC;

//////////////////////////////////////////////////////////////////////////////
struct rtipc* rtipc_create (const char *name, const char *cache_dir)
{
    return (struct rtipc*) new RtIPC::Main (name, cache_dir ? cache_dir : "");
}

//////////////////////////////////////////////////////////////////////////////
struct rtipc_group* rtipc_create_group (
        struct rtipc* rtipc, double sample_time)
{
    Main *main = reinterpret_cast<RtIPC::Main*>(rtipc);

    return (struct rtipc_group *) main->addGroup(sample_time);
}

//////////////////////////////////////////////////////////////////////////////
struct txpdo* rtipc_txpdo (struct rtipc_group *g, const char *name,
        enum rtipc_datatype_t datatype, const void *addr, size_t n)
{
    Group *group = reinterpret_cast<RtIPC::Group*>(g);

    return (struct txpdo*)group->addTxPdo(name, datatype, addr, n);
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_set_txpdo_addr (const struct txpdo* pdo, const void *addr)
{
    const BB::Signal* s = reinterpret_cast<const BB::Signal*>(pdo);
    s->group->setAddr(s, addr);
}

//////////////////////////////////////////////////////////////////////////////
struct rxpdo* rtipc_rxpdo (struct rtipc_group *g,
        const char *name, enum rtipc_datatype_t datatype,
        void *addr, size_t n, unsigned char *connected)
{
    Group *group = reinterpret_cast<RtIPC::Group*>(g);
    const RxPdo* pdo = group->addRxPdo(name, datatype, addr, n, connected);
    return (struct rxpdo*)pdo;
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_set_rxpdo_addr (const struct rxpdo* pdo, const void *addr)
{
    const RxPdo* s = reinterpret_cast<const RxPdo*>(pdo);
    s->group->setAddr(s, addr);
}

//////////////////////////////////////////////////////////////////////////////
int rtipc_prepare (struct rtipc* rtipc)
{
    return reinterpret_cast<RtIPC::Main*>(rtipc)->start();
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_tx (struct rtipc_group *group)
{
    return reinterpret_cast<RtIPC::Group*>(group)->bbGroup->transmit();
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_rx (struct rtipc_group *group)
{
    return reinterpret_cast<RtIPC::Group*>(group)->receive();
}

//////////////////////////////////////////////////////////////////////////////
void rtipc_exit (struct rtipc* rtipc)
{
    delete reinterpret_cast<RtIPC::Main*>(rtipc);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
Main::Main (const std::string &name, const std::string &cache_dir):
    name(name)
{
    confDir = cache_dir.empty() ?  QUOTE(SYSCONFDIR) "/rtipc" : cache_dir;
    if (*confDir.rbegin() != '/')
        confDir.append(1,'/');

    if (::access(confDir.c_str(), R_OK))
        throw std::runtime_error(
                std::string("No access to directory ").append(confDir));
}

//////////////////////////////////////////////////////////////////////////////
Main::~Main ()
{
    for (Applications::iterator it = applications.begin();
            it != applications.end(); it++)
        delete *it;
}

//////////////////////////////////////////////////////////////////////////////
Group* Main::addGroup (double sampleTime)
{
    Group *g = new Group(this, BB::Main::addGroup(sampleTime));
    groups.push_back(g);
    return g;
}

//////////////////////////////////////////////////////////////////////////////
void Main::verifyConfig (const std::string& confFile)
{
    log_debug() << "Verify compatable";
    if (!::access(confFile.c_str(), F_OK)) {
        // Config file exists. Load it
        try {
            BulletinBoard::Main bb(confFile);

            log_debug() << "Testing compatability to" << confFile;
            if (compatible(bb))
                return;
        }
        catch (const std::exception& e) {
            // Some parsing or config file syntax error occurred
            log_debug() << "Configuration file corrupt";
        }
    }

    log_debug() << confFile << "is not compatable. saving";
    save(confFile);
}

/////////////////////////////////////////////////////////////////////////////
bool Main::setupRx (BB::Main *bb)
{
    bool used = false;

    for (Groups::iterator it = groups.begin(); it != groups.end(); it++)
        used |= (*it)->setupRx(bb);

    return used;
}

/////////////////////////////////////////////////////////////////////////////
int Main::start ()
{
    size_t begin = name.rfind('/');
    begin = begin == std::string::npos ? 0 : (begin + 1);
    std::string confFile = confDir + name.substr(begin) + ".conf";

    verifyConfig(confFile);

    // If the shared memory exists
    int rv = openSharedMemory(true, confFile);
    if (rv)
        return rv;

    for (Groups::iterator it = groups.begin(); it != groups.end(); it++)
        (*it)->setupTx();

    // Connect to signals inside the application itself
    setupRx(this);

    DIR *dirp = opendir(confDir.c_str());
    BB::Main *bb = 0;
    while (dirp) {
        struct dirent *dp = readdir(dirp);
        struct stat fstat;

        if (!dp)
            break;

        std::string f = confDir + dp->d_name;

        // Skip if the file is the private config file
        // or the file does not end in .conf or is not a regular file
        if (f == confFile or f.size() <= 5 or f.substr(f.size() - 5) != ".conf"
                or ::stat(f.c_str(), &fstat) or !S_ISREG(fstat.st_mode))
            continue;

        if (!bb)
            bb = new BB::Main;

        bb->load(f);

        if (setupRx(bb)) {
            applications.push_back(bb);
            bb = 0;
        }
    }
    delete bb;

    return 0;
}
