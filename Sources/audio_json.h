/* Copyright (c) 2018-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */
#pragma once

#include <nlohmann/json.hpp>

namespace FredEmmott::Audio {

enum class AudioDeviceState;
void from_json(const nlohmann::json&, AudioDeviceState&);
void to_json(nlohmann::json&, const AudioDeviceState&);

struct AudioDeviceInfo;
void to_json(nlohmann::json&, const AudioDeviceInfo&);
void from_json(const nlohmann::json&, AudioDeviceInfo&);

}// namespace FredEmmott::Audio
