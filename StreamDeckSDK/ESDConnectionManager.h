//==============================================================================
/**
@file       ESDConnectionManager.h

@brief      Wrapper to implement the communication with the Stream Deck application

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#pragma once

#include "ESDBasePlugin.h"
#include "ESDSDKDefines.h"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::client<websocketpp::config::asio_client> WebsocketClient;

class ESDConnectionManager
{
public:
	
	ESDConnectionManager(
		int inPort,
		const std::string &inPluginUUID,
		const std::string &inRegisterEvent,
		const std::string &inInfo,
		ESDBasePlugin *inPlugin);
	
	// Start the event loop
	void Run();
	
	// API to communicate with the Stream Deck application
	void SetTitle(const std::string &inTitle, const std::string& inContext, ESDSDKTarget inTarget);
	void SetImage(const std::string &inBase64ImageString, const std::string& inContext, ESDSDKTarget inTarget);
	void ShowAlertForContext(const std::string& inContext);
	void ShowOKForContext(const std::string& inContext);
	void SetSettings(const json &inSettings, const std::string& inContext);
	void SetState(int inState, const std::string& inContext);
	void SendToPropertyInspector(const std::string& inAction, const std::string& inContext, const json &inPayload);
	void SwitchToProfile(const std::string& inDeviceID, const std::string& inProfileName);
	void LogMessage(const std::string& inMessage);

private:
	
	// Websocket callbacks
	void OnOpen(WebsocketClient * inClient, websocketpp::connection_hdl inConnectionHandler);
	void OnFail(WebsocketClient * inClient, websocketpp::connection_hdl inConnectionHandler);
	void OnClose(WebsocketClient * inClient, websocketpp::connection_hdl inConnectionHandler);
	void OnMessage(websocketpp::connection_hdl, WebsocketClient::message_ptr inMsg);
	
	// Member variables
	int mPort = 0;
	std::string mPluginUUID;
	std::string mRegisterEvent;
	websocketpp::connection_hdl mConnectionHandle;
	WebsocketClient mWebsocket;
	ESDBasePlugin * mPlugin = nullptr;
};

