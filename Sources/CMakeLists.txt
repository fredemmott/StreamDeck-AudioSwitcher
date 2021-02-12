set(CMAKE_CXX_STANDARD 20)

set(
  SOURCES
  MyStreamDeckPlugin.cpp
  main.cpp
)
if(WIN32)
  list(APPEND SOURCES AudioDevicesWindows.cpp StringEncoding.cpp)
endif()
if(APPLE)
  list(APPEND SOURCES AudioDevicesMacOS.cpp)
endif()

add_executable(
  sdaudioswitch
  ${SOURCES}
)
target_link_libraries(sdaudioswitch StreamDeckSDK)
if(APPLE)
  find_library(COREAUDIO_FRAMEWORK CoreAudio)
  find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
  target_link_libraries(sdaudioswitch ${COREAUDIO_FRAMEWORK} ${COREFOUNDATION_FRAMEWORK})
endif()
sign_target(sdaudioswitch)
install(TARGETS sdaudioswitch DESTINATION ${CMAKE_INSTALL_PREFIX})
