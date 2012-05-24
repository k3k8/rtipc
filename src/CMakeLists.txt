#############################################################################
#
#  $Id$
#
#  Copyright 2012 Richard Hacker (lerichi at gmx dot net)
#
#  This file is part of the rtipc library.
#
#  The rtipc library is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  The rtipc library is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
#  License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with the rtipc library. If not, see <http://www.gnu.org/licenses/>.
#
#############################################################################

INCLUDE_DIRECTORIES(
        ${PROJECT_BINARY_DIR}       # for config.h
        ${PROJECT_SOURCE_DIR}
#         ${LIBCCEXT2_INCLUDE_DIRS}
        )

SET (CMAKE_CXX_FLAGS "-Wall")

FIND_PACKAGE (Yaml REQUIRED)

ADD_LIBRARY( ${PROJECT_NAME} SHARED
    Group.cpp Group.h
    Main.cpp Main.h
    Debug.cpp Debug.h
    RxPdo.cpp RxPdo.h

    BulletinBoard/Group.cpp BulletinBoard/Group.h
    BulletinBoard/Flock.cpp BulletinBoard/Flock.h
    BulletinBoard/Main.cpp BulletinBoard/Main.h
    BulletinBoard/Config.cpp BulletinBoard/Config.h
    BulletinBoard/DataType.cpp BulletinBoard/DataType.h
    BulletinBoard/Semaphore.cpp BulletinBoard/Semaphore.h
    BulletinBoard/Signal.cpp BulletinBoard/Signal.h
    )

SET_TARGET_PROPERTIES ( ${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION}
    SOVERSION "${SOVERSION}"
    CLEAN_DIRECT_OUTPUT 1)

TARGET_LINK_LIBRARIES (${PROJECT_NAME} ${YAML_LIBRARIES} mhash)
INSTALL (TARGETS ${PROJECT_NAME} DESTINATION "${ABS_LIBDIR}" LIBRARY)

# pkgconfig files
CONFIGURE_FILE (
    "${CMAKE_CURRENT_SOURCE_DIR}/librtipc.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/librtipc.pc" @ONLY
    )
INSTALL (FILES "${CMAKE_CURRENT_BINARY_DIR}/librtipc.pc"
    DESTINATION "${ABS_LIBDIR}/pkgconfig")
