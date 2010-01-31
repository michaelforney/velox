# velox: CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project(velox C)

option(ENABLE_LAYOUT_TILE   "Build the tile layout"         ON)
option(ENABLE_LAYOUT_GRID   "Build the grid layout"         ON)
option(ENABLE_MPD           "Build the MPD plugin"          ON)
option(ENABLE_SPAWN         "Build the spawn plugin"        ON)
option(ENABLE_WALLPAPER     "Build the wallpaper plugin"    ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(PkgConfig REQUIRED)
find_package(LibYAML REQUIRED)
find_package(XCB REQUIRED)
find_package(XCB-Atom REQUIRED)
find_package(XCB-ICCCM REQUIRED)
find_package(XCB-Aux REQUIRED)
find_package(X11 REQUIRED) # For XStringToKeysym

if(ENABLE_MPD)
    find_package(LibMPDClient REQUIRED)
endif(ENABLE_MPD)

add_subdirectory(libvelox)
add_subdirectory(velox)
add_subdirectory(modules)

install(FILES velox.yaml keys.yaml DESTINATION /etc/velox)

