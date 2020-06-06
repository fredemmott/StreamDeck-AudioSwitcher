set(
  SOURCES
  ESDConnectionManager.cpp
  ESDLocalizer.cpp
  ESDMain.cpp
)
if (WIN32)
  list(APPEND SOURCES ESDUtilitiesWindows.cpp ESDCommonWindows.cpp)
elseif (APPLE)
  list(APPEND SOURCES ESDUtilitiesMac.cpp)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

add_library(
  StreamDeckSDK
  STATIC
  ${SOURCES}
)
target_link_libraries(
  StreamDeckSDK
  PRIVATE
  asio
  json
)
target_link_libraries(
  StreamDeckSDK
  PUBLIC
  websocketpp
)

if (APPLE)
  find_library(CORE_FOUNDATION_LIBRARY CoreFoundation REQUIRED)
  target_link_libraries(
    StreamDeckSDK
    PUBLIC
    ${CORE_FOUNDATION_LIBRARY}
  )
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_definitions(
    StreamDeckSDK
    PUBLIC
    "-DDEBUG=1"
  )
else()
  target_compile_definitions(
    StreamDeckSDK
    PUBLIC
    "-DDEBUG=0"
  )
endif()
