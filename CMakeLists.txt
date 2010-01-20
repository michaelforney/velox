# mwm: CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project(mwm C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(XCB REQUIRED xcb xcb-atom xcb-icccm xcb-aux)

add_subdirectory(libmwm)
add_subdirectory(mwm)

