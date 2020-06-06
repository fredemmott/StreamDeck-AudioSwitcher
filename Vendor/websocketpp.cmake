include(ExternalProject)

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
