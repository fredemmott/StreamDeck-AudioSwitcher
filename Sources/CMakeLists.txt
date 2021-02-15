set(CMAKE_CXX_STANDARD 20)

set(
  SOURCES
  AudioSwitcherStreamDeckPlugin.cpp
  main.cpp
)
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
