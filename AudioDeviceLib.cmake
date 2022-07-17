include(FetchContent)

FetchContent_Declare(
  AudioDeviceLib
  URL https://github.com/fredemmott/AudioDeviceLib/releases/download/v2.1.0/AudioDeviceLib-v2.1.0.zip
  URL_HASH SHA512=ecd4be984caaee11af199c6d5eb02986c53908ce8ee60ebb83ce193182a94109f28742335f6d734110d583dd0e5c7f01a9a9fa54f615d9f3e351163c2ad7dce5
)

FetchContent_GetProperties(AudioDeviceLib)
if(NOT audiodevicelib_POPULATED)
  FetchContent_Populate(AudioDeviceLib)
  add_subdirectory("${audiodevicelib_SOURCE_DIR}" "${audiodevicelib_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()
