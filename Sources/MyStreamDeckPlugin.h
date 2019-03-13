//==============================================================================
/**
@file       MyStreamDeckPlugin.h

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include <mutex>
#include "AudioFunctions.h"
#include "Common/ESDBasePlugin.h"

class CallBackTimer;

class MyStreamDeckPlugin : public ESDBasePlugin {
 public:
  MyStreamDeckPlugin();
  virtual ~MyStreamDeckPlugin();

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
  std::recursive_mutex mVisibleContextsMutex;
  std::set<std::string> mVisibleContexts;

  struct ButtonSettings {
    std::string action;
    Direction direction;
    Role role;
    std::string primaryDevice;
    std::string secondaryDevice;
  };
  static ButtonSettings ButtonSettingsFromJSON(const json& payload);
  void UpdateCallback(
    const std::string& context,
    const ButtonSettings& settings);
  void UpdateState(
    const std::string& context,
    const ButtonSettings& settings,
    const std::string& activeAudioDeviceID = "");

  std::map<std::string, DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE> mCallbacks;

  CallBackTimer* mTimer;
};
