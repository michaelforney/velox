# velox: velox/CMakeLists.txt

add_executable(velox
    velox.c
    window.c
    layout.c
    hook.c
    tag.c
    keybinding.c
    config_file.c
    module.c
)

include_directories(${velox_SOURCE_DIR})

target_link_libraries(velox
    libvelox
    ${XCB_LIBRARIES}
    ${XCB-Atom_LIBRARIES}
    ${XCB-ICCCM_LIBRARIES}
    ${XCB-Aux_LIBRARIES}
    ${LibYAML_LIBRARIES}
    ${X11_LIBRARIES}
    dl
    m
)

install(TARGETS velox DESTINATION bin)

install(FILES
    config_file.h
    hook.h
    keybinding.h
    layout.h
    velox.h
    tag.h
    window.h
    module.h

    DESTINATION include/velox
)

