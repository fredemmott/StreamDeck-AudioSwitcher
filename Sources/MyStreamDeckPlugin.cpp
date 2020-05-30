//==============================================================================
/**
@file       MyStreamDeckPlugin.cpp

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include "MyStreamDeckPlugin.h"
#include <atomic>
#include <mutex>

#include "AudioFunctions.h"
#include "Common/EPLJSONUtils.h"
#include "Common/ESDConnectionManager.h"

#ifdef _MSVC_LANG
static_assert(_MSVC_LANG > 201402L, "C++17 not enabled in _MSVC_LANG");
static_assert(_HAS_CXX17, "C++17 feature flag not enabled");
#endif

namespace {
const char* SET_ACTION_ID = "com.fredemmott.audiooutputswitch.set";
const char* TOGGLE_ACTION_ID = "com.fredemmott.audiooutputswitch.toggle";
}// namespace

void to_json(json& j, const AudioDeviceInfo& device) {
  j = device.displayName;
}

MyStreamDeckPlugin::MyStreamDeckPlugin() {
  CoInitialize(NULL);// initialize COM for the main thread
}

MyStreamDeckPlugin::~MyStreamDeckPlugin() {
}

void MyStreamDeckPlugin::KeyDownForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  const auto state = EPLJSONUtils::GetIntByName(inPayload, "state");
}

void MyStreamDeckPlugin::KeyUpForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  DebugPrint("SDAudioSwitch: Key Up: %s", inPayload.dump().c_str());
  std::scoped_lock lock(mVisibleContextsMutex);

  const auto settings = ButtonSettingsFromJSON(inPayload);
  UpdateCallback(inAction, inContext, settings);
  // this looks inverted - but if state is 0, we want to move to state 1, so we
  // want the secondary devices. if state is 1, we want state 0, so we want the
  // primary device
  const auto state = EPLJSONUtils::GetIntByName(inPayload, "state");
  const auto deviceId = (state != 0 || inAction == SET_ACTION_ID)
                          ? settings.primaryDevice
                          : settings.secondaryDevice;
  if (deviceId.empty()) {
    return;
  }

  SetDefaultAudioDeviceID(settings.direction, settings.role, deviceId);
}

void MyStreamDeckPlugin::WillAppearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  std::scoped_lock lock(mVisibleContextsMutex);
  // Remember the context
  mVisibleContexts.insert(inContext);
  const auto settings = ButtonSettingsFromJSON(inPayload);
  DebugPrint(
    "SDAudioSwitch: Will appear: %s %s", settings.primaryDevice.c_str(),
    inAction.c_str());
  UpdateCallback(inAction, inContext, settings);
  UpdateState(inAction, inContext, settings);
}

void MyStreamDeckPlugin::WillDisappearForAction(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  // Remove the context
  mVisibleContextsMutex.lock();
  mVisibleContexts.erase(inContext);
  mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::SendToPlugin(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  json outPayload;

  const auto event = EPLJSONUtils::GetStringByName(inPayload, "event");
  DebugPrint("SDAudioSwitch: Received event %s", event.c_str());

  if (event == "getDeviceList") {
    const auto outputList = GetAudioDeviceList(AudioDeviceDirection::OUTPUT);
    DebugPrint("SDAudioSwitch: got output list");
    const auto inputList = GetAudioDeviceList(AudioDeviceDirection::INPUT);
    DebugPrint("SDAudioSwitch: got input list");
    DebugPrint(
      "SDAudioSwitch: device json: %s",
      json({{"outputDevices", GetAudioDeviceList(AudioDeviceDirection::OUTPUT)},
            {"inputDevices", GetAudioDeviceList(AudioDeviceDirection::INPUT)}})
        .dump()
        .c_str());
    mConnectionManager->SendToPropertyInspector(
      inAction, inContext,
      json(
        {{"event", event},
         {"outputDevices", GetAudioDeviceList(AudioDeviceDirection::OUTPUT)},
         {"inputDevices", GetAudioDeviceList(AudioDeviceDirection::INPUT)}}));
    return;
  }
}

void MyStreamDeckPlugin::UpdateCallback(
  const std::string& action,
  const std::string& context,
  const ButtonSettings& settings) {
  if (mCallbacks.find(context) != mCallbacks.end()) {
    RemoveDefaultAudioDeviceChangeCallback(mCallbacks[context]);
  }

  mCallbacks[context] = AddDefaultAudioDeviceChangeCallback(
    [this, action, context, settings](
      AudioDeviceDirection direction, AudioDeviceRole role,
      const std::string& deviceID) {
      if (direction != settings.direction) {
        return;
      }
      if (role != settings.role) {
        return;
      }
      UpdateState(action, context, settings, deviceID);
    });
}

MyStreamDeckPlugin::ButtonSettings MyStreamDeckPlugin::ButtonSettingsFromJSON(
  const json& inPayload) {
  ButtonSettings settings;
  json jsonSettings;
  EPLJSONUtils::GetObjectByName(inPayload, "settings", jsonSettings);
  settings.primaryDevice
    = EPLJSONUtils::GetStringByName(jsonSettings, "primary");
  settings.secondaryDevice
    = EPLJSONUtils::GetStringByName(jsonSettings, "secondary");
  settings.direction
    = EPLJSONUtils::GetStringByName(jsonSettings, "direction", "output")
          == "output"
        ? AudioDeviceDirection::OUTPUT
        : AudioDeviceDirection::INPUT;
  settings.role = EPLJSONUtils::GetStringByName(jsonSettings, "role", "default")
                      == "communication"
                    ? AudioDeviceRole::COMMUNICATION
                    : AudioDeviceRole::DEFAULT;
  return settings;
}

void MyStreamDeckPlugin::UpdateState(
  const std::string& action,
  const std::string& context,
  const ButtonSettings& settings,
  const std::string& _active) {
  const auto active = _active.empty() ? GetDefaultAudioDeviceID(
                                          settings.direction, settings.role)
                                      : _active;
  DebugPrint(
    "SDAudioSwitch: setting active ID %s %s %s", active.c_str(),
    settings.primaryDevice.c_str(), settings.secondaryDevice.c_str());

  std::scoped_lock lock(mVisibleContextsMutex);
  if (action == SET_ACTION_ID) {
    mConnectionManager->SetState(
      active == settings.primaryDevice ? 0 : 1, context);
    return;
  }
  if (active == settings.primaryDevice) {
    mConnectionManager->SetState(0, context);
  } else if (active == settings.secondaryDevice) {
    mConnectionManager->SetState(1, context);
  } else {
    mConnectionManager->ShowAlertForContext(context);
  }
}

void MyStreamDeckPlugin::DeviceDidConnect(
  const std::string& inDeviceID,
  const json& inDeviceInfo) {
  // Nothing to do
}

void MyStreamDeckPlugin::DeviceDidDisconnect(const std::string& inDeviceID) {
  // Nothing to do
}

void MyStreamDeckPlugin::DidReceiveGlobalSettings(const json& inPayload) {
}

void MyStreamDeckPlugin::DidReceiveSettings(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload,
  const std::string& inDeviceID) {
  WillAppearForAction(inAction, inContext, inPayload, inDeviceID);
}
