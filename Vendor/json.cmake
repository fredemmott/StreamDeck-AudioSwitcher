include(ExternalProject)

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
