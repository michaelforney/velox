# - Try to find XCB-Atom
# Once done, this will define
#
#  XCB-Atom_FOUND - system has XCB-Atom
#  XCB-Atom_INCLUDE_DIRS - the XCB-Atom include directories
#  XCB-Atom_LIBRARIES - link these to use XCB-Atom

include(LibFindMacros)

libfind_package(XCB-Atom XCB)
libfind_pkg_check_modules(XCB-Atom_PKGCONF xcb-atom)

find_path(XCB-Atom_INCLUDE_DIR
    NAMES xcb/xcb_atom.h
    PATHS ${XCB-Atom_PKGCONF_INCLUDE_DIRS}
)

find_library(XCB-Atom_LIBRARY
    NAMES xcb-atom xcb-util
    PATHS ${XCB-Atom_PKGCONF_LIBRARY_DIRS}
)

set(XCB-Atom_PROCESS_INCLUDES XCB-Atom_INCLUDE_DIR)
set(XCB-Atom_PROCESS_LIBS XCB-Atom_LIBRARY)
libfind_process(XCB-Atom)

