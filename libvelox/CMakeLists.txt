# velox: libvelox/CMakeLists.txt

add_library(libvelox STATIC
    area.c
    hashtable.c
    vector.c
)

set_target_properties(libvelox PROPERTIES OUTPUT_NAME velox)

install(FILES
    area.h
    hashtable.h
    vector.h

    DESTINATION include/libvelox
)

