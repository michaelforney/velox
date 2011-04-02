# - Try to find LibPulse
# Once done, this will define
#
#  LibPulse_FOUND - system has Pulse
#  LibPulse_INCLUDE_DIRS - the Pulse include directories
#  LibPulse_LIBRARIES - link these to use Pulse

include(LibFindMacros)

libfind_pkg_check_modules(LibPulse_PKGCONF libpulse)

find_path(LibPulse_INCLUDE_DIR
    NAMES pulse/pulseaudio.h
    PATHS ${LibPulse_PKGCONF_INCLUDE_DIRS}
)

find_library(LibPulse_LIBRARY
    NAMES pulse
    PATHS ${LibPulse_PKGCONF_LIBRARY_DIRS}
)

set(LibPulse_PROCESS_INCLUDES LibPulse_INCLUDE_DIR)
set(LibPulse_PROCESS_LIBS LibPulse_LIBRARY)
libfind_process(LibPulse)

