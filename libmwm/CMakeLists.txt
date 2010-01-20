# mwm: libmwm/CMakeLists.txt

add_library(libmwm SHARED
    tag.c
)

set_target_properties(libmwm PROPERTIES OUTPUT_NAME mwm)

