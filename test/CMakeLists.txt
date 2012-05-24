##############################################################################
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
##############################################################################

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

INCLUDE_DIRECTORIES(
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/include
    )

LINK_DIRECTORIES(${rtipc_BINARY_DIR}/src)

ADD_EXECUTABLE(test1 test1.cpp)
TARGET_LINK_LIBRARIES(test1 ${PROJECT_NAME} rt)

ADD_TEST(test1 test1)
