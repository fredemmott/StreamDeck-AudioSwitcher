// Include order matters for these; don't let the autoformatter break things

// clang-format off
#include "windows.h"
#include "endpointvolume.h"
#include "mmdeviceapi.h"
#include "mmsystem.h"
#include "PolicyConfig.h"
#include "Functiondiscoverykeys_devpkey.h"
// clang-format on

#ifdef HAVE_FEEDBACK_SOUNDS
#include "resource.h"
#endif

#include "../AudioFunctions.h"

#include <cassert>
#include <codecvt>
#include <locale>

namespace {
std::string WCharPtrToString(LPCWSTR in) {
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
    .to_bytes(in);
}

std::wstring Utf8StrToWString(const std::string& in) {
  return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
    .from_bytes(in);
}

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

IMMDevice* DeviceIDToDevice(const std::string& in) {
  IMMDeviceEnumerator* de;
  CoCreateInstance(
    __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator), (void**)&de);
  IMMDevice* device = nullptr;
  de->GetDevice(Utf8StrToWString(in).data(), &device);
  de->Release();
  return device;
}

IAudioEndpointVolume* DeviceIDToAudioEndpointVolume(
  const std::string& deviceID) {
  auto device = DeviceIDToDevice(deviceID);
  if (!device) {
    return nullptr;
  }
  IAudioEndpointVolume* volume = nullptr;
  device->Activate(
    __uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**)&volume);
  device->Release();
  return volume;
}

class ScopeExit {
 public:
  ScopeExit(std::function<void()> fun) : fun(fun) {
  }

  ~ScopeExit() {
    fun();
  }

 private:
  std::function<void()> fun;
};

#define _SCOPE_EXIT_CAT(a, b) a##b
#define _SCOPE_EXIT_UNIQUE_ID(counter) \
  _SCOPE_EXIT_CAT(_SCOPE_EXIT_INSTANCE_, counter)
#define SCOPE_EXIT(x) \
  const ScopeExit _SCOPE_EXIT_UNIQUE_ID(__COUNTER__)([&]() x)

AudioDeviceState GetAudioDeviceState(IMMDevice* device) {
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
  assert(false);
}

}// namespace

AudioDeviceState GetAudioDeviceState(const std::string& id) {
  auto device = DeviceIDToDevice(id);
  if (device == nullptr) {
    return AudioDeviceState::DEVICE_NOT_PRESENT;
  }
  SCOPE_EXIT({ device->Release(); });
  return GetAudioDeviceState(device);
}

std::map<std::string, AudioDeviceInfo> GetAudioDeviceList(
  AudioDeviceDirection direction) {
  IMMDeviceEnumerator* de;
  CoCreateInstance(
    __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator), (void**)&de);
  SCOPE_EXIT({ de->Release(); });

  IMMDeviceCollection* devices;
  de->EnumAudioEndpoints(
    AudioDeviceDirectionToEDataFlow(direction), DEVICE_STATEMASK_ALL, &devices);
  SCOPE_EXIT({ devices->Release(); });

  UINT deviceCount;
  devices->GetCount(&deviceCount);
  std::map<std::string, AudioDeviceInfo> out;

  for (UINT i = 0; i < deviceCount; ++i) {
    IMMDevice* device;
    devices->Item(i, &device);
    SCOPE_EXIT({ device->Release(); });
    LPWSTR nativeID;
    device->GetId(&nativeID);
    const auto id = WCharPtrToString(nativeID);
    IPropertyStore* properties;
    device->OpenPropertyStore(STGM_READ, &properties);
    SCOPE_EXIT({ properties->Release(); });
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

    // TODO: use designated initializers once I upgrade to VS2019 (which has
    // C++20)
    out[id] = AudioDeviceInfo{
      id,
      WCharPtrToString(nativeInterfaceName.pwszVal),
      WCharPtrToString(nativeEndpointName.pwszVal),
      WCharPtrToString(nativeCombinedName.pwszVal),
      direction,
      GetAudioDeviceState(device),
    };
  }
  return out;
}

std::string GetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role) {
  IMMDeviceEnumerator* de;
  CoCreateInstance(
    __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator), (void**)&de);
  IMMDevice* device;
  de->GetDefaultAudioEndpoint(
    AudioDeviceDirectionToEDataFlow(direction), AudioDeviceRoleToERole(role),
    &device);
  LPWSTR deviceID;
  device->GetId(&deviceID);
  const auto ret = WCharPtrToString(deviceID);
  device->Release();
  de->Release();
  return ret;
}

