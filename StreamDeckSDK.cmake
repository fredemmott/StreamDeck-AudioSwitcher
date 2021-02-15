include(ExternalProject)

ExternalProject_Add(
  StreamDeckSDK_build
  URL https://github.com/fredemmott/StreamDeck-CPPSDK/releases/download/v2.0-rc11/StreamDeckSDK-v2.0-rc11.zip
  URL_HASH SHA512=50bad0da62b3a60c038250955092419630ae3e6ac33aae362ce84b1d0cb0cc720ff0467544241d21bdc4c3ca64dfd6a7a041af44b8d4a8d6304ba0b0d12e7adc
  CMAKE_ARGS
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
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
  "${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}fmt$<$<CONFIG:Debug>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
target_include_directories(StreamDeckSDK INTERFACE ${INSTALL_DIR}/include)
target_compile_definitions(StreamDeckSDK INTERFACE -DASIO_STANDALONE=1)
