/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include <CoreAudio/CoreAudio.h>
#include <StreamDeckSDK/ESDLogger.h>
#include <fmt/format.h>

#include "AudioDevices.h"

namespace FredEmmott::Audio {

namespace {

std::string Utf8StringFromCFString(CFStringRef ref, size_t buf_size = 1024) {
  // Re-use the existing buffer if possible...
  auto ptr = CFStringGetCStringPtr(ref, kCFStringEncodingUTF8);
  if (ptr) {
    return ptr;
  }
  // ... but sometimes it isn't. Copy.
  char buf[buf_size];
  CFStringGetCString(ref, buf, buf_size, kCFStringEncodingUTF8);
  return buf;
}

template <class T>
T GetAudioObjectProperty(
  AudioObjectID id,
  const AudioObjectPropertyAddress& prop) {
  T value;
  UInt32 size = sizeof(value);
  const auto result
    = AudioObjectGetPropertyData(id, &prop, 0, nullptr, &size, &value);
  switch (result) {
    case kAudioHardwareBadDeviceError:
    case kAudioHardwareBadObjectError:
      throw device_not_available_error();
    case kAudioHardwareUnsupportedOperationError:
    case kAudioHardwareUnknownPropertyError: {
      char buf[5];
      buf[4] = 0;
      memcpy(buf, &prop.mSelector, 4);
      buf[0] ^= buf[3];
      buf[3] ^= buf[0];
      buf[0] ^= buf[3];
      buf[1] ^= buf[2];
      buf[2] ^= buf[1];
      buf[1] ^= buf[2];
      ESDLog("Get prop bad prop: {}", buf);
    }
      throw operation_not_supported_error();
    default:
      break;
  }
  return value;
}

template <>
bool GetAudioObjectProperty<bool>(
  UInt32 id,
  const AudioObjectPropertyAddress& prop) {
  return GetAudioObjectProperty<UInt32>(id, prop);
}

template <>
std::string GetAudioObjectProperty<std::string>(
  UInt32 id,
  const AudioObjectPropertyAddress& prop) {
  CFStringRef value = nullptr;
  UInt32 size = sizeof(value);
  AudioObjectGetPropertyData(id, &prop, 0, nullptr, &size, &value);

  if (!value) {
    return std::string();
  }
  auto ret = Utf8StringFromCFString(value);
  CFRelease(value);
  return ret;
}

std::string MakeDeviceID(UInt32 id, AudioDeviceDirection dir) {
  const auto uid = GetAudioObjectProperty<std::string>(
    id, {kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal,
         kAudioObjectPropertyElementMaster});
  return fmt::format(
    "{}/{}", dir == AudioDeviceDirection::INPUT ? "input" : "output", uid);
}

std::tuple<UInt32, AudioDeviceDirection> ParseDeviceID(const std::string& id) {
  auto idx = id.find_first_of('/');
  auto direction = id.substr(0, idx) == "input" ? AudioDeviceDirection::INPUT
                                                : AudioDeviceDirection::OUTPUT;
  CFStringRef uid = CFStringCreateWithCString(
    kCFAllocatorDefault, id.substr(idx + 1).c_str(), kCFStringEncodingUTF8);
  UInt32 device_id;
  AudioValueTranslation value{
    &uid, sizeof(CFStringRef), &device_id, sizeof(device_id)};
  AudioObjectPropertyAddress prop{
    kAudioHardwarePropertyDeviceForUID, kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster};
  UInt32 size = sizeof(value);

  AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &prop, 0, nullptr, &size, &value);
  CFRelease(uid);

  return std::make_tuple(device_id, direction);
}

void SetAudioDeviceIsMuted(const std::string& id, bool muted) {
  const UInt32 value = muted;
  const auto [native_id, direction] = ParseDeviceID(id);
  AudioObjectPropertyAddress prop{
    kAudioDevicePropertyMute,
    direction == AudioDeviceDirection::INPUT ? kAudioDevicePropertyScopeInput
                                             : kAudioDevicePropertyScopeOutput,
    0};
  const auto result = AudioObjectSetPropertyData(
    native_id, &prop, 0, NULL, sizeof(value), &value);
  switch (result) {
    case kAudioHardwareBadDeviceError:
    case kAudioHardwareBadObjectError:
      ESDLog("bad device");
      throw device_not_available_error();
    case kAudioHardwareUnsupportedOperationError:
    case kAudioHardwareUnknownPropertyError:
      ESDLog("bad operation");
      throw operation_not_supported_error();
    default:
      ESDLog("Unhandled error {}", result);
    case kAudioHardwareNoError:
      break;
  }
}

}// namespace

