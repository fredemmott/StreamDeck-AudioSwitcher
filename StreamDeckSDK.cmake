include(FetchContent)

FetchContent_Declare(
  StreamDeckSDK
  URL https://github.com/fredemmott/StreamDeck-CPPSDK/releases/download/v3.0.2/StreamDeckSDK-v3.0.2.zip
  URL_HASH SHA512=4881ca3b93f92cbd4542ccbff57ad2a2028dd00d1064fc9bc7eecbe04a9573c48efb23693304aee06eaab8bcdef9c263c01e2482a1be2d2f7be9cdaf6c10fd60
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
