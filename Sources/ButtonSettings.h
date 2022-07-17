/* Copyright (c) 2018-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#pragma once

#include <AudioDevices/AudioDevices.h>

#include <nlohmann/json.hpp>

using namespace FredEmmott::Audio;

enum class DeviceMatchStrategy {
  ID,
  Fuzzy,
};

struct ButtonSettings {
  AudioDeviceDirection direction = AudioDeviceDirection::INPUT;
  AudioDeviceRole role = AudioDeviceRole::DEFAULT;
  AudioDeviceInfo primaryDevice;
  AudioDeviceInfo secondaryDevice;
  DeviceMatchStrategy matchStrategy = DeviceMatchStrategy::ID;

  // Changes if there's a fuzzy match
  std::string VolatilePrimaryID() const;
  std::string VolatileSecondaryID() const;
};

void from_json(const nlohmann::json&, ButtonSettings&);
void to_json(nlohmann::json&, const ButtonSettings&);
