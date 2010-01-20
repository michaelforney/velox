# mwm: mwm/CMakeLists.txt

add_executable(mwm
    mwm.c
    window.c
    layout.c
    hook.c
    tag.c
    keybinding.c
)

target_link_libraries(mwm libmwm ${XCB_LIBRARIES} m)

install(TARGETS mwm DESTINATION bin)