std::string GetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role) {
  if (role != AudioDeviceRole::DEFAULT) {
    return std::string();
  }
  AudioDeviceID native_id = 0;
  UInt32 native_id_size = sizeof(native_id);
  AudioObjectPropertyAddress prop = {
    direction == AudioDeviceDirection::INPUT
      ? kAudioHardwarePropertyDefaultInputDevice
      : kAudioHardwarePropertyDefaultOutputDevice,
    kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};

  AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &prop, 0, NULL, &native_id_size, &native_id);
  return MakeDeviceID(native_id, direction);
}

void SetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role,
  const std::string& deviceID) {

  if (role != AudioDeviceRole::DEFAULT) {
    return;
  }
  const auto [native_id, id_dir] = ParseDeviceID(deviceID);
  if (id_dir != direction) {
    return;
  }

	AudioObjectPropertyAddress prop = {
		direction == AudioDeviceDirection::INPUT
			? kAudioHardwarePropertyDefaultInputDevice
			: kAudioHardwarePropertyDefaultOutputDevice,
		kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
  AudioObjectSetPropertyData(
    kAudioObjectSystemObject, &prop, 0, NULL, sizeof(native_id), &native_id);
}

bool IsAudioDeviceMuted(const std::string& id) {
  const auto [native_id, direction] = ParseDeviceID(id);
  return GetAudioObjectProperty<UInt32>(
    native_id,
    {kAudioDevicePropertyMute,
     direction == AudioDeviceDirection::INPUT ? kAudioObjectPropertyScopeInput
                                              : kAudioObjectPropertyScopeOutput,
     kAudioObjectPropertyElementMaster});
};

void MuteAudioDevice(const std::string& id) {
  SetAudioDeviceIsMuted(id, true);
}

void UnmuteAudioDevice(const std::string& id) {
  SetAudioDeviceIsMuted(id, false);
}

namespace {
std::string GetDataSourceName(
  AudioDeviceID device_id,
  AudioObjectPropertyScope scope) {
  AudioObjectID data_source;
  try {
    data_source = GetAudioObjectProperty<AudioObjectID>(
      device_id, {kAudioDevicePropertyDataSource, scope,
                  kAudioObjectPropertyElementMaster});
  } catch (operation_not_supported_error) {
    return std::string();
  }

  CFStringRef value = nullptr;
  AudioValueTranslation translate{
    &data_source, sizeof(data_source), &value, sizeof(value)};
  UInt32 size = sizeof(translate);
  const AudioObjectPropertyAddress prop{
    kAudioDevicePropertyDataSourceNameForIDCFString, scope,
    kAudioObjectPropertyElementMaster};
  AudioObjectGetPropertyData(device_id, &prop, 0, nullptr, &size, &translate);
  if (!value) {
    return std::string();
  }
  auto ret = Utf8StringFromCFString(value);
  CFRelease(value);
  return ret;
}
}// namespace

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(
  AudioDeviceDirection direction) {
  UInt32 size = 0;
  AudioObjectPropertyAddress prop = {
    kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMaster};

  AudioObjectGetPropertyDataSize(
    kAudioObjectSystemObject, &prop, 0, nullptr, &size);
  const auto count = size / sizeof(AudioDeviceID);

  AudioDeviceID ids[count];
  AudioObjectGetPropertyData(
    kAudioObjectSystemObject, &prop, 0, nullptr, &size, ids);

  std::map<std::string, AudioDeviceInfo> out;

  // The array of devices will always contain both input and output, even if
  // we set the scope above; instead filter inside the loop
  const auto scope = direction == AudioDeviceDirection::INPUT
                       ? kAudioObjectPropertyScopeInput
                       : kAudioObjectPropertyScopeOutput;
  for (const auto id : ids) {
    const auto interface_name = GetAudioObjectProperty<std::string>(
      id, {kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal,
           kAudioObjectPropertyElementMaster});
    // ... and we do that filtering by finding out how many channels there
    // are. No channels for a given direction? Not a valid device for that
    // direction.
    UInt32 size;
    prop
      = {kAudioDevicePropertyStreams, scope, kAudioObjectPropertyScopeGlobal};
    AudioObjectGetPropertyDataSize(id, &prop, 0, nullptr, &size);
    if (size == 0) {
      continue;
    }

    AudioDeviceInfo info{
      .id = MakeDeviceID(id, direction),
      .interfaceName = interface_name,
      .direction = direction};
    info.state = GetAudioDeviceState(info.id);

    const auto data_source_name = GetDataSourceName(id, scope);
    if (data_source_name.empty()) {
      info.displayName = info.interfaceName;
    } else {
      info.displayName = data_source_name;
    }

    out.emplace(info.id, info);
  }
  return out;
}

AudioDeviceState GetAudioDeviceState(const std::string& id) {
  const auto [native_id, direction] = ParseDeviceID(id);
  if (!native_id) {
    return AudioDeviceState::DEVICE_NOT_PRESENT;
  }
  const auto scope = (direction == AudioDeviceDirection::INPUT)
                       ? kAudioDevicePropertyScopeInput
                       : kAudioObjectPropertyScopeOutput;

  const auto transport = GetAudioObjectProperty<UInt32>(
    native_id, {kAudioDevicePropertyTransportType, scope,
                kAudioObjectPropertyElementMaster});

  // no jack: 'Internal Speakers'
  // jack: 'Headphones'
  //
  // Showing plugged/unplugged for these is just noise
  if (transport == kAudioDeviceTransportTypeBuiltIn) {
    return AudioDeviceState::CONNECTED;
  }

  const AudioObjectPropertyAddress prop = {
    kAudioDevicePropertyJackIsConnected, scope,
    kAudioObjectPropertyElementMaster};
  const auto supports_jack = AudioObjectHasProperty(native_id, &prop);
  if (!supports_jack) {
    return AudioDeviceState::CONNECTED;
  }

  const auto is_plugged = GetAudioObjectProperty<bool>(native_id, prop);
  return is_plugged ? AudioDeviceState::CONNECTED
                    : AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION;
}

namespace {

template <class TProp>
struct BaseCallbackHandleImpl {
  typedef std::function<void(TProp value)> UserCallback;
  BaseCallbackHandleImpl(
    UserCallback callback,
    AudioDeviceID device,
    AudioObjectPropertyAddress prop)
    : mProp(prop), mDevice(device), mCallback(callback) {
    AudioObjectAddPropertyListener(mDevice, &mProp, &OSCallback, this);
  }

  virtual ~BaseCallbackHandleImpl() {
    AudioObjectRemovePropertyListener(mDevice, &mProp, &OSCallback, this);
  }

 private:
  const AudioObjectPropertyAddress mProp;
  AudioDeviceID mDevice;
  UserCallback mCallback;

  static OSStatus OSCallback(
    AudioDeviceID id,
    UInt32 _prop_count,
    const AudioObjectPropertyAddress* _props,
    void* data) {
    auto self = reinterpret_cast<BaseCallbackHandleImpl<TProp>*>(data);
    const auto value = GetAudioObjectProperty<TProp>(id, self->mProp);
    self->mCallback(value);
    return 0;
  }
};

}// namespace

struct MuteCallbackHandleImpl : BaseCallbackHandleImpl<bool> {
  MuteCallbackHandleImpl(
    UserCallback cb,
    AudioDeviceID device,
    AudioDeviceDirection direction)
    : BaseCallbackHandleImpl(
      cb,
      device,
      {kAudioDevicePropertyMute,
       direction == AudioDeviceDirection::INPUT
         ? kAudioObjectPropertyScopeInput
         : kAudioObjectPropertyScopeOutput,
       kAudioObjectPropertyElementMaster}) {
  }
};

MuteCallbackHandle::MuteCallbackHandle(MuteCallbackHandleImpl* impl)
  : AudioDeviceCallbackHandle(impl) {
}
MuteCallbackHandle::~MuteCallbackHandle() {
}

std::unique_ptr<MuteCallbackHandle> AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)> cb) {
  const auto [id, direction] = ParseDeviceID(deviceID);
  return std::make_unique<MuteCallbackHandle>(
    new MuteCallbackHandleImpl(cb, id, direction));
}

