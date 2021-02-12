/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "AudioMuteStreamDeckPlugin.h"

#include <StreamDeckSDK/EPLJSONUtils.h>
#include <StreamDeckSDK/ESDAction.h>
#include <StreamDeckSDK/ESDConnectionManager.h>
#include <StreamDeckSDK/ESDLogger.h>

#include <atomic>
#include <mutex>

#include "AudioDevices.h"
#include "DefaultAudioDevices.h"
#include "MuteAction.h"
#include "ToggleMuteAction.h"
#include "UnmuteAction.h"

AudioMuteStreamDeckPlugin::AudioMuteStreamDeckPlugin(): ESDPlugin() {
#ifdef _MSC_VER
  CoInitializeEx(
    NULL, COINIT_MULTITHREADED);// initialize COM for the main thread
#endif
}

AudioMuteStreamDeckPlugin::~AudioMuteStreamDeckPlugin() {
}

std::shared_ptr<ESDAction> AudioMuteStreamDeckPlugin::GetOrCreateAction(
  const std::string& action,
  const std::string& context) {
  std::scoped_lock lock(mActionsMutex);
  auto it = mActions.find(context);
  if (it != mActions.end()) {
    ESDDebug("Found existing action: {} {}", action, context);
    return it->second;
  }

  ESDLog("Creating action {} with context {}", action, context);

  if (action == ToggleMuteAction::ACTION_ID) {
    ESDDebug("Creating ToggleMuteAction");
    auto impl = std::make_shared<ToggleMuteAction>(mConnectionManager, context);
    mActions.emplace(context, impl);
    return impl;
  }

  if (action == MuteAction::ACTION_ID) {
    ESDDebug("Creating MuteAction");
    auto impl = std::make_shared<MuteAction>(mConnectionManager, context);
    mActions.emplace(context, impl);
    return impl;
  }

  if (action == UnmuteAction::ACTION_ID) {
    ESDDebug("Creating UnmuteAction");
    auto impl = std::make_shared<UnmuteAction>(mConnectionManager, context);
    mActions.emplace(context, impl);
    return impl;
  }

  ESDLog("Didn't recognize action '{}'", action);
  return nullptr;
}
