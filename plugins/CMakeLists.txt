# mwm: plugins/CMakeLists.txt

set(MWM_PLUGINS wallpaper)

include_directories(${mwm_SOURCE_DIR})

foreach(PLUGIN ${MWM_PLUGINS})
    add_library(mwm_${PLUGIN} MODULE ${PLUGIN}.c)
    set_target_properties(mwm_${PLUGIN} PROPERTIES PREFIX "")
    install(TARGETS mwm_${PLUGIN} DESTINATION lib${LIB_SUFFIX}/mwm/plugins)
endforeach()

