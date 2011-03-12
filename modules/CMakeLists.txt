# velox: modules/CMakeLists.txt

set(VELOX_MODULES
    layout_tile
    layout_grid
    wallpaper
    spawn
    mpd
    alsa
    bar
    pulseaudio
)

include_directories(${velox_SOURCE_DIR})

foreach(module ${VELOX_MODULES})
    string(TOUPPER ${module} module_upper)
    if(ENABLE_${module_upper})
        add_library(velox_${module} MODULE ${module}.c)
        set_target_properties(velox_${module} PROPERTIES PREFIX "")
        install(TARGETS velox_${module} DESTINATION lib${LIB_SUFFIX}/velox/modules)
    endif(ENABLE_${module_upper})
endforeach(module)

if(ENABLE_MPD)
    target_link_libraries(velox_mpd ${LibMPDClient_LIBRARIES})
endif(ENABLE_MPD)

if(ENABLE_ALSA)
    target_link_libraries(velox_alsa ${ALSA_LIBRARIES})
    include_directories(${ALSA_INCLUDE_DIR})
endif(ENABLE_ALSA)

if(ENABLE_PULSEAUDIO)
    target_link_libraries(velox_pulseaudio ${LibPulse_LIBRARIES})
    include_directories(${LibPulse_INCLUDE_DIR})
endif(ENABLE_PULSEAUDIO)

