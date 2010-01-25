# - Try to find XCB-Aux
# Once done, this will define
#
#  XCB-Aux_FOUND - system has XCB-Aux
#  XCB-Aux_INCLUDE_DIRS - the XCB-Aux include directories
#  XCB-Aux_LIBRARIES - link these to use XCB-Aux

include(LibFindMacros)

libfind_package(XCB-Aux XCB)
libfind_pkg_check_modules(XCB-Aux_PKGCONF xcb-aux)

find_path(XCB-Aux_INCLUDE_DIR
    NAMES xcb/xcb_aux.h
    PATHS ${XCB-Aux_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB-Aux_LIBRARY
    NAMES xcb-aux
    PATHS ${XCB-Aux_PKGCONF_LIBRARY_DIRS}
)

set(XCB-Aux_PROCESS_INCLUDES XCB-Aux_INCLUDE_DIR)
set(XCB-Aux_PROCESS_LIBS XCB-Aux_LIBRARY)
libfind_process(XCB-Aux)

