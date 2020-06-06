//==============================================================================
/**
@file       ESDUtilities.h

@brief      Various filesystem and other utility functions

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#pragma once


class ESDUtilities
{
public:

	static void DoSleep(int inMilliseconds);
	
	// Return a path with the appending path component
	static std::string AddPathComponent(const std::string &inPath, const std::string &inComponentToAdd);
	
	// Return the path without the last component. Returns path if it is already a root folder (i.e. 'C:\\', '\\ABC' or '/').
	// Return an empty string if error
	static std::string GetFolderPath(const std::string& inPath);
	
	// Get the path of the .sdPlugin bundle
	static std::string GetPluginPath();
};

