/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include <functional>
#include <map>
#include <stdexcept>
#include <string>

namespace FredEmmott::Audio {

class device_error : public std::runtime_error {
 protected:
  device_error(const char* what) : std::runtime_error(what) {
  }
};

class operation_not_supported_error : public device_error {
 public:
  operation_not_supported_error() : device_error("Operation not supported") {
  }
};

class device_not_available_error : public device_error {
 public:
  device_not_available_error() : device_error("Device not available") {
  }
};

enum class AudioDeviceRole {
  DEFAULT,
  COMMUNICATION,
};

enum class AudioDeviceDirection {
  OUTPUT,
  INPUT,
};

enum class AudioDeviceState {
  CONNECTED,
  DEVICE_NOT_PRESENT,// USB device unplugged
  DEVICE_DISABLED,
  DEVICE_PRESENT_NO_CONNECTION,// device present, but nothing's plugged into it,
                               // e.g. headphone jack with nothing plugged in
};

struct AudioDeviceInfo {
  std::string id;
  std::string interfaceName;// e.g. "Generic USB Audio Device"
  std::string endpointName;// e.g. "Speakers"
  std::string displayName;// e.g. "Generic USB Audio Device (Speakers)"
  AudioDeviceDirection direction;
  AudioDeviceState state;
};

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(AudioDeviceDirection);
AudioDeviceState GetAudioDeviceState(const std::string& id);

std::string GetDefaultAudioDeviceID(AudioDeviceDirection, AudioDeviceRole);
void SetDefaultAudioDeviceID(
  AudioDeviceDirection,
  AudioDeviceRole,
  const std::string& deviceID);

bool IsAudioDeviceMuted(const std::string& deviceID);
void MuteAudioDevice(const std::string& deviceID);
void UnmuteAudioDevice(const std::string& deviceID);

template <class TImpl>
class AudioDeviceCallbackHandle {
 public:
  AudioDeviceCallbackHandle(TImpl* impl) : mImpl(impl) {
  }
  ~AudioDeviceCallbackHandle() {
  }

  AudioDeviceCallbackHandle(const AudioDeviceCallbackHandle& other) = delete;
  AudioDeviceCallbackHandle& operator=(const AudioDeviceCallbackHandle& other)
    = delete;

 private:
  std::unique_ptr<TImpl> mImpl;
};

struct MuteCallbackHandleImpl;
class MuteCallbackHandle final
  : public AudioDeviceCallbackHandle<MuteCallbackHandleImpl> {
 public:
  MuteCallbackHandle(MuteCallbackHandleImpl* impl);
  ~MuteCallbackHandle();
};

std::unique_ptr<MuteCallbackHandle> AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)>);

struct DefaultChangeCallbackHandleImpl;
class DefaultChangeCallbackHandle final
  : public AudioDeviceCallbackHandle<DefaultChangeCallbackHandleImpl> {
 public:
  DefaultChangeCallbackHandle(DefaultChangeCallbackHandleImpl* impl);
  ~DefaultChangeCallbackHandle();
};

std::unique_ptr<DefaultChangeCallbackHandle>
  AddDefaultAudioDeviceChangeCallback(
    std::function<
      void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>);

}// namespace FredEmmott::Audio
