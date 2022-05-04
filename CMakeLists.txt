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


CMAKE_MINIMUM_REQUIRED (VERSION 2.8.5)
PROJECT (rtipc)
SET (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

#
# Release Instructions
#
# - Update version numbers below
# - Update SOVERSION below
# - Update ChangeLog (hg log --branch <branch> --style=changelog > ChangeLog)
# - Update NEWS file
# - make dist
#
SET (MAJOR_VERSION 0)
SET (MINOR_VERSION 99)
SET (PATCH_LEVEL 1)
SET (VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL})

# library version
SET (SOVERSION ${MAJOR_VERSION}.${MINOR_VERSION})

IF (DEFINED PREFIX)
    MESSAGE (WARNING "Using PREFIX is deprecated. Use CMAKE_INSTALL_PREFIX")
    SET (CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF ()

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
# FIND_PACKAGE (Mercurial)
# IF (MERCURIAL_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.hg")
# MERCURIAL_HG_INFO (${PROJECT_SOURCE_DIR} Package)
# ENDIF ()

ADD_SUBDIRECTORY (src)
ADD_SUBDIRECTORY (test)

SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "IPC for real time applications")
SET (CPACK_PACKAGE_VENDOR "Richard Hacker")
SET (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET (CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
SET (CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
SET (CPACK_PACKAGE_VERSION_PATCH ${PATCH_LEVEL})
SET (CPACK_PACKAGE_INSTALL_DIRECTORY
    "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET (CPACK_SOURCE_GENERATOR "TBZ2")
SET (CPACK_SOURCE_PACKAGE_FILE_NAME
      "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET (CPACK_GENERATOR "TGZ")
SET (CPACK_GENERATOR "DEB")
SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "Richard Hacker") #required
LIST (APPEND CPACK_SOURCE_IGNORE_FILES
    "/\\\\.hg"
    "/build*"
    "\\\\.sw.$"
    "~$"
    )

INCLUDE (CPack)
ADD_CUSTOM_TARGET (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

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
