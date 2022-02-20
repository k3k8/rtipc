find_path(Mhash_INCLUDE_DIR mhash.h)
find_library(Mhash_LIBRARY mhash)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mhash DEFAULT_MSG Mhash_LIBRARY Mhash_INCLUDE_DIR)

mark_as_advanced(Mhash_INCLUDE_DIR Mhash_LIBRARY)
