/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include <memory>

namespace FredEmmott::Audio {

template <class TImpl>
class AudioDeviceCallbackHandle {
 public:
  AudioDeviceCallbackHandle(TImpl* impl) : mImpl(impl) {
  }
  ~AudioDeviceCallbackHandle() {
  }

  AudioDeviceCallbackHandle(const AudioDeviceCallbackHandle& other) = delete;
  AudioDeviceCallbackHandle& operator=(const AudioDeviceCallbackHandle& other)
    = delete;

 private:
  std::unique_ptr<TImpl> mImpl;
};

struct MuteCallbackHandleImpl;
class MuteCallbackHandle final
  : public AudioDeviceCallbackHandle<MuteCallbackHandleImpl> {
 public:
  MuteCallbackHandle(MuteCallbackHandleImpl* impl);
  ~MuteCallbackHandle();
};

struct DefaultChangeCallbackHandleImpl;
class DefaultChangeCallbackHandle final
  : public AudioDeviceCallbackHandle<DefaultChangeCallbackHandleImpl> {
 public:
  DefaultChangeCallbackHandle(DefaultChangeCallbackHandleImpl* impl);
  ~DefaultChangeCallbackHandle();
};

}// namespace FredEmmott::Audio
