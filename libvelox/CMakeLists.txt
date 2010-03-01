# velox: libvelox/CMakeLists.txt

add_library(libvelox STATIC
    hashtable.c
)

set_target_properties(libvelox PROPERTIES OUTPUT_NAME velox)

install(FILES
    hashtable.h

    DESTINATION include/libvelox
)