struct DefaultChangeCallbackHandleImpl {
  DefaultChangeCallbackHandleImpl(
    std::function<
      void(AudioDeviceDirection, AudioDeviceRole, const std::string&)> cb)
    : mInputImpl(
      [=](AudioDeviceID native_id) {
        const auto device
          = MakeDeviceID(native_id, AudioDeviceDirection::INPUT);
        cb(AudioDeviceDirection::INPUT, AudioDeviceRole::DEFAULT, device);
      },
      kAudioObjectSystemObject,
      {kAudioHardwarePropertyDefaultInputDevice,
       kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}),
      mOutputImpl(
        [=](AudioDeviceID native_id) {
          const auto device
            = MakeDeviceID(native_id, AudioDeviceDirection::OUTPUT);
          cb(AudioDeviceDirection::OUTPUT, AudioDeviceRole::DEFAULT, device);
        },
        kAudioObjectSystemObject,
        {kAudioHardwarePropertyDefaultOutputDevice,
         kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster}) {
  }

 private:
  BaseCallbackHandleImpl<AudioDeviceID> mInputImpl;
  BaseCallbackHandleImpl<AudioDeviceID> mOutputImpl;
};

DefaultChangeCallbackHandle::DefaultChangeCallbackHandle(
  DefaultChangeCallbackHandleImpl* impl)
  : AudioDeviceCallbackHandle(impl) {
}
DefaultChangeCallbackHandle::~DefaultChangeCallbackHandle() {
}

std::unique_ptr<DefaultChangeCallbackHandle>
AddDefaultAudioDeviceChangeCallback(
  std::function<void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>
    cb) {
  return std::make_unique<DefaultChangeCallbackHandle>(
    new DefaultChangeCallbackHandleImpl(cb));
}

}// namespace FredEmmott::Audio
