# velox: libvelox/CMakeLists.txt

add_library(libvelox SHARED
    area.c
    list.c
    loop.c
    hashtable.c
    vector.c
)

set_target_properties(libvelox PROPERTIES OUTPUT_NAME velox)

install(TARGETS libvelox DESTINATION lib${LIB_SUFFIX})

install(FILES
    list.h
    loop.h
    hashtable.h
    vector.h

    DESTINATION include/libvelox
)

