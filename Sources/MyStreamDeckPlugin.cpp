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

MyStreamDeckPlugin::MyStreamDeckPlugin()
{
	CoInitialize(NULL); // initialize COM for the main thread
}

MyStreamDeckPlugin::~MyStreamDeckPlugin()
{
}

void MyStreamDeckPlugin::UpdateTimer()
{
	//
	// Warning: UpdateTimer() is running in the timer thread
	//
	if(mConnectionManager != nullptr)
	{
		mVisibleContextsMutex.lock();
		for (const std::string& context : mVisibleContexts)
		{
			const auto settings = mSettings[context];
			const auto currentDeviceId = GetDefaultAudioDeviceID(
				settings.direction,
				settings.role
			);
			if (settings.action == "com.fredemmott.audiooutputswitch.set") {
				mConnectionManager->SetState(currentDeviceId == settings.primaryDevice ? 0 : 1, context);
				continue;
			}

			if (settings.action == "com.fredemmott.audiooutputswitch.toggle") {
				if (currentDeviceId == settings.primaryDevice) {
					mConnectionManager->SetState(0, context);
				}
				else if (currentDeviceId == settings.secondaryDevice) {
					mConnectionManager->SetState(1, context);
				}
				else {
					mConnectionManager->ShowAlertForContext(context);
				}
				continue;
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
	const auto settings = mSettings[inContext];
	const auto deviceId = (state != 0 || inAction == "com.fredemmott.audiooutputswitch.set")
		? settings.primaryDevice
		: settings.secondaryDevice;
	if (deviceId == "") {
		return;
	}
	// We lock the mutex to stop the display flickering if we come along at the same time as the timer
	mVisibleContextsMutex.lock();

	SetDefaultAudioDeviceID(settings.direction, settings.role, deviceId);
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

	// I want C++20 :'(
	ButtonSettings bsettings{
		/* .inAction = */ inAction,
		/* .direction = */ (EPLJSONUtils::GetStringByName(settings, "direction", "output") == "input")
			? Direction::INPUT : Direction::OUTPUT,
		/* .role = */ (EPLJSONUtils::GetStringByName(settings, "role", "default") == "communication")
			? Role::COMMUNICATION : Role::DEFAULT,
		/* .primaryDevice = */ EPLJSONUtils::GetStringByName(settings, "primary", GetDefaultAudioDeviceID(Direction::OUTPUT, Role::DEFAULT)),
		/* .secondaryDevice = */ EPLJSONUtils::GetStringByName(settings, "secondary"),
	};
	mSettings[inContext] = bsettings;

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
	auto settings = mSettings[inContext];

	if (event == "getData") {
		json outPayload {
			{ "event", event },
			{ "outputDevices", GetAudioDeviceList(Direction::OUTPUT) },
			{ "inputDevices", GetAudioDeviceList(Direction::INPUT) },
			{ "settings", {
				{ "direction", settings.direction == Direction::INPUT ? "input" : "output" },
				{ "role", settings.role == Role::COMMUNICATION ? "communication" : "default" },
				{ "primary", settings.primaryDevice },
				{ "secondary", settings.secondaryDevice }
			}}
		};
		mConnectionManager->SendToPropertyInspector(inAction, inContext, outPayload);
		return;
	}

	if (event == "saveSettings") {
		json jsonSettings;
		EPLJSONUtils::GetObjectByName(inPayload, "settings", jsonSettings);
		settings.primaryDevice = EPLJSONUtils::GetStringByName(jsonSettings, "primary");
		settings.secondaryDevice = EPLJSONUtils::GetStringByName(jsonSettings, "secondary");
		settings.direction = EPLJSONUtils::GetStringByName(jsonSettings, "direction", "output") == "output"
			? Direction::OUTPUT
			: Direction::INPUT;
		settings.role = EPLJSONUtils::GetStringByName(jsonSettings, "role", "default") == "communication"
			? Role::COMMUNICATION
			: Role::DEFAULT;
		mSettings[inContext] = settings;
		mConnectionManager->SetSettings(jsonSettings, inContext);
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
