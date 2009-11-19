# mwm: CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project(mwm)

find_package(PkgConfig REQUIRED)

pkg_check_modules(XCB REQUIRED xcb xcb-atom xcb-icccm)

add_executable(mwm
    mwm.c
    window.c
    layout.c
    hook.c
    tag.c
    bar.c
    keybinding.c
)

target_link_libraries(mwm ${XCB_LIBRARIES})

install(TARGETS mwm DESTINATION bin)

