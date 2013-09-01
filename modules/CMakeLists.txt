# velox: modules/CMakeLists.txt

set(VELOX_MODULES
    tile
    grid
    wallpaper
    spawn
    mpd
    alsa
    #bar
    pulseaudio
    desktop
)

include_directories(${velox_SOURCE_DIR})

macro(VELOX_MODULE _name)
    add_library(${_name} MODULE ${ARGN})
    set_target_properties(${_name} PROPERTIES PREFIX "")
    install(TARGETS ${_name} DESTINATION lib${LIB_SUFFIX}/velox/modules)
endmacro()

foreach(module ${VELOX_MODULES})
    string(TOUPPER ${module} module_upper)
    if(ENABLE_${module_upper})
        velox_module(${module} ${module}.c)
    endif()
endforeach()

if(ENABLE_MPD)
    target_link_libraries(mpd ${LIBMPDCLIENT_LIBRARIES})
endif()

if(ENABLE_ALSA)
    target_link_libraries(alsa ${ALSA_LIBRARIES})
    include_directories(${ALSA_INCLUDE_DIR})
endif()

if(ENABLE_PULSEAUDIO)
    target_link_libraries(pulseaudio ${LIBPULSE_LIBRARIES})
    include_directories(${LibPulse_INCLUDE_DIR})
endif()

