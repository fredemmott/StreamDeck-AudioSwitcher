//==============================================================================
/**
@file       AudioSwitcherStreamDeckPlugin.h

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include <AudioDevices/AudioDevices.h>
#include <StreamDeckSDK/ESDBasePlugin.h>

#include <mutex>
#include <set>

using json = nlohmann::json;
using namespace FredEmmott::Audio;

class CallBackTimer;

class AudioSwitcherStreamDeckPlugin : public ESDBasePlugin {
 public:
  AudioSwitcherStreamDeckPlugin();
  virtual ~AudioSwitcherStreamDeckPlugin();

  void KeyDownForAction(
    const std::string& inAction,
    const std::string& inContext,
    const json& inPayload,
    const std::string& inDeviceID) override;
  void KeyUpForAction(
    const std::string& inAction,
    const std::string& inContext,
    const json& inPayload,
    const std::string& inDeviceID) override;

  void WillAppearForAction(
    const std::string& inAction,
    const std::string& inContext,
    const json& inPayload,
    const std::string& inDeviceID) override;
  void WillDisappearForAction(
    const std::string& inAction,
    const std::string& inContext,
    const json& inPayload,
    const std::string& inDeviceID) override;

  void SendToPlugin(
    const std::string& inAction,
    const std::string& inContext,
    const json& inPayload,
    const std::string& inDeviceID) override;

  void DeviceDidConnect(const std::string& inDeviceID, const json& inDeviceInfo)
    override;
  void DeviceDidDisconnect(const std::string& inDeviceID) override;

  void DidReceiveGlobalSettings(const json& inPayload) override;
  void DidReceiveSettings(
    const std::string& inAction,
    const std::string& inContext,
    const json& inPayload,
    const std::string& inDeviceID) override;

 private:
  struct ButtonSettings {
    AudioDeviceDirection direction = AudioDeviceDirection::INPUT;
    AudioDeviceRole role = AudioDeviceRole::DEFAULT;
    AudioDeviceInfo primaryDevice;
    AudioDeviceInfo secondaryDevice;
  };
  struct Button {
    std::string action;
    std::string context;
    ButtonSettings settings;
  };

  std::recursive_mutex mVisibleContextsMutex;
  std::set<std::string> mVisibleContexts;

  std::map<std::string, Button> mButtons;
  DefaultChangeCallbackHandle mCallbackHandle;

  static ButtonSettings ButtonSettingsFromJSON(const json& payload);
  void OnDefaultDeviceChanged(
    AudioDeviceDirection direction,
    AudioDeviceRole role,
    const std::string& activeAudioDeviceID);
  void UpdateState(const std::string& context, const std::string& device = "");
};
