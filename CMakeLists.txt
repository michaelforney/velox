# mwm: CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project(mwm C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(PkgConfig REQUIRED)
find_package(LibYAML REQUIRED)
find_package(XCB REQUIRED)
find_package(XCB-Atom REQUIRED)
find_package(XCB-ICCCM REQUIRED)
find_package(XCB-Aux REQUIRED)
find_package(X11 REQUIRED) # For XStringToKeysym

add_subdirectory(libmwm)
add_subdirectory(mwm)
add_subdirectory(plugins)

install(FILES mwm.yaml keys.yaml DESTINATION /etc/mwm)

