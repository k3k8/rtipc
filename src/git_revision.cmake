##############################################################################
#
#  Copyright (C) 2023  Bjarne von Horn (vh at igh dot de)
#
#  This file is part of the Real Time IPC library.
#
#  The Real Time IPC library is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  The Real Time IPC library is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
#  License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with the Real Time IPC library. If not, see <http://www.gnu.org/licenses/>.
#
##############################################################################

# This script generates a file ${TARGET_FILE} which will contain a commit hash
# and a plus sign in case uncommitted changes exist in the source code.
# The name of the C Macro in the file will be named
# as configured with the variable ${HASH_MACRO_NAME}.
# Furthermore, the variable ${SOURCE_DIR} has to contain the path of
# the source directory, where the .git folder is.
# The timestamp of this file is only updated in case the commit hash and/or
# the state of uncommitted changes changes.
#
# To use this script, add a custom target and reference the file mentioned
# as BYPRODUCT. Use the CMAKE_COMMAND with the -P option to call this script.
# Don't forget to pass the variables as -D options,
# they have to precede the -P option!
# Finally, add a dependency between your exe/dll/so and the custom target.
# In your code, include the file and use the macro
# which will expand to the commit hash.

execute_process(COMMAND git -C "${SOURCE_DIR}" describe --dirty=+ --always --tags
    OUTPUT_VARIABLE GIT_REV_HYPHEN
    RESULT_VARIABLE GIT_STATUS
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ("${GIT_REV_HYPHEN}" STREQUAL "" OR NOT ${GIT_STATUS} EQUAL 0)
    set(GIT_REV "unknown")
else()
    # replace hyphens from git describe with dots
    string(REPLACE "-" "." GIT_REV "${GIT_REV_HYPHEN}")
endif()

set(VERSION "#define ${HASH_MACRO_NAME} \"${GIT_REV}\"
")

if(EXISTS ${TARGET_FILE})
    file(READ ${TARGET_FILE} VERSION_)
else()
    set(VERSION_ "")
endif()


if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE "${TARGET_FILE}" "${VERSION}")
endif()
