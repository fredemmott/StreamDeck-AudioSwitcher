#pragma once

#include <map>
#include <string>

std::map<std::string, std::string> GetAudioDeviceList();

std::string GetDefaultAudioDeviceID();
void SetDefaultAudioDeviceID(const std::string& deviceID);