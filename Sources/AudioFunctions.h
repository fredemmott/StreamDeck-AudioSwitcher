#pragma once

#include <functional>
#include <map>
#include <string>

enum class Role {
  DEFAULT,
  COMMUNICATION,
};

enum class Direction {
  OUTPUT,
  INPUT,
};

enum class MuteAction {
  UNMUTE,
  MUTE,
  TOGGLE,
};

std::map<std::string, std::string> GetAudioDeviceList(Direction);

std::string GetDefaultAudioDeviceID(Direction, Role);
void SetDefaultAudioDeviceID(Direction, Role, const std::string& deviceID);

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
  std::function<void(Direction, Role, const std::string&)>);
void RemoveDefaultAudioDeviceChangeCallback(
  DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE);
