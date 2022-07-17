set(CMAKE_CXX_STANDARD 20)

set(
  SOURCES
  audio_json.cpp
  AudioSwitcherStreamDeckPlugin.cpp
  ButtonSettings.cpp
  main.cpp
)

if(WIN32)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
    @ONLY
  )
  list(APPEND SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version.rc")
endif()

add_executable(
  sdaudioswitch
  ${SOURCES}
)
target_link_libraries(sdaudioswitch AudioDeviceLib StreamDeckSDK)
sign_target(sdaudioswitch)
install(TARGETS sdaudioswitch DESTINATION ${CMAKE_INSTALL_PREFIX})

if(WIN32)
  install(FILES "$<TARGET_PDB_FILE:sdaudioswitch>" DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()
