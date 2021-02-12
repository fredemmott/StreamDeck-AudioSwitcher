/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include <StreamDeckSDK/ESDPlugin.h>

#include <map>
#include <mutex>

class AudioMuteStreamDeckPlugin : public ESDPlugin {
 public:
  AudioMuteStreamDeckPlugin();
  virtual ~AudioMuteStreamDeckPlugin();

 protected:
  std::shared_ptr<ESDAction> GetOrCreateAction(
    const std::string& action,
    const std::string& context);

 private:
  std::mutex mActionsMutex;
  std::map<std::string, std::shared_ptr<ESDAction>> mActions;
};
