# mwm: modules/CMakeLists.txt

set(MWM_MODULES
    layout_tile
    layout_grid
    wallpaper
    spawn
)

include_directories(${mwm_SOURCE_DIR})

foreach(module ${MWM_MODULES})
    add_library(mwm_${module} MODULE ${module}.c)
    set_target_properties(mwm_${module} PROPERTIES PREFIX "")
    install(TARGETS mwm_${module} DESTINATION lib${LIB_SUFFIX}/mwm/modules)
endforeach(module)

