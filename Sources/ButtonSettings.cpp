/* Copyright (c) 2018-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "ButtonSettings.h"

#include "audio_json.h"

void from_json(const nlohmann::json& j, ButtonSettings& bs) {
  if (!j.contains("direction")) {
    return;
  }

  bs.direction = (j.at("direction").get<std::string>() == "output")
                   ? AudioDeviceDirection::OUTPUT
                   : AudioDeviceDirection::INPUT;

  if (j.contains("role")) {
    bs.role = (j.at("role").get<std::string>() == "communication")
                ? AudioDeviceRole::COMMUNICATION
                : AudioDeviceRole::DEFAULT;
  }

  if (j.contains("primary")) {
    const auto& primary = j.at("primary");
    if (primary.is_string()) {
      bs.primaryDevice.id = primary;
    } else {
      bs.primaryDevice = primary;
    }
  }

  if (j.contains("secondary")) {
    const auto& secondary = j.at("secondary");
    if (secondary.is_string()) {
      bs.secondaryDevice.id = secondary;
    } else {
      bs.secondaryDevice = secondary;
    }
  }
}

void to_json(nlohmann::json&, const ButtonSettings&) {
  throw std::logic_error("Unimplemented");
}
