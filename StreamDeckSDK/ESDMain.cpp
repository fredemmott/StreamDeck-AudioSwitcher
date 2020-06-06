//==============================================================================
/**
@file       main.cpp

@brief		Parse arguments and start the plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include "EPLJSONUtils.h"
#include "ESDConnectionManager.h"
#include "ESDLocalizer.h"
#include "ESDLogger.h"

int esd_main(int argc, const char** argv, ESDBasePlugin* plugin) {
  if (argc != 9) {
    ESDDebug("Invalid number of parameters %d instead of 9\n", argc);
    return 1;
  }

  int port = 0;
  std::string pluginUUID;
  std::string registerEvent;
  std::string info;

  for (int argumentIndex = 0; argumentIndex < 4; argumentIndex++) {
    std::string parameter(argv[1 + 2 * argumentIndex]);
    std::string value(argv[1 + 2 * argumentIndex + 1]);

    if (parameter == kESDSDKPortParameter) {
      port = std::atoi(value.c_str());
    } else if (parameter == kESDSDKPluginUUIDParameter) {
      pluginUUID = value;
    } else if (parameter == kESDSDKRegisterEventParameter) {
      registerEvent = value;
    } else if (parameter == kESDSDKInfoParameter) {
      info = value;
    }
  }

  if (port == 0) {
    ESDDebug("Invalid port number\n");
    return 1;
  }
  if (pluginUUID.empty()) {
    ESDDebug("Invalid plugin UUID\n");
    return 1;
  }

  if (registerEvent.empty()) {
    ESDDebug("Invalid registerEvent\n");
    return 1;
  }

  if (info.empty()) {
    ESDDebug("Invalid info\n");
    return 1;
  }
  // Initialize localization helper
  std::string language = "en";

  try {
    json infoJson = json::parse(info);
    json applicationInfo;
    if (EPLJSONUtils::GetObjectByName(
          infoJson, kESDSDKApplicationInfo, applicationInfo)) {
      language = EPLJSONUtils::GetStringByName(
        applicationInfo, kESDSDKApplicationInfoLanguage, language);
    }
  } catch (...) {
  }

  ESDLocalizer::Initialize(language);

  // Create the connection manager
  ESDConnectionManager* connectionManager
    = new ESDConnectionManager(port, pluginUUID, registerEvent, info, plugin);
  ESDLogger::Get()->SetConnectionManager(connectionManager);

  // Connect and start the event loop
  connectionManager->Run();

  return 0;
}