void SetDefaultAudioDeviceID(
  AudioDeviceDirection direction,
  AudioDeviceRole role,
  const std::string& desiredID) {
  if (desiredID == GetDefaultAudioDeviceID(direction, role)) {
    return;
  }

  IPolicyConfigVista* pPolicyConfig;

  CoCreateInstance(
    __uuidof(CPolicyConfigVistaClient), NULL, CLSCTX_ALL,
    __uuidof(IPolicyConfigVista), (LPVOID*)&pPolicyConfig);
  pPolicyConfig->SetDefaultEndpoint(
    Utf8StrToWString(desiredID).c_str(), AudioDeviceRoleToERole(role));
  pPolicyConfig->Release();
}

bool IsAudioDeviceMuted(const std::string& deviceID) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return false;
  }
  BOOL ret;
  volume->GetMute(&ret);
  volume->Release();
  return ret;
}

void SetIsAudioDeviceMuted(const std::string& deviceID, MuteAction action) {
  auto volume = DeviceIDToAudioEndpointVolume(deviceID);
  if (!volume) {
    return;
  }
  if (action == MuteAction::MUTE) {
    volume->SetMute(true, nullptr);
  } else if (action == MuteAction::UNMUTE) {
    volume->SetMute(false, nullptr);
  } else {
    assert(action == MuteAction::TOGGLE);
    BOOL muted;
    volume->GetMute(&muted);
    volume->SetMute(!muted, nullptr);
  }
  volume->Release();
}

namespace {
class VolumeCallback : public IAudioEndpointVolumeCallback {
 public:
  VolumeCallback(std::function<void(bool isMuted)> cb) : mCB(cb), mRefs(1) {
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

struct VolumeCallbackHandle {
  std::string deviceID;
  VolumeCallback* impl;
  IAudioEndpointVolume* dev;
};

}// namespace

AUDIO_DEVICE_MUTE_CALLBACK_HANDLE
AddAudioDeviceMuteUnmuteCallback(
  const std::string& deviceID,
  std::function<void(bool isMuted)> cb) {
  auto dev = DeviceIDToAudioEndpointVolume(deviceID);
  if (!dev) {
    return nullptr;
  }
  auto impl = new VolumeCallback(cb);
  auto ret = dev->RegisterControlChangeNotify(impl);
  if (ret != S_OK) {
    dev->Release();
    delete impl;
    return nullptr;
  }
  return new VolumeCallbackHandle({deviceID, impl, dev});
}

void RemoveAudioDeviceMuteUnmuteCallback(
  AUDIO_DEVICE_MUTE_CALLBACK_HANDLE _handle) {
  if (!_handle) {
    return;
  }
  const auto handle = reinterpret_cast<VolumeCallbackHandle*>(_handle);
  handle->dev->UnregisterControlChangeNotify(handle->impl);
  handle->dev->Release();
  delete handle;
  return;
}

namespace {
typedef std::function<
  void(AudioDeviceDirection, AudioDeviceRole, const std::string&)>
  DefaultChangeCallbackFun;
class DefaultChangeCallback : public IMMNotificationClient {
 public:
  DefaultChangeCallback(DefaultChangeCallbackFun cb) : mCB(cb), mRefs(1) {
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
    mCB(direction, role, WCharPtrToString(defaultDeviceID));

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

struct DefaultChangeCallbackHandle {
  DefaultChangeCallback* impl;
  IMMDeviceEnumerator* enumerator;
};

#ifdef HAVE_FEEDBACK_SOUNDS
const auto muteWav = MAKEINTRESOURCE(IDR_MUTE);
const auto unmuteWav = MAKEINTRESOURCE(IDR_UNMUTE);
#endif
}// namespace

DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE
AddDefaultAudioDeviceChangeCallback(DefaultChangeCallbackFun cb) {
  IMMDeviceEnumerator* de = nullptr;
  CoCreateInstance(
    __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator), (void**)&de);
  if (!de) {
    return nullptr;
  }
  auto impl = new DefaultChangeCallback(cb);
  if (de->RegisterEndpointNotificationCallback(impl) != S_OK) {
    de->Release();
    delete impl;
    return nullptr;
  }

  return new DefaultChangeCallbackHandle({impl, de});
}

#ifdef HAVE_FEEDBACK_SOUNDS
void PlayFeedbackSound(MuteAction action) {
  assert(action != MuteAction::TOGGLE);
  const auto feedbackWav = (action == MuteAction::MUTE) ? muteWav : unmuteWav;
  PlaySound(feedbackWav, GetModuleHandle(NULL), SND_ASYNC | SND_RESOURCE);
}
#endif

void RemoveDefaultAudioDeviceChangeCallback(
  DEFAULT_AUDIO_DEVICE_CHANGE_CALLBACK_HANDLE _handle) {
  if (!_handle) {
    return;
  }
  return;
  const auto handle = reinterpret_cast<DefaultChangeCallbackHandle*>(_handle);
  handle->enumerator->UnregisterEndpointNotificationCallback(handle->impl);
  handle->enumerator->Release();
  delete handle;
  return;
}
