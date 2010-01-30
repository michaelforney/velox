# velox: CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project(velox C)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(PkgConfig REQUIRED)
find_package(LibYAML REQUIRED)
find_package(XCB REQUIRED)
find_package(XCB-Atom REQUIRED)
find_package(XCB-ICCCM REQUIRED)
find_package(XCB-Aux REQUIRED)
find_package(X11 REQUIRED) # For XStringToKeysym

add_subdirectory(libvelox)
add_subdirectory(velox)
add_subdirectory(modules)

install(FILES velox.yaml keys.yaml DESTINATION /etc/velox)

