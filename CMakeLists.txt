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

if (APPLE)
  set(
    STREAMDECK_PLUGIN_DIR
    "$ENV{HOME}/Library/ApplicationSupport/com.elgato.StreamDeck/Plugins"
  )
elseif (WIN32)
  STRING(
    REPLACE
    "\\"
    "/"
    STREAMDECK_PLUGIN_DIR
    "$ENV{appdata}/Elgato/StreamDeck/Plugins"
  )
endif()
message(STATUS "StreamDeck plugin dir: ${STREAMDECK_PLUGIN_DIR}")
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(
    CMAKE_INSTALL_PREFIX
    "${STREAMDECK_PLUGIN_DIR}/${CMAKE_PROJECT_NAME}"
    CACHE PATH "..." FORCE
  )
endif()
message(STATUS "Install dir: ${CMAKE_INSTALL_PREFIX}")

include_directories("${CMAKE_SOURCE_DIR}")

include("AudioDeviceLib.cmake")
include("StreamDeckSDK.cmake")
include("sign_target.cmake")

add_subdirectory(Sources)
add_subdirectory(sdPlugin)

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX})
