//==============================================================================
/**
@file       ESDConnectionManager.cpp

@brief      Wrapper to implement the communication with the Stream Deck
application

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include "ESDConnectionManager.h"
#include "EPLJSONUtils.h"

void ESDConnectionManager::OnOpen(
  WebsocketClient* inClient,
  websocketpp::connection_hdl inConnectionHandler) {
  // Register plugin with StreamDeck
  json jsonObject;
  jsonObject["event"] = mRegisterEvent;
  jsonObject["uuid"] = mPluginUUID;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::OnFail(
  WebsocketClient* inClient,
  websocketpp::connection_hdl inConnectionHandler) {
  std::string reason;

  if (inClient != nullptr) {
    WebsocketClient::connection_ptr connection
      = inClient->get_con_from_hdl(inConnectionHandler);
    if (connection != NULL) {
      reason = connection->get_ec().message();
    }
  }

  DebugPrint("Failed with reason: %s\n", reason.c_str());
}

void ESDConnectionManager::OnClose(
  WebsocketClient* inClient,
  websocketpp::connection_hdl inConnectionHandler) {
  std::string reason;

  if (inClient != nullptr) {
    WebsocketClient::connection_ptr connection
      = inClient->get_con_from_hdl(inConnectionHandler);
    if (connection != NULL) {
      reason = connection->get_remote_close_reason();
    }
  }

  DebugPrint("Close with reason: %s\n", reason.c_str());
}

void ESDConnectionManager::OnMessage(
  websocketpp::connection_hdl,
  WebsocketClient::message_ptr inMsg) {
  if (
    inMsg != NULL && inMsg->get_opcode() == websocketpp::frame::opcode::text) {
    std::string message = inMsg->get_payload();

    try {
      json receivedJson = json::parse(message);

      std::string event
        = EPLJSONUtils::GetStringByName(receivedJson, kESDSDKCommonEvent);
      std::string context
        = EPLJSONUtils::GetStringByName(receivedJson, kESDSDKCommonContext);
      std::string action
        = EPLJSONUtils::GetStringByName(receivedJson, kESDSDKCommonAction);
      std::string deviceID
        = EPLJSONUtils::GetStringByName(receivedJson, kESDSDKCommonDevice);
      json payload;
      EPLJSONUtils::GetObjectByName(
        receivedJson, kESDSDKCommonPayload, payload);

      if (event == kESDSDKEventKeyDown) {
        mPlugin->KeyDownForAction(action, context, payload, deviceID);
      } else if (event == kESDSDKEventKeyUp) {
        mPlugin->KeyUpForAction(action, context, payload, deviceID);
      } else if (event == kESDSDKEventWillAppear) {
        mPlugin->WillAppearForAction(action, context, payload, deviceID);
      } else if (event == kESDSDKEventWillDisappear) {
        mPlugin->WillDisappearForAction(action, context, payload, deviceID);
      } else if (event == kESDSDKEventDeviceDidConnect) {
        json deviceInfo;
        EPLJSONUtils::GetObjectByName(
          receivedJson, kESDSDKCommonDeviceInfo, deviceInfo);
        mPlugin->DeviceDidConnect(deviceID, deviceInfo);
      } else if (event == kESDSDKEventDeviceDidDisconnect) {
        mPlugin->DeviceDidDisconnect(deviceID);
      } else if (event == kESDSDKEventDidReceiveGlobalSettings) {
        mPlugin->DidReceiveGlobalSettings(payload);
      } else if (event == kESDSDKEventSendToPlugin) {
        mPlugin->SendToPlugin(action, context, payload, deviceID);
      }
    } catch (...) {
    }
  }
}

ESDConnectionManager::ESDConnectionManager(
  int inPort,
  const std::string& inPluginUUID,
  const std::string& inRegisterEvent,
  const std::string& inInfo,
  ESDBasePlugin* inPlugin)
  :

    mPort(inPort),
    mPluginUUID(inPluginUUID),
    mRegisterEvent(inRegisterEvent),
    mPlugin(inPlugin) {
  if (inPlugin != nullptr)
    inPlugin->SetConnectionManager(this);
}

void ESDConnectionManager::Run() {
  try {
    // Create the endpoint
    mWebsocket.clear_access_channels(websocketpp::log::alevel::all);
    mWebsocket.clear_error_channels(websocketpp::log::elevel::all);

    // Initialize ASIO
    mWebsocket.init_asio();

    // Register our message handler
    mWebsocket.set_open_handler(websocketpp::lib::bind(
      &ESDConnectionManager::OnOpen, this, &mWebsocket,
      websocketpp::lib::placeholders::_1));
    mWebsocket.set_fail_handler(websocketpp::lib::bind(
      &ESDConnectionManager::OnFail, this, &mWebsocket,
      websocketpp::lib::placeholders::_1));
    mWebsocket.set_close_handler(websocketpp::lib::bind(
      &ESDConnectionManager::OnClose, this, &mWebsocket,
      websocketpp::lib::placeholders::_1));
    mWebsocket.set_message_handler(websocketpp::lib::bind(
      &ESDConnectionManager::OnMessage, this,
      websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));

    websocketpp::lib::error_code ec;
    std::string uri = "ws://localhost:" + std::to_string(mPort);
    WebsocketClient::connection_ptr connection
      = mWebsocket.get_connection(uri, ec);
    if (ec) {
      DebugPrint("Connect initialization error: %s\n", ec.message().c_str());
      return;
    }

    mConnectionHandle = connection->get_handle();

    // Note that connect here only requests a connection. No network messages
    // are exchanged until the event loop starts running in the next line.
    mWebsocket.connect(connection);

    // Start the ASIO io_service run loop
    // this will cause a single connection to be made to the server.
    // mWebsocket.run() will exit when this connection is closed.
    mWebsocket.run();
  } catch (websocketpp::exception const& e) {
    // Prevent an unused variable warning in release builds
    (void)e;
    DebugPrint("Websocket threw an exception: %s\n", e.what());
  }
}

void ESDConnectionManager::SetTitle(
  const std::string& inTitle,
  const std::string& inContext,
  ESDSDKTarget inTarget) {
  json jsonObject;

  jsonObject[kESDSDKCommonEvent] = kESDSDKEventSetTitle;
  jsonObject[kESDSDKCommonContext] = inContext;

  json payload;
  payload[kESDSDKPayloadTarget] = inTarget;
  payload[kESDSDKPayloadTitle] = inTitle;
  jsonObject[kESDSDKCommonPayload] = payload;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::SetImage(
  const std::string& inBase64ImageString,
  const std::string& inContext,
  ESDSDKTarget inTarget) {
  json jsonObject;

  jsonObject[kESDSDKCommonEvent] = kESDSDKEventSetImage;
  jsonObject[kESDSDKCommonContext] = inContext;

  json payload;
  payload[kESDSDKPayloadTarget] = inTarget;
  const std::string prefix = "data:image/png;base64,";
  if (
    inBase64ImageString.empty()
    || inBase64ImageString.substr(0, prefix.length()).find(prefix) == 0)
    payload[kESDSDKPayloadImage] = inBase64ImageString;
  else
    payload[kESDSDKPayloadImage]
      = "data:image/png;base64," + inBase64ImageString;
  jsonObject[kESDSDKCommonPayload] = payload;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::SendToPropertyInspector(
  const std::string& inAction,
  const std::string& inContext,
  const json& inPayload) {
  json jsonObject;
  jsonObject[kESDSDKCommonEvent] = kESDSDKEventSendToPropertyInspector;
  jsonObject[kESDSDKCommonContext] = inContext;
  jsonObject[kESDSDKCommonAction] = inAction;
  jsonObject[kESDSDKCommonPayload] = inPayload;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::ShowAlertForContext(const std::string& inContext) {
  json jsonObject;

  jsonObject[kESDSDKCommonEvent] = kESDSDKEventShowAlert;
  jsonObject[kESDSDKCommonContext] = inContext;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::ShowOKForContext(const std::string& inContext) {
  json jsonObject;

  jsonObject[kESDSDKCommonEvent] = kESDSDKEventShowOK;
  jsonObject[kESDSDKCommonContext] = inContext;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::LogMessage(const std::string& message) {
  json payload;
  payload["message"] = message;
  const json jsonObject{{kESDSDKCommonEvent, kESDSDKEventLogMessage},
                        {kESDSDKCommonPayload, payload}};
  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::GetGlobalSettings() {
  json jsonObject{{kESDSDKCommonEvent, kESDSDKEventGetGlobalSettings},
                  {kESDSDKCommonContext, mPluginUUID}};
  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::SetGlobalSettings(const json& inSettings) {
  json jsonObject;

  jsonObject[kESDSDKCommonEvent] = KESDSDKEventSetGlobalSettings;
  jsonObject[kESDSDKCommonContext] = mPluginUUID;
  jsonObject[kESDSDKCommonPayload] = inSettings;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::SetSettings(
  const json& inSettings,
  const std::string& inContext) {
  json jsonObject;

  jsonObject[kESDSDKCommonEvent] = kESDSDKEventSetSettings;
  jsonObject[kESDSDKCommonContext] = inContext;
  jsonObject[kESDSDKCommonPayload] = inSettings;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}

void ESDConnectionManager::SetState(int inState, const std::string& inContext) {
  json jsonObject;

  json payload;
  payload[kESDSDKPayloadState] = inState;

  jsonObject[kESDSDKCommonEvent] = kESDSDKEventSetState;
  jsonObject[kESDSDKCommonContext] = inContext;
  jsonObject[kESDSDKCommonPayload] = payload;

  websocketpp::lib::error_code ec;
  mWebsocket.send(
    mConnectionHandle, jsonObject.dump(), websocketpp::frame::opcode::text, ec);
}
