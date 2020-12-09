set(CMAKE_CXX_STANDARD 20)

set(
  SOURCES
  AudioFunctions.cpp
  MyStreamDeckPlugin.cpp
  main.cpp
)

add_executable(
  sdaudioswitch
  ${SOURCES}
)
target_link_libraries(sdaudioswitch StreamDeckSDK)
sign_target(sdaudioswitch)
install(TARGETS sdaudioswitch DESTINATION ${CMAKE_INSTALL_PREFIX})
