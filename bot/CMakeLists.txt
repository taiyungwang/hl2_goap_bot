include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(bot_main PluginAdaptor.cpp)

add_subdirectory(player)
add_subdirectory(move)
add_subdirectory(mods)
add_subdirectory(goap)
add_subdirectory(event)
add_subdirectory(weapon)
