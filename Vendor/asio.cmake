include(ExternalProject)

ExternalProject_Add(
  asio_source
  URL https://github.com/chriskohlhoff/asio/archive/asio-1-16-1.tar.gz
  URL_HASH SHA512=7e5f8a503b6e8e939b3e77921bea2a11312dbe2ec609669c387ff11ebb97c2fbba96a57d064b34946b3db2cd45de6524a39d3050fd5b7d5b7f4fb595848a27ed
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(asio_source SOURCE_DIR)
add_library(asio INTERFACE)
add_dependencies(asio asio_source)
target_compile_definitions(
  asio
  INTERFACE
  ASIO_STANDALONE=1
)
target_include_directories(
  asio
  INTERFACE
  ${SOURCE_DIR}/asio/include
)
