# mwm: libmwm/CMakeLists.txt

add_library(libmwm SHARED
    list.c
    loop.c
    hashtable.c
)

set_target_properties(libmwm PROPERTIES OUTPUT_NAME mwm)

install(TARGETS libmwm DESTINATION lib${LIB_SUFFIX})

install(FILES
    list.h
    loop.h
    hashtable.h

    DESTINATION include/libmwm
)

