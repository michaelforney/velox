# mwm: plugins/CMakeLists.txt

add_library(mwm_wallpaper MODULE wallpaper.c)
set_target_properties(mwm_wallpaper PROPERTIES PREFIX "")
install(TARGETS mwm_wallpaper DESTINATION lib${LIB_SUFFIX}/mwm)


