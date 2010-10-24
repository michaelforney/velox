# - Try to find XCB-Image
# Once done, this will define
#
#  XCB-Image - system has XCB-Image
#  XCB-Image_INCLUDE_DIRS - the XCB-Image include directories
#  XCB-Image_LIBRARIES - link these to use XCB-Image

include(LibFindMacros)

libfind_package(XCB-Image XCB)
libfind_pkg_check_modules(XCB-Image_PKGCONF xcb-image)

find_path(XCB-Image_INCLUDE_DIR
    NAMES xcb/xcb_image.h
    PATHS ${XCB-Image_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB-Image_LIBRARY
    NAMES xcb-image
    PATHS ${XCB-Image_PKGCONF_LIBRARY_DIRS}
)

set(XCB-Image_PROCESS_INCLUDES XCB-Image_INCLUDE_DIR)
set(XCB-Image_PROCESS_LIBS XCB-Image_LIBRARY)
libfind_process(XCB-Image)

