# - Try to find XCB
# Once done, this will define
#
#  XCB_FOUND - system has XCB
#  XCB_INCLUDE_DIRS - the XCB include directories
#  XCB_LIBRARIES - link these to use XCB

include(LibFindMacros)

libfind_pkg_check_modules(XCB_PKGCONF xcb)

find_path(XCB_INCLUDE_DIR
    NAMES xcb/xcb.h
    PATHS ${XCB_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB_LIBRARY
    NAMES xcb
    PATHS ${XCB_PKGCONF_LIBRARY_DIRS}
)

set(XCB_PROCESS_INCLUDES XCB_INCLUDE_DIR)
set(XCB_PROCESS_LIBS XCB_LIBRARY)
libfind_process(XCB)

