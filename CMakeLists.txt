cmake_minimum_required(VERSION 3.15)

# CMAKE_MSVC_RUNTIME_LIBRARY support
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(
  com.fredemmott.audiooutputswitch.sdPlugin
  LANGUAGES CXX
  VERSION 3.0
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)
if (MSVC)
  add_definitions("/Zc:__cplusplus -DUNICODE=1")
endif()

include_directories("${CMAKE_SOURCE_DIR}")

include("AudioDeviceLib.cmake")
include("StreamDeckSDK.cmake")
include("sign_target.cmake")

set_default_install_dir_to_streamdeck_plugin_dir()

add_subdirectory(Sources)
add_subdirectory(sdPlugin)

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX})
