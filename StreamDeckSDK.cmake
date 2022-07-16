include(FetchContent)

FetchContent_Declare(
  StreamDeckSDK
  URL https://github.com/fredemmott/StreamDeck-CPPSDK/releases/download/v3.0.0/StreamDeckSDK-v3.0.0.zip
  URL_HASH SHA512=7ecd21731e112762215ba93830ee263cd0b434e1477406e973d1ee329f098baef4fd496ba1687f9ac6aff9a1167bcb1c52cfde9e6289b855de8b15e26414fc19
)

FetchContent_GetProperties(StreamDeckSDK)
if(NOT streamdecksdk_POPULATED)
  FetchContent_Populate(StreamDeckSDK)
  add_subdirectory("${streamdecksdk_SOURCE_DIR}" "${streamdecksdk_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()

if(APPLE)
  set(
    STREAMDECK_PLUGIN_DIR
    "$ENV{HOME}/Library/Application Support/com.elgato.StreamDeck/Plugins"
  )
elseif(WIN32)
  string(
    REPLACE
    "\\"
    "/"
    STREAMDECK_PLUGIN_DIR
    "$ENV{appdata}/Elgato/StreamDeck/Plugins"
  )
endif()

set(
  STREAMDECK_PLUGIN_DIR
  ${STREAMDECK_PLUGIN_DIR}
  CACHE PATH "Path to this system's streamdeck plugin directory"
)

function(set_default_install_dir_to_streamdeck_plugin_dir)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(
      CMAKE_INSTALL_PREFIX
      "${STREAMDECK_PLUGIN_DIR}/${CMAKE_PROJECT_NAME}"
      CACHE PATH "See cmake documentation"
      FORCE
    )
  endif()
endfunction()
