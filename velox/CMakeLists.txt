# velox: velox/CMakeLists.txt

set(VELOX_SOURCES
    binding.c
    config_file.c
    hook.c
    layout.c
    module.c
    resource.c
    velox.c
    window.c
    work_area.c
    workspace.c
    layer.c
    bound_layer.c
    free_layer.c

    hashtable.c
)

if(WITH_X11)
    set(VELOX_SOURCES ${VELOX_SOURCES}
        x11/atom.c
        x11/event_handler.c
        x11/ewmh.c
        x11/keyboard_mapping.c
        x11/window.c
        x11/x11.c
    )
endif(WITH_X11)

add_executable(velox ${VELOX_SOURCES})

include_directories(${velox_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(velox
    ${XCB_LIBRARIES}
    ${XCB_UTIL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    ${X11_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
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
    workspace.h
    window.h
    module.h
    resource.h
    work_area.h
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

if(WITH_X11)
    install(FILES
        x11/atom.h
        x11/event_handler.h
        x11/ewmh.h
        x11/keyboard_mapping.h

        DESTINATION include/velox/x11
    )
endif(WITH_X11)

