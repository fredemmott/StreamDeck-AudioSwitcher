#pragma once

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

std::map<std::string, std::string> GetAudioDeviceList(Direction);

std::string GetDefaultAudioDeviceID(Direction, Role);
void SetDefaultAudioDeviceID(Direction, Role, const std::string& deviceID);