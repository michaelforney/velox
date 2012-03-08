# velox: velox/CMakeLists.txt

add_executable(velox
    velox.c
    event_handler.c
    window.c
    layout.c
    hook.c
    tag.c
    binding.c
    config_file.c
    module.c
    ewmh.c
    work_area.c
    keyboard_mapping.c

    hashtable.c
)

include_directories(${velox_SOURCE_DIR})

target_link_libraries(velox
    ${XCB_LIBRARIES}
    ${XCB-Atom_LIBRARIES}
    ${XCB-ICCCM_LIBRARIES}
    ${XCB-Aux_LIBRARIES}
    ${XCB-EWMH_LIBRARIES}
    ${XCB-Image_LIBRARIES}
    ${XCB-Keysyms_LIBRARIES}
    ${LibYAML_LIBRARIES}
    ${X11_LIBRARIES}
    pthread
    dl
    m
)

install(TARGETS velox DESTINATION bin)

install(FILES
    config_file.h
    hook.h
    binding.h
    modifier.h
    layout.h
    velox.h
    tag.h
    window.h
    module.h
    ewmh.h
    work_area.h
    keyboard_mapping.h
    debug.h

    # Utility headers
    area.h
    vector.h
    hashtable.h
    list.h

    DESTINATION include/velox
)

