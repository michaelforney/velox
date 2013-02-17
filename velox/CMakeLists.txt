# velox: velox/CMakeLists.txt

add_executable(velox
    atom.c
    binding.c
    config_file.c
    event_handler.c
    ewmh.c
    hook.c
    keyboard_mapping.c
    layout.c
    module.c
    velox.c
    window.c
    work_area.c
    workspace.c

    hashtable.c
)

include_directories(${velox_SOURCE_DIR})

target_link_libraries(velox
    ${XCB_LIBRARIES}
    ${XCB_UTIL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    ${X11_LIBRARIES}
    pthread
    dl
    m
)

install(TARGETS velox DESTINATION bin)

install(FILES
    atom.h
    config_file.h
    hook.h
    binding.h
    modifier.h
    layout.h
    velox.h
    workspace.h
    window.h
    module.h
    ewmh.h
    work_area.h
    keyboard_mapping.h
    function.h
    argument.h
    debug.h

    # Utility headers
    area.h
    vector.h
    hashtable.h
    list.h

    DESTINATION include/velox
)

