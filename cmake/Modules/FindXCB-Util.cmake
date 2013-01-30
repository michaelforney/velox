# - Try to find XCB-Util
# Once done, this will define
#
#   XCB_UTIL_FOUND - System has XCB-Util
#   XCB_UTIL_INCLUDE_DIRS - The XCB-Util include directories
#   XCB_UTIL_LIBRARIES - The libraries needed to use XCB-Util
#   XCB_UTIL_DEFINITIONS - Compiler switches required for using XCB-Util
#
# Additionally, these per-component variables will be defined:
#
#   XCB_UTIL_ATOM_FOUND - System has xcb-atom
#   XCB_UTIL_ATOM_INCLUDE_DIR - The xcb-atom include directory
#   XCB_UTIL_ATOM_LIBRARY - The xcb-atom library
#
#   XCB_UTIL_AUX_FOUND - System has xcb-aux
#   XCB_UTIL_AUX_INCLUDE_DIR - The xcb-aux include directory
#   XCB_UTIL_AUX_LIBRARY - The xcb-aux library
#
#   XCB_UTIL_EVENT_FOUND - System has xcb-event
#   XCB_UTIL_EVENT_INCLUDE_DIR - The xcb-event include directory
#   XCB_UTIL_EVENT_LIBRARY - The xcb-event library
#
#   XCB_UTIL_ICCCM_FOUND - System has xcb-icccm
#   XCB_UTIL_ICCCM_INCLUDE_DIR - The xcb-icccm include directory
#   XCB_UTIL_ICCCM_LIBRARY - The xcb-icccm library
#
#   XCB_UTIL_EWMH_FOUND - System has xcb-ewmh
#   XCB_UTIL_EWMH_INCLUDE_DIR - The xcb-ewmh include directory
#   XCB_UTIL_EWMH_LIBRARY - The xcb-ewmh library
#
#   XCB_UTIL_KEYSYMS_FOUND - System has xcb-keysyms
#   XCB_UTIL_KEYSYMS_INCLUDE_DIR - The xcb-keysyms include directory
#   XCB_UTIL_KEYSYMS_LIBRARY - The xcb-keysyms library
#
#   XCB_UTIL_IMAGE_FOUND - System has xcb-image
#   XCB_UTIL_IMAGE_INCLUDE_DIR - The xcb-image include directory
#   XCB_UTIL_IMAGE_LIBRARY - The xcb-image library

find_package(PkgConfig)

set(XCB_UTIL_COMPONENTS Atom Aux Event ICCCM EWMH Keysyms Image)

if(NOT XCB-Util_FIND_COMPONENTS)
    message(FATAL_ERROR "Please specify Wayland components")
endif()

foreach(component ${XCB-Util_FIND_COMPONENTS})
    if(XCB_UTIL_COMPONENTS MATCHES ${component})
        string(TOLOWER ${component} _lower_component)
        string(TOUPPER ${component} _upper_component)
        pkg_check_modules(PC_XCB_UTIL_${_upper_component} QUIET
            xcb-${_lower_component})
        set(XCB_UTIL_DEFINITIONS ${XCB_UTIL_DEFINITIONS}
            ${PC_XCB_UTIL_${_upper_component}_CFLAGS_EXTRA})

        find_path(XCB_UTIL_${_upper_component}_INCLUDE_DIR
            NAMES xcb/xcb_${_lower_component}.h
            HINTS ${PC_XCB_UTIL_${_upper_component}_INCLUDE_DIR}
                  ${PC_XCB_UTIL_${_upper_component}_INCLUDE_DIRS}
        )

        find_library(XCB_UTIL_${_upper_component}_LIBRARY
            NAMES xcb-${_lower_component} xcb-util
            HINTS ${PC_XCB_UTIL_${_upper_component}_LIBRARY}
                  ${PC_XCB_UTIL_${_upper_component}_LIBRARY_DIRS}
        )

        if(XCB_UTIL_${_upper_component}_LIBRARY
            AND XCB_UTIL_${_upper_component}_INCLUDE_DIR)
            set(XCB_UTIL_${_upper_component}_FOUND 1)
        endif()

        mark_as_advanced(XCB_UTIL_${_upper_component}_LIBRARY
                         XCB_UTIL_${_upper_component}_INCLUDE_DIR)
        string(TOUPPER ${component} _upper_component)
        set(_XCB_UTIL_REQUIRED_VARS ${_XCB_UTIL_REQUIRED_VARS}
            XCB_UTIL_${_upper_component}_LIBRARY
            XCB_UTIL_${_upper_component}_INCLUDE_DIR)
        set(XCB_UTIL_LIBRARIES ${XCB_UTIL_LIBRARIES}
            ${XCB_UTIL_${_upper_component}_LIBRARY})
        set(XCB_UTIL_INCLUDE_DIRS ${XCB_UTIL_INCLUDE_DIRS}
            ${XCB_UTIL_${_upper_component}_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "Unknown XCB-Util component ${component}")
    endif()
endforeach(component)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XCB-Util
    REQUIRED_VARS ${_XCB_UTIL_REQUIRED_VARS})

