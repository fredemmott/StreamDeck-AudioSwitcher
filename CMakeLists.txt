cmake_minimum_required(VERSION 3.10)

project(com.fredemmott.audiooutputswitch.sdPlugin VERSION 2.4)

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

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)
if (MSVC)
  add_definitions("/Zc:__cplusplus -DUNICODE=1")
endif()
include_directories("${CMAKE_SOURCE_DIR}")

include("StreamDeckSDK.cmake")

add_subdirectory(Sources)
add_subdirectory(sdplugin)
