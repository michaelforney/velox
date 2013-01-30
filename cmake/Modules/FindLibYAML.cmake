# - Try to find LibYAML
# Once done, this will define
#
#   LIBYAML_FOUND - System has LibYAML
#   LIBYAML_INCLUDE_DIRS - The LibYAML include directories
#   LIBYAML_LIBRARIES - The libraries needed to use LibYAML
#   LIBYAML_DEFINITIONS - Compiler switches required for using LibYAML

find_package(PkgConfig)
pkg_check_modules(PC_LIBYAML QUIET yaml)
set(LIBYAML_DEFINITIONS ${PC_LIBYAML_CFLAGS_OTHER})

find_path(LIBYAML_INCLUDE_DIR
    NAMES yaml.h
    HINTS ${PC_LIBYAML_INCLUDE_DIR} ${PC_LIBYAML_INCLUDE_DIRS}
)

find_library(LIBYAML_LIBRARY
    NAMES yaml
    HINTS ${PC_LIBYAML_LIBRARY} ${PC_LIBYAML_LIBRARY_DIRS}
)

set(LIBYAML_LIBRARIES ${LIBYAML_LIBRARY})
set(LIBYAML_INCLUDE_DIRS ${LIBYAML_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibYAML DEFAULT_MSG
    LIBYAML_LIBRARY
    LIBYAML_INCLUDE_DIR
)

mark_as_advanced(LIBYAML_LIBRARY LIBYAML_INCLUDE_DIR)

