include(ExternalProject)

ExternalProject_Add(
  StreamDeckSDK_build
  URL https://github.com/fredemmott/StreamDeck-CPPSDK/releases/download/v1.0/StreamDeckSDK-v1.0.zip
  URL_HASH SHA512=3a2c5e2c0fc7ea4dca6e6a0a36bfe9726bcbbf9248d70bac1542362bcdc395ba26713891082798346a7d17035bba366fae9d6f371c9fcff227ea8d19a23a07c8
  CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

ExternalProject_Get_Property(
  StreamDeckSDK_build
  INSTALL_DIR
)
add_library(StreamDeckSDK INTERFACE)
add_dependencies(StreamDeckSDK StreamDeckSDK_build)
target_link_libraries(
  StreamDeckSDK
  INTERFACE
  ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}StreamDeckSDK${CMAKE_STATIC_LIBRARY_SUFFIX}
)
target_include_directories(StreamDeckSDK INTERFACE ${INSTALL_DIR}/include)
target_compile_definitions(StreamDeckSDK INTERFACE -DASIO_STANDALONE=1)

if (APPLE)
  find_library(CORE_FOUNDATION_LIBRARY CoreFoundation REQUIRED)
  target_link_libraries(
    StreamDeckSDK
    INTERFACE
    ${CORE_FOUNDATION_LIBRARY}
  )
endif()
