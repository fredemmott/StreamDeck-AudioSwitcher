include(FetchContent)

FetchContent_Declare(
  AudioDeviceLib
  URL https://github.com/fredemmott/AudioDeviceLib/releases/download/v2.0.0/AudioDeviceLib-v2.0.0.zip
  URL_HASH SHA512=10ac9f13c4238d8407fffa44b310cb049090251b6acb5170b38a4122926fd5b209aab7c7528a23497d409c17c770037b15bd00953b223e2c3b7403dd6583a495
)

FetchContent_GetProperties(AudioDeviceLib)
if(NOT audiodevicelib_POPULATED)
  FetchContent_Populate(AudioDeviceLib)
  add_subdirectory("${audiodevicelib_SOURCE_DIR}" "${audiodevicelib_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()
