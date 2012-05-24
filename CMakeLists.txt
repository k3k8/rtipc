#############################################################################
#
#  $Id$
#
#  Copyright 2012 Richard Hacker (lerichi at gmx dot net)
#
#  This is the main cmake file for PdServ target for Real-Time Workshop(TM)
#  taget of Mathworks.
#
#  The following options can be specified on the command line of cmake to
#  change the installation paths (Defaults are in <>)
#       -DPREFIX=</opt/etherlab>
#
#  For debugging, use the
#       -DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo|MinSizeRel
#  option
#
#  This file is part of the PdServ-MATLAB package.
#
#  PdServ-MATLAB is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  PdServ-MATLAB is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with PdServ-MATLAB. See COPYING. If not, see
#  <http://www.gnu.org/licenses/>.
#
#############################################################################


CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (rtipc)
SET (CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

SET (MAJOR_VERSION 0)
SET (MINOR_VERSION 99)
SET (PATCH_LEVEL 0)
SET (VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL})

# library version
SET (SOVERSION ${MAJOR_VERSION}.${MINOR_VERSION})

# Project path options
IF (NOT PREFIX)         # Project root
    SET (PREFIX "/opt/etherlab")
ENDIF ()

SET (CMAKE_INSTALL_PREFIX "${PREFIX}")
LIST (APPEND CMAKE_PREFIX_PATH "${PREFIX}")

IF (SYSCONFDIR)         # configuration files
    SET (ABS_SYSCONFDIR "${SYSCONFDIR}")
    SET (PC_SYSCONFDIR "${SYSCONFDIR}")
ELSE ()
    SET (ABS_SYSCONFDIR ${PREFIX}/etc)
    SET (PC_SYSCONFDIR "\${prefix}/etc")
ENDIF ()

IF (INCLUDEDIR)         # include files
    SET (ABS_INCLUDEDIR "${INCLUDEDIR}")
    SET (PC_INCLUDEDIR "${INCLUDEDIR}")
    LIST (APPEND CMAKE_INCLUDE_PATH "${INCLUDEDIR}")
ELSE ()
    SET (ABS_INCLUDEDIR ${PREFIX}/include)
    SET (PC_INCLUDEDIR "\${prefix}/include")
ENDIF ()

IF (EXECDIR)            # executables
    SET (ABS_EXECDIR "${EXECDIR}")
    SET (PC_EXECDIR "${EXECDIR}")
ELSE ()
    SET (ABS_EXECDIR ${PREFIX}/bin)
    SET (PC_EXECDIR "\${prefix}/bin")
ENDIF ()

IF (LIBDIR)             # libraries
    SET (ABS_LIBDIR "${LIBDIR}")
    SET (PC_LIBDIR "${LIBDIR}")
ELSE ()
    SET (ABS_LIBDIR ${PREFIX}/lib)
    SET (PC_LIBDIR "\${prefix}/lib")
ENDIF ()

# Make sure that the project options are remembered
SET (PREFIX "${PREFIX}" CACHE PATH
    "Root directory for the project" FORCE)
SET (SYSCONFDIR "${SYSCONFDIR}" CACHE PATH
    "Directory for configuration files" FORCE)
SET (INCLUDEDIR "${INCLUDEDIR}" CACHE PATH
    "Directory for header files" FORCE)
SET (EXECDIR "${EXECDIR}" CACHE PATH
    "Directory for executables" FORCE)
SET (LIBDIR "${LIBDIR}" CACHE PATH
    "Directory for libraries" FORCE)

OPTION ( DEBUG "Debugging output" OFF )
SET  (RTIPC_DEBUG ${DEBUG})

if (DEBUG)
    string(LENGTH "${PROJECT_SOURCE_DIR}/src/" SRC_PATH_LENGTH)
endif (DEBUG)

# Search for mercurial, used to add ID information to the library
LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# FIND_PACKAGE (Mercurial)
# IF (MERCURIAL_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.hg")
# MERCURIAL_HG_INFO (${PROJECT_SOURCE_DIR} Package)
# ENDIF ()

ADD_SUBDIRECTORY (rtw)
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
SET (CPACK_SOURCE_IGNORE_FILES
    "/CMakeCache.txt"
    "/CMakeFiles/"
    "/Makefile$"
    "/\\\\.hg"
    "/build.*"
    "/config.h$"
    "/etherlab-.*\\\\.tar\\\\."
    "\\\\.so"
    "\\\\.sw.$"
    "~$"
    "${CPACK_SOURCE_IGNORE_FILES}"
    )

INCLUDE (CPack)
ADD_CUSTOM_TARGET (dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

CONFIGURE_FILE (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )
