# velox: velox/CMakeLists.txt

set(VELOX_SOURCES
    binding.c
    bound_layer.c
    config_file.c
    free_layer.c
    hook.c
    layer.c
    layout.c
    module.c
    operations-dummy.c
    resource.c
    velox.c
    window.c
    work_area.c
    workspace.c

    hashtable.c
)

add_executable(velox ${VELOX_SOURCES})

include_directories(${velox_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(velox
    ${LIBYAML_LIBRARIES}
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

