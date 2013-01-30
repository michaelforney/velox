# - Try to find LibPulse
# Once done, this will define
#
#   LIBPULSE_FOUND - System has LibPulse
#   LIBPULSE_INCLUDE_DIRS - The LibPulse include directories
#   LIBPULSE_LIBRARIES - The libraries needed to use LibPulse
#   LIBPULSE_DEFINITIONS - Compiler switches required for using LibPulse

find_package(PkgConfig)
pkg_check_modules(PC_LIBPULSE QUIET libpulse)
set(LIBPULSE_DEFINITIONS ${PC_LIBPULSE_CFLAGS_OTHER})

find_path(LIBPULSE_INCLUDE_DIR
    NAMES pulse/pulseaudio.h
    HINTS ${PC_LIBPULSE_INCLUDE_DIR} ${PC_LIBPULSE_INCLUDE_DIRS}
)

find_library(LIBPULSE_LIBRARY
    NAMES pulse
    HINTS ${PC_LIBPULSE_LIBRARY} ${PC_LIBPULSE_LIBRARY_DIRS}
)

set(LIBPULSE_LIBRARIES ${LIBPULSE_LIBRARY})
set(LIBPULSE_INCLUDE_DIRS ${LIBPULSE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibPulse DEFAULT_MSG
    LIBPULSE_LIBRARY
    LIBPULSE_INCLUDE_DIR
)

mark_as_advanced(LIBPULSE_LIBRARY LIBPULSE_INCLUDE_DIR)

