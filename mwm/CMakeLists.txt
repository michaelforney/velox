# mwm: mwm/CMakeLists.txt

add_executable(mwm
    mwm.c
    window.c
    layout.c
    hook.c
    tag.c
    keybinding.c
    config_file.c
    plugin.c
)

include_directories(${mwm_SOURCE_DIR})

target_link_libraries(mwm
    libmwm
    ${XCB_LIBRARIES}
    ${XCB-Atom_LIBRARIES}
    ${XCB-ICCCM_LIBRARIES}
    ${XCB-Aux_LIBRARIES}
    ${LibYAML_LIBRARIES}
    ${X11_LIBRARIES}
    dl
    m
)

install(TARGETS mwm DESTINATION bin)

install(FILES
    config_file.h
    hook.h
    keybinding.h
    layout.h
    mwm.h
    tag.h
    window.h

    DESTINATION include/mwm
)

