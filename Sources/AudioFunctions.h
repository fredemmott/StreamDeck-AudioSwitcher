#pragma once

#include <functional>
#include <map>
#include <string>

enum class AudioDeviceRole {
  DEFAULT,
  COMMUNICATION,
};

enum class AudioDeviceDirection {
  OUTPUT,
  INPUT,
};

enum class MuteAction {
  UNMUTE,
  MUTE,
  TOGGLE,
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
  std::string displayName;
  AudioDeviceDirection direction;
  AudioDeviceState state;
};

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(AudioDeviceDirection);

std::string GetDefaultAudioDeviceID(AudioDeviceDirection, AudioDeviceRole);
void SetDefaultAudioDeviceID(
  AudioDeviceDirection,
  AudioDeviceRole,
  const std::string& deviceID);

bool IsAudioDeviceMuted(const std::string& deviceID);
void SetIsAudioDeviceMuted(const std::string& deviceID, MuteAction);

typedef void* AUDIO_DEVICE_MUTE_CALLBACK_HANDLE;
AUDIO_DEVICE_MUTE_CALLBACK_HANDLE AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)>);
void RemoveAudioDeviceMuteUnmuteCallback(AUDIO_DEVICE_MUTE_CALLBACK_HANDLE);

typedef void* DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE;
DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE
AddDefaultAudioDeviceChangeCallback(
  std::function<
    void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>);
void RemoveDefaultAudioDeviceChangeCallback(
  DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE);
#ifdef HAVE_FEEDBACK_SOUNDS
void PlayFeedbackSound(MuteAction action);
#endif
