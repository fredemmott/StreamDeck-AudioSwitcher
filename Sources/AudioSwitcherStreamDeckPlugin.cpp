//==============================================================================
/**
@file       AudioSwitcherStreamDeckPlugin.cpp

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include "AudioSwitcherStreamDeckPlugin.h"

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include <atomic>
#include <mutex>

#ifdef _MSC_VER
#include <objbase.h>
#endif

using namespace FredEmmott::Audio;

namespace {
const char* SET_ACTION_ID = "com.fredemmott.audiooutputswitch.set";
const char* TOGGLE_ACTION_ID = "com.fredemmott.audiooutputswitch.toggle";
}// namespace

namespace FredEmmott::Audio {
void to_json(json& j, const AudioDeviceState& state) {
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

void from_json(const json& j, AudioDeviceState& state) {
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
    state == AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION;
    return;
  }
}

void to_json(json& j, const AudioDeviceInfo& device) {
  j = json(
    {{"id", device.id},
     {"interfaceName", device.interfaceName},
     {"endpointName", device.endpointName},
     {"displayName", device.displayName},
     {"state", device.state}});
}

void from_json(const json& j, AudioDeviceInfo& device) {
  device = {
    .id = j.at("id"),
    .interfaceName = j.at("interfaceName"),
    .endpointName = j.at("endpointName"),
    .displayName = j.at("displayName"),
    .state = j.at("state"),
  };
}

}// namespace FredEmmott::Audio

void to_json(json& j, const AudioDeviceState& state) {
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

AudioSwitcherStreamDeckPlugin::AudioSwitcherStreamDeckPlugin() {
#ifdef _MSC_VER
  CoInitializeEx(
    NULL, COINIT_MULTITHREADED);// initialize COM for the main thread
#endif
  mCallbackHandle = std::move(AddDefaultAudioDeviceChangeCallback(std::bind(
    &AudioSwitcherStreamDeckPlugin::OnDefaultDeviceChanged, this,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
}

AudioSwitcherStreamDeckPlugin::~AudioSwitcherStreamDeckPlugin() {
  mCallbackHandle = {};
}

void AudioSwitcherStreamDeckPlugin::OnDefaultDeviceChanged(
  AudioDeviceDirection direction,
  AudioDeviceRole role,
  const std::string& device) {
  std::scoped_lock lock(mVisibleContextsMutex);
  for (const auto& [context, button] : mButtons) {
    if (button.settings.direction != direction) {
      continue;
    }
    if (button.settings.role != role) {
      continue;
    }
    UpdateState(context, device);
  }
}

void AudioSwitcherStreamDeckPlugin::KeyDownForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  const auto state = EPLJSONUtils::GetIntByName(inPayload, "state");
}

void AudioSwitcherStreamDeckPlugin::KeyUpForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  ESDDebug("{}: {}", __FUNCTION__, inPayload.dump());
  std::scoped_lock lock(mVisibleContextsMutex);

  const auto settings = ButtonSettingsFromJSON(inPayload);
  mButtons[inContext].settings = settings;
  const auto state = EPLJSONUtils::GetIntByName(inPayload, "state");
  // this looks inverted - but if state is 0, we want to move to state 1, so
  // we want the secondary devices. if state is 1, we want state 0, so we want
  // the primary device
  const auto device = (state != 0 || inAction == SET_ACTION_ID)
                        ? settings.primaryDevice
                        : settings.secondaryDevice;
  const auto& deviceId = device.id;
  if (deviceId.empty()) {
    ESDDebug("Doing nothing, no device ID");
    return;
  }

  const auto deviceState = GetAudioDeviceState(deviceId);
  if (deviceState != AudioDeviceState::CONNECTED) {
    if (inAction == SET_ACTION_ID) {
      mConnectionManager->SetState(1, inContext);
    }
    mConnectionManager->ShowAlertForContext(inContext);
    return;
  }

  if (
    inAction == SET_ACTION_ID
    && deviceId == GetDefaultAudioDeviceID(settings.direction, settings.role)) {
    // We already have the correct device, undo the state change
    mConnectionManager->SetState(state, inContext);
    ESDDebug("Already set, nothing to do");
    return;
  }

  ESDDebug("Setting device to {}", deviceId);
  SetDefaultAudioDeviceID(settings.direction, settings.role, deviceId);
}

void AudioSwitcherStreamDeckPlugin::WillAppearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  std::scoped_lock lock(mVisibleContextsMutex);
  // Remember the context
  mVisibleContexts.insert(inContext);
  const auto settings = ButtonSettingsFromJSON(inPayload);
  mButtons[inContext] = {inAction, inContext, settings};
  UpdateState(inContext);
}

void AudioSwitcherStreamDeckPlugin::WillDisappearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // Remove the context
  std::scoped_lock lock(mVisibleContextsMutex);
  mVisibleContexts.erase(inContext);
  mButtons.erase(inContext);
}

void AudioSwitcherStreamDeckPlugin::SendToPlugin(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  json outPayload;

  const auto event = EPLJSONUtils::GetStringByName(inPayload, "event");
  ESDDebug("Received event {}", event);

  if (event == "getDeviceList") {
    const auto outputList = GetAudioDeviceList(AudioDeviceDirection::OUTPUT);
    const auto inputList = GetAudioDeviceList(AudioDeviceDirection::INPUT);
    mConnectionManager->SendToPropertyInspector(
      inAction, inContext,
      json({
        {"event", event},
        {"outputDevices", outputList},
        {"inputDevices", inputList},
      }));
    return;
  }
}

AudioSwitcherStreamDeckPlugin::ButtonSettings
AudioSwitcherStreamDeckPlugin::ButtonSettingsFromJSON(const json& inPayload) {
  if (!inPayload.contains("settings")) {
    return {};
  }
  const auto jsonSettings = inPayload.at("settings");

  if (!jsonSettings.contains("direction")) {
    return {};
  }

  ButtonSettings settings;

  if (jsonSettings.contains("direction")) {
    settings.direction
      = (jsonSettings.at("direction").get<std::string>() == "output")
          ? AudioDeviceDirection::OUTPUT
          : AudioDeviceDirection::INPUT;
  }

  if (jsonSettings.contains("role")) {
    settings.role
      = (jsonSettings.at("role").get<std::string>() == "communication")
          ? AudioDeviceRole::COMMUNICATION
          : AudioDeviceRole::DEFAULT;
  }

  if (jsonSettings.contains("primary")) {
    const auto& primary = jsonSettings.at("primary");
    if (primary.is_string()) {
      settings.primaryDevice.id = primary;
    } else {
      settings.primaryDevice = primary;
    }
  }

  if (jsonSettings.contains("secondary")) {
    const auto& secondary = jsonSettings.at("secondary");
    if (secondary.is_string()) {
      settings.secondaryDevice.id = secondary;
    } else {
      settings.secondaryDevice = secondary;
    }
  }

  return settings;
}

void AudioSwitcherStreamDeckPlugin::UpdateState(
  const std::string& context,
  const std::string& optionalDefaultDevice) {
  const auto button = mButtons[context];
  const auto action = button.action;
  const auto settings = button.settings;
  const auto activeDevice
    = optionalDefaultDevice.empty()
        ? GetDefaultAudioDeviceID(settings.direction, settings.role)
        : optionalDefaultDevice;
  ESDDebug(
    "setting active ID {} {} {}", activeDevice, settings.primaryDevice.id,
    settings.secondaryDevice.id);

  std::scoped_lock lock(mVisibleContextsMutex);
  if (action == SET_ACTION_ID) {
    mConnectionManager->SetState(
      activeDevice == settings.primaryDevice.id ? 0 : 1, context);
    return;
  }

  if (activeDevice == settings.primaryDevice.id) {
    mConnectionManager->SetState(0, context);
  } else if (activeDevice == settings.secondaryDevice.id) {
    mConnectionManager->SetState(1, context);
  } else {
    mConnectionManager->ShowAlertForContext(context);
  }
}

void AudioSwitcherStreamDeckPlugin::DeviceDidConnect(
  const std::string& inDeviceID,
  const json& inDeviceInfo) {
  // Nothing to do
}

void AudioSwitcherStreamDeckPlugin::DeviceDidDisconnect(
  const std::string& inDeviceID) {
  // Nothing to do
}

void AudioSwitcherStreamDeckPlugin::DidReceiveGlobalSettings(
  const json& inPayload) {
}

void AudioSwitcherStreamDeckPlugin::DidReceiveSettings(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  WillAppearForAction(inAction, inContext, inPayload, inDeviceID);
}
