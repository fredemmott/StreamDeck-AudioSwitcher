/* Copyright (c) 2019-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

// Include order matters for these; don't let the autoformatter break things
// clang-format off
#include "windows.h"
#include "endpointvolume.h"
#include "mmdeviceapi.h"
#include "PolicyConfig.h"
#include "Functiondiscoverykeys_devpkey.h"
// clang-format on

#include "AudioDevices.h"
#include "StringEncoding.h"

#include <StreamDeckSDK/ESDLogger.h>

#include <atlbase.h>
#include <comip.h>

using namespace FredEmmott::Encoding;

namespace FredEmmott::Audio {

namespace {

EDataFlow AudioDeviceDirectionToEDataFlow(const AudioDeviceDirection dir) {
  switch (dir) {
    case AudioDeviceDirection::INPUT:
      return eCapture;
    case AudioDeviceDirection::OUTPUT:
      return eRender;
  }
  __assume(0);
}

ERole AudioDeviceRoleToERole(const AudioDeviceRole role) {
  switch (role) {
    case AudioDeviceRole::COMMUNICATION:
      return eCommunications;
    case AudioDeviceRole::DEFAULT:
      return eConsole;
  }
  __assume(0);
}

CComPtr<IMMDevice> DeviceIDToDevice(const std::string& device_id) {
  static std::map<std::string, CComPtr<IMMDevice>> cache;
  const auto cached = cache.find(device_id);
  if (cached != cache.end()) {
    return cached->second;
  }

  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));
  CComPtr<IMMDevice> device;
  auto utf16 = Utf8ToUtf16(device_id);
  de->GetDevice(utf16.c_str(), &device);
  if (!device) {
    throw device_not_available_error();
  }
  cache.emplace(device_id, device);
  return device;
}

CComPtr<IAudioEndpointVolume> DeviceIDToAudioEndpointVolume(
  const std::string& device_id) {
  static std::map<std::string, CComPtr<IAudioEndpointVolume>> cache;
  const auto cached = cache.find(device_id);
  if (cached != cache.end()) {
    return cached->second;
  }
  auto device = DeviceIDToDevice(device_id);
  CComPtr<IAudioEndpointVolume> volume;
  device->Activate(
    __uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&volume);
  if (!volume) {
    throw operation_not_supported_error();
  }
  cache[device_id] = volume;
  return volume;
}

AudioDeviceState GetAudioDeviceState(CComPtr<IMMDevice> device) {
  DWORD nativeState;
  device->GetState(&nativeState);

  switch (nativeState) {
    case DEVICE_STATE_ACTIVE:
      return AudioDeviceState::CONNECTED;
    case DEVICE_STATE_DISABLED:
      return AudioDeviceState::DEVICE_DISABLED;
    case DEVICE_STATE_NOTPRESENT:
      return AudioDeviceState::DEVICE_NOT_PRESENT;
    case DEVICE_STATE_UNPLUGGED:
      return AudioDeviceState::DEVICE_PRESENT_NO_CONNECTION;
  }

  __assume(0);
}

}// namespace

AudioDeviceState GetAudioDeviceState(const std::string& id) {
  auto device = DeviceIDToDevice(id);
  if (device == nullptr) {
    return AudioDeviceState::DEVICE_NOT_PRESENT;
  }
  return GetAudioDeviceState(device);
}

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(
  AudioDeviceDirection direction) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));

  CComPtr<IMMDeviceCollection> devices;
  de->EnumAudioEndpoints(
    AudioDeviceDirectionToEDataFlow(direction), DEVICE_STATEMASK_ALL, &devices);

  UINT deviceCount;
  devices->GetCount(&deviceCount);
  std::map<std::string, AudioDeviceInfo> out;

  for (UINT i = 0; i < deviceCount; ++i) {
    CComPtr<IMMDevice> device;
    devices->Item(i, &device);
    LPWSTR nativeID;
    device->GetId(&nativeID);
    const auto id = Utf16ToUtf8(nativeID);
    CComPtr<IPropertyStore> properties;
    const auto propStoreResult = device->OpenPropertyStore(STGM_READ, &properties);
    if (!properties) {
      ESDLog(L"Got a soundcard ({}) with inaccessible property store - error {}", nativeID, propStoreResult);
      continue;
    }
    PROPVARIANT nativeCombinedName;
    properties->GetValue(PKEY_Device_FriendlyName, &nativeCombinedName);
    PROPVARIANT nativeInterfaceName;
    properties->GetValue(
      PKEY_DeviceInterface_FriendlyName, &nativeInterfaceName);
    PROPVARIANT nativeEndpointName;
    properties->GetValue(PKEY_Device_DeviceDesc, &nativeEndpointName);

    if (!nativeCombinedName.pwszVal) {
      continue;
    }

    out[id] = AudioDeviceInfo{
      .id = id,
      .interfaceName = Utf16ToUtf8(nativeInterfaceName.pwszVal),
      .endpointName = Utf16ToUtf8(nativeEndpointName.pwszVal),
      .displayName = Utf16ToUtf8(nativeCombinedName.pwszVal),
      .direction = direction,
      .state = GetAudioDeviceState(device)};
  }
  return out;
}

std::string GetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));
  if (!de) {
    ESDDebug("Failed to create MMDeviceEnumerator");
    return std::string();
  }
  CComPtr<IMMDevice> device;
  de->GetDefaultAudioEndpoint(
    AudioDeviceDirectionToEDataFlow(direction), AudioDeviceRoleToERole(role),
    &device);
  if (!device) {
    ESDDebug("No default audio device");
    return std::string();
  }
  LPWSTR deviceID;
  device->GetId(&deviceID);
  if (!deviceID) {
    ESDDebug("No default audio device ID");
    return std::string();
  }
  return Utf16ToUtf8(deviceID);
}

void SetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role,
  const std::string& desiredID) {
  if (desiredID == GetDefaultAudioDeviceID(direction, role)) {
    return;
  }

  CComPtr<IPolicyConfigVista> pPolicyConfig;
  pPolicyConfig.CoCreateInstance(__uuidof(CPolicyConfigVistaClient));
  const auto utf16 = Utf8ToUtf16(desiredID);
  pPolicyConfig->SetDefaultEndpoint(
    utf16.c_str(), AudioDeviceRoleToERole(role));
}

bool IsAudioDeviceMuted(const std::string& deviceID) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return false;
  }
  BOOL ret;
  volume->GetMute(&ret);
  return ret;
}

void MuteAudioDevice(const std::string& deviceID) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return;
  }
  volume->SetMute(true, nullptr);
}

void UnmuteAudioDevice(const std::string& deviceID) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return;
  }
  volume->SetMute(false, nullptr);
}

namespace {
class VolumeCOMCallback : public IAudioEndpointVolumeCallback {
 public:
  VolumeCOMCallback(std::function<void(bool isMuted)> cb) : mCB(cb), mRefs(0) {
  }

  ~VolumeCOMCallback() {
  }

  virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ret)
    override {
    if (iid == IID_IUnknown || iid == __uuidof(IAudioEndpointVolumeCallback)) {
      *ret = static_cast<IUnknown*>(this);
      AddRef();
      return S_OK;
    }
    *ret = nullptr;
    return E_NOINTERFACE;
  }

  virtual ULONG __stdcall AddRef() override {
    return InterlockedIncrement(&mRefs);
  }
  virtual ULONG __stdcall Release() override {
    if (InterlockedDecrement(&mRefs) == 0) {
      delete this;
      return 0;
    }
    return mRefs;
  }
  virtual HRESULT OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override {
    mCB(pNotify->bMuted);
    return S_OK;
  }

 private:
  std::function<void(bool)> mCB;
  long mRefs;
};

}// namespace

struct MuteCallbackHandleImpl {
  CComPtr<VolumeCOMCallback> impl;
  CComPtr<IAudioEndpointVolume> dev;

  MuteCallbackHandleImpl(
    CComPtr<VolumeCOMCallback> impl,
    CComPtr<IAudioEndpointVolume> dev)
    : impl(impl), dev(dev) {
  }

  ~MuteCallbackHandleImpl() {
    dev->UnregisterControlChangeNotify(impl);
  }

  MuteCallbackHandleImpl(const VolumeCOMCallback& other) = delete;
  MuteCallbackHandleImpl& operator=(const VolumeCOMCallback& other) = delete;
};

MuteCallbackHandle::MuteCallbackHandle(MuteCallbackHandleImpl* impl)
  : AudioDeviceCallbackHandle(impl) {
}
MuteCallbackHandle::~MuteCallbackHandle() {
}

std::unique_ptr<MuteCallbackHandle> AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)> cb) {
  auto dev = DeviceIDToAudioEndpointVolume(deviceID);
  auto impl = CComPtr(new VolumeCOMCallback(cb));
  auto ret = dev->RegisterControlChangeNotify(impl);
  if (ret != S_OK) {
    throw operation_not_supported_error();
  }
  return std::make_unique<MuteCallbackHandle>(
    new MuteCallbackHandleImpl(impl, dev));
}

namespace {
typedef std::function<
  void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>
  DefaultChangeCallbackFun;
class DefaultChangeCOMCallback : public IMMNotificationClient {
 public:
  DefaultChangeCOMCallback(DefaultChangeCallbackFun cb) : mCB(cb), mRefs(1) {
  }

  virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ret)
    override {
    if (iid == IID_IUnknown || iid == __uuidof(IMMNotificationClient)) {
      *ret = static_cast<IUnknown*>(this);
      AddRef();
      return S_OK;
    }
    *ret = nullptr;
    return E_NOINTERFACE;
  }

  virtual ULONG __stdcall AddRef() override {
    return InterlockedIncrement(&mRefs);
  }
  virtual ULONG __stdcall Release() override {
    if (InterlockedDecrement(&mRefs) == 0) {
      delete this;
      return 0;
    }
    return mRefs;
  }

  virtual HRESULT OnDefaultDeviceChanged(
    EDataFlow flow,
    ERole winAudioDeviceRole,
    LPCWSTR defaultDeviceID) override {
    AudioDeviceRole role;
    switch (winAudioDeviceRole) {
      case ERole::eMultimedia:
        return S_OK;
      case ERole::eCommunications:
        role = AudioDeviceRole::COMMUNICATION;
        break;
      case ERole::eConsole:
        role = AudioDeviceRole::DEFAULT;
        break;
    }
    const AudioDeviceDirection direction = (flow == EDataFlow::eCapture)
                                             ? AudioDeviceDirection::INPUT
                                             : AudioDeviceDirection::OUTPUT;
    mCB(direction, role, Utf16ToUtf8(defaultDeviceID));

    return S_OK;
  };

  virtual HRESULT OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
    return S_OK;
  };

  virtual HRESULT OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
    return S_OK;
  };

  virtual HRESULT OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
    override {
    return S_OK;
  };

  virtual HRESULT OnPropertyValueChanged(
    LPCWSTR pwstrDeviceId,
    const PROPERTYKEY key) override {
    return S_OK;
  };

 private:
  DefaultChangeCallbackFun mCB;
  long mRefs;
};

}// namespace

struct DefaultChangeCallbackHandleImpl {
  CComPtr<DefaultChangeCOMCallback> impl;
  CComPtr<IMMDeviceEnumerator> enumerator;

  DefaultChangeCallbackHandleImpl(
    CComPtr<DefaultChangeCOMCallback> impl,
    CComPtr<IMMDeviceEnumerator> enumerator) {
    this->impl = impl;
    this->enumerator = enumerator;
  }

  ~DefaultChangeCallbackHandleImpl() {
    enumerator->UnregisterEndpointNotificationCallback(this->impl);
  }

  DefaultChangeCallbackHandleImpl(const DefaultChangeCallbackHandleImpl& copied)
    = delete;
  DefaultChangeCallbackHandleImpl& operator=(
    const DefaultChangeCallbackHandleImpl& other)
    = delete;
};

DefaultChangeCallbackHandle::DefaultChangeCallbackHandle(
  DefaultChangeCallbackHandleImpl* impl)
  : AudioDeviceCallbackHandle(impl) {
}

DefaultChangeCallbackHandle::~DefaultChangeCallbackHandle() {
}

std::unique_ptr<DefaultChangeCallbackHandle>
AddDefaultAudioDeviceChangeCallback(DefaultChangeCallbackFun cb) {
  CComPtr<IMMDeviceEnumerator> de;
  de.CoCreateInstance(__uuidof(MMDeviceEnumerator));
  if (!de) {
    ESDDebug("failed to get enumerator");
    return nullptr;
  }
  CComPtr<DefaultChangeCOMCallback> impl(new DefaultChangeCOMCallback(cb));
  if (de->RegisterEndpointNotificationCallback(impl) != S_OK) {
    ESDDebug("failed to register default callback");
    return nullptr;
  }

  ESDDebug("returning new default callback handle");
  return std::make_unique<DefaultChangeCallbackHandle>(
    new DefaultChangeCallbackHandleImpl(impl, de));
}

}// namespace FredEmmott::Audio
