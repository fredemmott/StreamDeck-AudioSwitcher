##### asio #####

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

##### json #####

ExternalProject_Add(
  json_source
  URL "https://github.com/nlohmann/json/releases/download/v3.7.3/include.zip"
  URL_HASH SHA512=8efd82a54472335e548d0d5c375b6f2781b4a0f2dbc5aa0acc3f504277ec455e0782046286cf08eb4864ac8bcaa1a9691af8d0259dd71a9e539cfc12e0478eb5
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(json_source SOURCE_DIR)
add_library(json INTERFACE)
add_dependencies(json json_source)
target_include_directories(
 json 
  INTERFACE
  "${SOURCE_DIR}/single_include"
)

#### websocketpp #####

ExternalProject_Add(
  websocketpp_source
  URL https://github.com/zaphoyd/websocketpp/archive/0.8.2.tar.gz
  URL_HASH SHA512=b2afc63edb69ce81a3a6c06b3d857b3e8820f0e22300ac32bb20ab30ff07bd58bd5ada3e526ed8ab52de934e0e3a26cad2118b0e68ecf3e5e9e8d7101348fd06
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(websocketpp_source SOURCE_DIR)
add_library(websocketpp INTERFACE)
add_dependencies(websocketpp websocketpp_source)
target_include_directories(
  websocketpp
  INTERFACE
  "${SOURCE_DIR}"
)
target_link_libraries(
  websocketpp
  INTERFACE
  asio
)
target_compile_definitions(
  websocketpp
  INTERFACE
  ASIO_STANDALONE=1
)
