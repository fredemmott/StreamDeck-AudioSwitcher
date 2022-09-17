include(FetchContent)

FetchContent_Declare(
  AudioDeviceLib
  URL https://github.com/fredemmott/AudioDeviceLib/releases/download/v2.1.1/AudioDeviceLib-v2.1.1.zip
  URL_HASH SHA512=7f59d62a094eb140008f7d9d6ab72b2a7a360c23a5dee47f533c96ae3e65762acc6e927a8fccbad5adfc247790fc7bb92a3ff204c1b48fefb7a6f3df8243d5fa
)

FetchContent_GetProperties(AudioDeviceLib)
if(NOT audiodevicelib_POPULATED)
  FetchContent_Populate(AudioDeviceLib)
  add_subdirectory("${audiodevicelib_SOURCE_DIR}" "${audiodevicelib_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()
