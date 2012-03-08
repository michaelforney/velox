# - Try to find XCB-Keysyms
# Once done, this will define
#
#  XCB-Keysyms_FOUND - system has XCB-Keysyms
#  XCB-Keysyms_INCLUDE_DIRS - the XCB-Keysyms include directories
#  XCB-Keysyms_LIBRARIES - link these to use XCB-Keysyms

include(LibFindMacros)

libfind_package(XCB-Keysyms XCB)
libfind_pkg_check_modules(XCB-Keysyms_PKGCONF xcb-keysyms)

find_path(XCB-Keysyms_INCLUDE_DIR
    NAMES xcb/xcb_keysyms.h
    PATHS ${XCB-Keysyms_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB-Keysyms_LIBRARY
    NAMES xcb-keysyms xcb-util
    PATHS ${XCB-Keysyms_PKGCONF_LIBRARY_DIRS}
)

set(XCB-Keysyms_PROCESS_INCLUDES XCB-Keysyms_INCLUDE_DIR)
set(XCB-Keysyms_PROCESS_LIBS XCB-Keysyms_LIBRARY)
libfind_process(XCB-Keysyms)

