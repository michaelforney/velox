# velox: modules/CMakeLists.txt

set(VELOX_MODULES
    layout_tile
    layout_grid
    wallpaper
    spawn
)

include_directories(${velox_SOURCE_DIR})

foreach(module ${VELOX_MODULES})
    add_library(velox_${module} MODULE ${module}.c)
    set_target_properties(velox_${module} PROPERTIES PREFIX "")
    install(TARGETS velox_${module} DESTINATION lib${LIB_SUFFIX}/velox/modules)
endforeach(module)

