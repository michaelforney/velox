# - Try to find XCB
# Once done, this will define
#
#   XCB_FOUND - System has XCB
#   XCB_INCLUDE_DIRS - The XCB include directories
#   XCB_LIBRARIES - The libraries needed to use XCB
#   XCB_DEFINITIONS - Compiler switches required for using XCB

find_package(PkgConfig)
pkg_check_modules(PC_XCB QUIET xcb)
set(XCB_DEFINITIONS ${PC_XCB_CFLAGS_OTHER})

find_path(XCB_INCLUDE_DIR
    NAMES xcb/xcb.h
    HINTS ${PC_XCB_INCLUDE_DIR} ${PC_XCB_INCLUDE_DIRS}
)

find_library(XCB_LIBRARY
    NAMES xcb
    HINTS ${PC_XCB_LIBRARY} ${PC_XCB_LIBRARY_DIRS}
)

set(XCB_LIBRARIES ${XCB_LIBRARY})
set(XCB_INCLUDE_DIRS ${XCB_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XCB DEFAULT_MSG
    XCB_LIBRARY
    XCB_INCLUDE_DIR
)

mark_as_advanced(XCB_LIBRARY XCB_INCLUDE_DIR)

