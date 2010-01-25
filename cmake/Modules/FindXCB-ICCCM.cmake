# - Try to find XCB-ICCCM
# Once done, this will define
#
#  XCB-ICCCM_FOUND - system has XCB-ICCCM
#  XCB-ICCCM_INCLUDE_DIRS - the XCB-ICCCM include directories
#  XCB-ICCCM_LIBRARIES - link these to use XCB-ICCCM

include(LibFindMacros)

libfind_package(XCB-ICCCM XCB)
libfind_pkg_check_modules(XCB-ICCCM_PKGCONF xcb-icccm)

find_path(XCB-ICCCM_INCLUDE_DIR
    NAMES xcb/xcb_icccm.h
    PATHS ${XCB-ICCCM_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB-ICCCM_LIBRARY
    NAMES xcb-icccm
    PATHS ${XCB-ICCCM_PKGCONF_LIBRARY_DIRS}
)

set(XCB-ICCCM_PROCESS_INCLUDES XCB-ICCCM_INCLUDE_DIR)
set(XCB-ICCCM_PROCESS_LIBS XCB-ICCCM_LIBRARY)
libfind_process(XCB-ICCCM)

