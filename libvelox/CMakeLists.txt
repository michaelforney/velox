# velox: libvelox/CMakeLists.txt

add_library(libvelox STATIC
    area.c
    list.c
    loop.c
    hashtable.c
    vector.c
)

set_target_properties(libvelox PROPERTIES OUTPUT_NAME velox)

install(FILES
    area.h
    list.h
    loop.h
    hashtable.h
    vector.h

    DESTINATION include/libvelox
)

