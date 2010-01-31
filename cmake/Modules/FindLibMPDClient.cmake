# - Try to find LibMPDClient
# Once done, this will define
#
#  LibMPDClient_FOUND - system has LibMPDClient
#  LibMPDClient_INCLUDE_DIRS - the LibMPDClient include directories
#  LibMPDClient_LIBRARIES - link these to use LibMPDClient

include(LibFindMacros)

libfind_pkg_check_modules(LibMPDClient_PKGCONF libmpdclient)

find_path(LibMPDClient_INCLUDE_DIR
    NAMES mpd/player.h
    PATHS ${LibMPDClient_PKGCONF_INCLUDE_DIRS}
)

find_library(LibMPDClient_LIBRARY
    NAMES mpdclient
    PATHS ${LibMPDClient_PKGCONF_LIBRARY_DIRS}
)

set(LibMPDClient_PROCESS_INCLUDES LibMPDClient_INCLUDE_DIR)
set(LibMPDClient_PROCESS_LIBS LibMPDClient_LIBRARY)
libfind_process(LibMPDClient)

