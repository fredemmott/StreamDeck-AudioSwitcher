//==============================================================================
/**
@file       MyStreamDeckPlugin.cpp

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "MyStreamDeckPlugin.h"
#include <atomic>

#include "AudioFunctions.h"
#include "Common/ESDConnectionManager.h"
#include "Common/EPLJSONUtils.h"

class CallBackTimer
{
public:
    CallBackTimer() :_execute(false) { }

    ~CallBackTimer()
    {
        if( _execute.load(std::memory_order_acquire) )
        {
            stop();
        };
    }

    void stop()
    {
        _execute.store(false, std::memory_order_release);
        if(_thd.joinable())
            _thd.join();
    }

    void start(int interval, std::function<void(void)> func)
    {
        if(_execute.load(std::memory_order_acquire))
        {
            stop();
        };
        _execute.store(true, std::memory_order_release);
        _thd = std::thread([this, interval, func]()
        {
			CoInitialize(NULL); // initialize COM again for the timer thread
            while (_execute.load(std::memory_order_acquire))
            {
                func();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }
        });
    }

    bool is_running() const noexcept
    {
        return (_execute.load(std::memory_order_acquire) && _thd.joinable());
    }

private:
    std::atomic<bool> _execute;
    std::thread _thd;
};

MyStreamDeckPlugin::MyStreamDeckPlugin()
{
	CoInitialize(NULL); // initialize COM for the main thread
	mTimer = new CallBackTimer();
	mTimer->start(500, [this]()
	{
		this->UpdateTimer();
	});
}

MyStreamDeckPlugin::~MyStreamDeckPlugin()
{
	if (mTimer != nullptr)
	{
		mTimer->stop();

		delete mTimer;
		mTimer = nullptr;
	}
}

void MyStreamDeckPlugin::UpdateTimer()
{
	//
	// Warning: UpdateTimer() is running in the timer thread
	//
	if(mConnectionManager != nullptr)
	{
		mVisibleContextsMutex.lock();
		const auto currentDeviceId = GetDefaultAudioDeviceID();
		for (const std::string& context : mVisibleContexts)
		{
			const auto primary = mPrimaryDevices[context];
			const auto secondary = mSecondaryDevices[context];
			if (currentDeviceId == primary) {
				mConnectionManager->SetState(0, context);
			} else if (currentDeviceId == secondary) {
				mConnectionManager->SetState(1, context);
			}else {
				mConnectionManager->ShowAlertForContext(context);
			}
		}
		mVisibleContextsMutex.unlock();
	}
}

void MyStreamDeckPlugin::KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	const auto state = EPLJSONUtils::GetIntByName(inPayload, "state");
	// this looks inverted - but if state is 0, we want to move to state 1, so we want the secondary devices.
	// if state is 1, we want state 0, so we want the primary device
	const auto deviceId = state != 0 ? mPrimaryDevices[inContext] : mSecondaryDevices[inContext];
	if (deviceId == "") {
		return;
	}
	// We lock the mutex to stop the display flickering if we come along at the same time as the timer
	mVisibleContextsMutex.lock();
	const auto currentDeviceId = GetDefaultAudioDeviceID();
	SetDefaultAudioDeviceID(deviceId);
}

void MyStreamDeckPlugin::KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Remember the context
	mVisibleContextsMutex.lock();

	mVisibleContexts.insert(inContext);
	json settings;
	EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);
	mPrimaryDevices[inContext] = EPLJSONUtils::GetStringByName(settings, "primary", GetDefaultAudioDeviceID());
	mSecondaryDevices[inContext] = EPLJSONUtils::GetStringByName(settings, "secondary");

	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Remove the context
	mVisibleContextsMutex.lock();
	mVisibleContexts.erase(inContext);
	mVisibleContextsMutex.unlock();
}

void MyStreamDeckPlugin::SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	json outPayload;

	const auto event = EPLJSONUtils::GetStringByName(inPayload, "event");

	if (event == "getData") {
		json outPayload {
			{ "event", event },
			{ "allDevices", GetAudioDeviceList() },
			{ "settings", {
				{ "primary", mPrimaryDevices[inContext] },
				{ "secondary", mSecondaryDevices[inContext] }
			}}
		};
		mConnectionManager->SendToPropertyInspector(inAction, inContext, outPayload);
		return;
	}

	if (event == "saveSettings") {
		json settings;
		EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);
		mPrimaryDevices[inContext] = EPLJSONUtils::GetStringByName(settings, "primary");
		mSecondaryDevices[inContext] = EPLJSONUtils::GetStringByName(settings, "secondary");
		mConnectionManager->SetSettings(settings, inContext);
		return;
	}
}

void MyStreamDeckPlugin::DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo)
{
	// Nothing to do
}

void MyStreamDeckPlugin::DeviceDidDisconnect(const std::string& inDeviceID)
{
	// Nothing to do
}
