# - Try to find Yaml
# Once done this will define
#  Yaml_FOUND - System has yaml
#  Yaml_INCLUDE_DIRS - The yaml include directories
#  Yaml_LIBRARIES - The libraries needed to use yaml
#  Yaml_DEFINITIONS - Compiler switches required for using yaml

find_package(PkgConfig)

if (NOT ${CMAKE_VERSION} VERSION_LESS 2.8)
    set (QUIET QUIET)
endif ()
pkg_check_modules(PC_YAML ${QUIET} yaml)

set(Yaml_DEFINITIONS ${PC_Yaml_CFLAGS_OTHER})

find_path(Yaml_INCLUDE_DIR yaml.h
        HINTS ${PC_Yaml_INCLUDEDIR} ${PC_Yaml_INCLUDE_DIRS}
        PATH_SUFFIXES yaml )

find_library(Yaml_LIBRARY
        NAMES ${PC_Yaml_LIBRARIES} yaml
        HINTS ${PC_Yaml_LIBDIR} ${PC_Yaml_LIBRARY_DIRS} )

set(Yaml_LIBRARIES ${Yaml_LIBRARY} )
set(Yaml_INCLUDE_DIRS ${Yaml_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set Yaml_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Yaml  DEFAULT_MSG
        Yaml_LIBRARY Yaml_INCLUDE_DIR)

mark_as_advanced(Yaml_INCLUDE_DIR Yaml_LIBRARY )
