//==============================================================================
/**
@file       ESDLocalizer.cpp

@brief      Utility functions to handle localization

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "ESDLocalizer.h"
#include "ESDUtilities.h"
#include "EPLJSONUtils.h"
#include <fstream>

static ESDLocalizer* sLocalizer = nullptr;

void ESDLocalizer::Initialize(const std::string &inLanguageCode)
{
	if(sLocalizer == nullptr)
	{
		sLocalizer = new ESDLocalizer(inLanguageCode);
	}
}

ESDLocalizer::ESDLocalizer(const std::string &inLanguageCode)
{
	try
	{
		std::string pluginPath = ESDUtilities::GetPluginPath();
		if (!inLanguageCode.empty() && !pluginPath.empty())
		{
			std::string localizationFilePath = ESDUtilities::AddPathComponent(pluginPath, inLanguageCode + ".json");
			std::ifstream localizationFile(localizationFilePath, std::ifstream::in);
			if (localizationFile.is_open())
			{
				json jsonData = json::parse(localizationFile);
				EPLJSONUtils::GetObjectByName(jsonData, "Localization", mLocalizationData);
			}
		}
	}
	catch(...)
	{
	
	}
}

std::string ESDLocalizer::GetLocalizedString(const std::string &inDefaultString)
{
	if (sLocalizer != nullptr)
	{
		return sLocalizer->GetLocalizedStringIntern(inDefaultString);
	}
	
	return inDefaultString;
}

std::string ESDLocalizer::GetLocalizedStringIntern(const std::string &inDefaultString)
{
	return EPLJSONUtils::GetStringByName(mLocalizationData, inDefaultString, inDefaultString);
}
