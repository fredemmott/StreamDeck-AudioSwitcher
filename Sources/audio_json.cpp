/* Copyright (c) 2018-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#include "audio_json.h"

#include <AudioDevices/AudioDevices.h>

namespace FredEmmott::Audio {

void from_json(const nlohmann::json& j, AudioDeviceState& state) {
  if (j == "connected") {
    state = AudioDeviceState::CONNECTED;
    return;
  }
  if (j == "device_not_present") {
    state = AudioDeviceState::DEVICE_NOT_PRESENT;
    return;
  }
  if (j == "device_disabled") {
    state = AudioDeviceState::DEVICE_DISABLED;
    return;
  }
  if (j == "device_present_no_connection") {
    state = AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION;
    return;
  }
}

void to_json(nlohmann::json& j, const AudioDeviceState& state) {
  switch (state) {
    case AudioDeviceState::CONNECTED:
      j = "connected";
      return;
    case AudioDeviceState::DEVICE_NOT_PRESENT:
      j = "device_not_present";
      return;
    case AudioDeviceState::DEVICE_DISABLED:
      j = "device_disabled";
      return;
    case AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION:
      j = "device_present_no_connection";
      return;
  }
}

void to_json(nlohmann::json& j, const AudioDeviceInfo& device) {
  j = {
    {"id", device.id},
    {"interfaceName", device.interfaceName},
    {"endpointName", device.endpointName},
    {"displayName", device.displayName},
    {"direction", device.direction},
    {"state", device.state},
  };
}

void from_json(const nlohmann::json& j, AudioDeviceInfo& device) {
  device = {
    .id = j.at("id"),
    .interfaceName = j.at("interfaceName"),
    .endpointName = j.at("endpointName"),
    .displayName = j.at("displayName"),
    .direction = j.at("direction"),
    .state = j.at("state"),
  };
}

void from_json(const nlohmann::json& j, AudioDeviceDirection& d) {
  d = (j == "output") ? AudioDeviceDirection::OUTPUT
                      : AudioDeviceDirection::INPUT;
}

void to_json(nlohmann::json& j, const AudioDeviceDirection& d) {
  switch (d) {
    case AudioDeviceDirection::OUTPUT:
      j = "output";
      return;
    case AudioDeviceDirection::INPUT:
      j = "input";
      return;
  }
}

void from_json(const nlohmann::json& j, AudioDeviceRole& r) {
  r = (j == "communication") ? AudioDeviceRole::COMMUNICATION
                             : AudioDeviceRole::DEFAULT;
}

void to_json(nlohmann::json& j, const AudioDeviceRole& r) {
  switch (r) {
    case AudioDeviceRole::COMMUNICATION:
      j = "communication";
      return;
    case AudioDeviceRole::DEFAULT:
      j = "default";
      return;
  }
}

}// namespace FredEmmott::Audio
