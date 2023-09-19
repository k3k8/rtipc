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


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

ADD_LIBRARY( ${PROJECT_NAME} SHARED
    Group.cpp Group.h
    Main.cpp Main.h
    Debug.cpp Debug.h
    RxPdo.cpp RxPdo.h

    BulletinBoard/Group.cpp BulletinBoard/Group.h
    BulletinBoard/Flock.cpp BulletinBoard/Flock.h
    BulletinBoard/Main.cpp BulletinBoard/Main.h
    BulletinBoard/YamlDoc.cpp BulletinBoard/YamlDoc.h
    BulletinBoard/DataType.cpp BulletinBoard/DataType.h
    BulletinBoard/Semaphore.cpp BulletinBoard/Semaphore.h
    BulletinBoard/Signal.cpp BulletinBoard/Signal.h
    )

SET_TARGET_PROPERTIES ( ${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION "${SOVERSION}"
    CLEAN_DIRECT_OUTPUT 1
    CXX_VISIBILITY_PRESET "hidden"
    VISIBILITY_INLINES_HIDDEN 1

)

TARGET_LINK_LIBRARIES (${PROJECT_NAME} PRIVATE
    ${Yaml_LIBRARIES}
    ${Mhash_LIBRARY}
)
TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<INSTALL_INTERFACE:include>"
)
TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} PRIVATE
    ${Yaml_INCLUDE_DIR}
    ${Mhash_INCLUDE_DIR}
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

# prepare version information

IF(VERSION_HASH)
    # Hash was given as option, so write it directly
    # useful for OBS packaging
    FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/git_revision_hash.h"
        "#define GIT_REV \"${VERSION_HASH}\"
")
ELSE()
    # recompute hash on every `make` invocation
    # Note: CMake variable definitions (-D) must be before the -P option!
    ADD_CUSTOM_TARGET(GitRevision
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/git_revision_hash.h"
        COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/.."
            -DHASH_MACRO_NAME="GIT_REV"
            -DTARGET_FILE="${CMAKE_CURRENT_BINARY_DIR}/git_revision_hash.h"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/git_revision.cmake
    )
    ADD_DEPENDENCIES(${PROJECT_NAME} GitRevision)
ENDIF()


# pkgconfig files
CONFIGURE_FILE (
    "${CMAKE_CURRENT_SOURCE_DIR}/librtipc.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/librtipc.pc" @ONLY
)

INCLUDE (CMakePackageConfigHelpers)
# cmake target export stuff
configure_package_config_file (config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
write_basic_package_version_file (
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    COMPATIBILITY SameMajorVersion
)
INSTALL (TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# cmake target export and pkg-config
INSTALL (FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
INSTALL (EXPORT ${PROJECT_NAME}Targets
    NAMESPACE EtherLab::
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

INSTALL (FILES "${CMAKE_CURRENT_BINARY_DIR}/librtipc.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

EXPORT(EXPORT ${PROJECT_NAME}Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
    NAMESPACE EtherLab::
)
