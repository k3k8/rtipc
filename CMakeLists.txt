#############################################################################
#
#  $Id$
#
#  Copyright 2012 Richard Hacker (lerichi at gmx dot net)
#
#  This is the main cmake file for the rtipc package
#
#  The following options can be specified on the command line of cmake to
#  change the installation paths (Defaults are in <>)
#       -DCMAKE_INSTALL_PREFIX=</usr/local>
#       -DCMAKE_INSTALL_INCLUDEDIR=<${CMAKE_INSTALL_PREFIX}/include>
#       -DCMAKE_INSTALL_LIBDIR=<${CMAKE_INSTALL_PREFIX}/lib(64)>
#       -DCMAKE_INSTALL_LOCALSTATEDIR=<${CMAKE_INSTALL_PREFIX}/var>
#
#  For debugging, use the
#       -DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo|MinSizeRel
#  option
#
#  This file is part of the rtipc package.
#
#  rtipc is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  rtipc is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with rtipc. See COPYING. If not, see
#  <http://www.gnu.org/licenses/>.
#
#############################################################################


CMAKE_MINIMUM_REQUIRED (VERSION 3.2)
PROJECT (rtipc VERSION 1.0.0)

#
# Release Instructions
#
# - Make sure new symbols are listed in src/librtipc.map
# - Update version numbers above (semantic versioning)
# - Update SOVERSION below on abi breakage
# - Update ChangeLog
# - Update NEWS file
# - make dist
#

# library version
SET (SOVERSION 1)

# GNU Paths:
#       CMAKE_INSTALL_*
INCLUDE (GNUInstallDirs)

OPTION ( DEBUG "Debugging output" OFF )
SET  (RTIPC_DEBUG ${DEBUG})

if (DEBUG)
    string(LENGTH "${PROJECT_SOURCE_DIR}/src/" SRC_PATH_LENGTH)
endif (DEBUG)

# Search for mercurial, used to add ID information to the library
LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
FIND_PACKAGE (Yaml REQUIRED)
FIND_PACKAGE (Mhash REQUIRED)

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (test)

INSTALL (DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING PATTERN "*.h")

# Directory where RTIPC stores its variable cache files
SET (VARDIR "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/${PROJECT_NAME}")
INSTALL (CODE "FILE(MAKE_DIRECTORY \"\$ENV{DESTDIR}${VARDIR}\")")

CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )
