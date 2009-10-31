# mwm: CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project(mwm)

find_package(PkgConfig REQUIRED)

pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(XCB-ATOM REQUIRED xcb-atom)

add_executable(mwm
    mwm.c
    window.c
    layout.c
    hook.c
    tag.c
    bar.c
)

target_link_libraries(mwm ${XCB-ATOM_LIBRARIES} ${XCB_LIBRARIES})

install(TARGETS mwm DESTINATION bin)

