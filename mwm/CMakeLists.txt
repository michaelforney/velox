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

include_directories(${mwm_SOURCE_DIR}/libmwm)

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

