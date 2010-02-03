# - Try to find XCB-EWMH
# Once done, this will define
#
#  XCB-EWMH_FOUND - system has XCB-EWMH
#  XCB-EWMH_INCLUDE_DIRS - the XCB-EWMH include directories
#  XCB-EWMH_LIBRARIES - link these to use XCB-EWMH

include(LibFindMacros)

libfind_package(XCB-EWMH XCB)
libfind_pkg_check_modules(XCB-EWMH_PKGCONF xcb-ewmh)

find_path(XCB-EWMH_INCLUDE_DIR
    NAMES xcb/xcb_ewmh.h
    PATHS ${XCB-EWMH_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB-EWMH_LIBRARY
    NAMES xcb-ewmh
    PATHS ${XCB-EWMH_PKGCONF_LIBRARY_DIRS}
)

set(XCB-EWMH_PROCESS_INCLUDES XCB-EWMH_INCLUDE_DIR)
set(XCB-EWMH_PROCESS_LIBS XCB-EWMH_LIBRARY)
libfind_process(XCB-EWMH)

