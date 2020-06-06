//==============================================================================
/**
@file       ESDUtilities.cpp

@brief      Various filesystem and other utility functions

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "ESDUtilities.h"

void ESDUtilities::DoSleep(int inMilliseconds)
{
	Sleep(inMilliseconds);
}


static bool HasPrefix(const std::string& inString, const std::string& inPrefix)
{
	return (inString.length() >= inPrefix.length()) && (inPrefix.length() > 0) && (inString.compare(0, inPrefix.length(), inPrefix) == 0);
}

static bool HasSuffix(const std::string& inString, const std::string& inSuffix)
{
	return (inString.length() >= inSuffix.length()) && (inSuffix.length() > 0) && (inString.compare(inString.size() - inSuffix.size(), inSuffix.size(), inSuffix) == 0);
}

static char GetFileSystemPathDelimiter()
{
	// In Windows both slash and backslash are allowed
	return '\\';
}


static bool IsNetworkDriveRoot(const std::string& inUtf8Path)
{
	if (!inUtf8Path.empty())
	{
		std::string delimiterString = std::string(1, GetFileSystemPathDelimiter());
		// On Windows, check if the platform specific delimiter is used. If not, fallback on the delimiter "/"
		size_t delimiterPosition = inUtf8Path.find_last_of(delimiterString);
		if (std::string::npos == delimiterPosition)
		{
			delimiterString = "/";
		}

		std::string networkPrefix = delimiterString + delimiterString;
		if (HasPrefix(inUtf8Path, networkPrefix))
		{
			std::string pathWithoutNetworkPrefix = inUtf8Path.substr(networkPrefix.length(), inUtf8Path.length() - networkPrefix.length());
			if (pathWithoutNetworkPrefix.find_first_of(delimiterString) == pathWithoutNetworkPrefix.find_last_of(delimiterString))
				return true;
		}
	}

	return false;
}

static std::string GetFileName(const std::string& inPath)
{
	//
	// Use the platform specific delimiter
	//
	std::string delimiterString = std::string(1, GetFileSystemPathDelimiter());

	//
	// On Windows, check if the platform specific delimiter is used.
	// If not, fallback on the delimiter "/"
	//
	size_t delimiterPosition = inPath.find_last_of(delimiterString);
	if (std::string::npos == delimiterPosition)
	{
		delimiterString = "/";
		delimiterPosition = inPath.find_last_of(delimiterString);
		if (std::string::npos == delimiterPosition)
		{
			// No delimiter -> return the path
			return inPath;
		}
	}
	else
	{
		// Special handling "C:\\"
		if (HasSuffix(inPath, ":\\"))
			return inPath;
	}

	//
	// Remove the trailing delimiters
	//
	std::string pathWithoutTrailingDelimiter = inPath;
	while (pathWithoutTrailingDelimiter.length() > delimiterString.length() && HasSuffix(pathWithoutTrailingDelimiter, delimiterString))
	{
		pathWithoutTrailingDelimiter = pathWithoutTrailingDelimiter.substr(0, pathWithoutTrailingDelimiter.length() - delimiterString.length());
	}

	//
	// Special cases
	//
	if (pathWithoutTrailingDelimiter.empty() || pathWithoutTrailingDelimiter == delimiterString)
	{
		return delimiterString;
	}

	//
	// Get the last path component
	//
	size_t pos = pathWithoutTrailingDelimiter.find_last_of(delimiterString);
	if (std::string::npos != pos && pathWithoutTrailingDelimiter.length() > pos)
	{
		return pathWithoutTrailingDelimiter.substr(pos + 1);
	}

	//
	// Fallback
	//
	return pathWithoutTrailingDelimiter;
}

static std::string GetExtension(const std::string& inPath)
{
	const std::string fileName = GetFileName(inPath);
	size_t pos = fileName.find_last_of(".");
	if (std::string::npos != pos && ((pos + 1) != fileName.length()))
	{
		return fileName.substr(pos);
	}
	else
	{
		return "";
	}
}

std::string ESDUtilities::AddPathComponent(const std::string &inPath, const std::string &inComponentToAdd)
{
	if (inPath.size() <= 0)
		return inComponentToAdd;

	char delimiter = GetFileSystemPathDelimiter();
	char lastChar = inPath[inPath.size() - 1];
	
	bool pathEndsWithDelimiter   = (delimiter == lastChar			) || ('/' == lastChar);
	bool compStartsWithDelimiter = (delimiter == inComponentToAdd[0]) || ('/' == inComponentToAdd[0]);

	std::string result;
	if (pathEndsWithDelimiter && compStartsWithDelimiter)
		result = inPath + inComponentToAdd.substr(1);
	else if (pathEndsWithDelimiter || compStartsWithDelimiter)
		result = inPath + inComponentToAdd;
	else
		result = inPath + GetFileSystemPathDelimiter() + inComponentToAdd;

	for (std::string::size_type i=0; i< result.size(); i++)
	{
		if (result[i] == '/')
			result[i] = delimiter;
	}

	return result;
}

std::string ESDUtilities::GetFolderPath(const std::string& inPath)
{
	//
	// Use the platform specific delimiter
	//
	std::string delimiterString = std::string(1, GetFileSystemPathDelimiter());

	//
	// On Windows, check if the platform specific delimiter is used.
	// If not, fallback on the delimiter "/"
	//
	size_t delimiterPosition = inPath.find_last_of(delimiterString);
	if (std::string::npos == delimiterPosition)
	{
		delimiterString = "/";
	}

	// Special handling "C:\\" or "C:/"
	if (HasSuffix(inPath, ":" + delimiterString))
		return inPath;

	if (IsNetworkDriveRoot(inPath))
		return inPath;

	//
	// Remove the trailing delimiters
	//
	std::string pathWithoutTrailingDelimiters = inPath;
	while (pathWithoutTrailingDelimiters.length() > delimiterString.length() && HasSuffix(pathWithoutTrailingDelimiters, delimiterString))
	{
		pathWithoutTrailingDelimiters = pathWithoutTrailingDelimiters.substr(0, pathWithoutTrailingDelimiters.length() - delimiterString.length());
	}

	size_t pos = pathWithoutTrailingDelimiters.find_last_of(delimiterString);
	if (std::string::npos != pos)
	{
		std::string foundPath = inPath.substr(0, pos);

		// Special handling "C:\\"
		if (HasSuffix(foundPath, ":\\"))
			return foundPath;
		else if (HasSuffix(foundPath, ":"))
			return foundPath + "\\";

		//
		// Remove the trailing delimiters
		//
		std::string foundPathWithoutTrailingDelimiters = foundPath;
		while (foundPathWithoutTrailingDelimiters.length() > delimiterString.length() && HasSuffix(foundPathWithoutTrailingDelimiters, delimiterString))
		{
			foundPathWithoutTrailingDelimiters = foundPathWithoutTrailingDelimiters.substr(0, foundPathWithoutTrailingDelimiters.length() - delimiterString.length());
		}

		if (foundPathWithoutTrailingDelimiters.empty() && delimiterString == "/")
		{
			return "/";
		}

		return foundPathWithoutTrailingDelimiters;
	}
	
	// Special handling "C:"
	if (HasSuffix(pathWithoutTrailingDelimiters, ":"))
		return pathWithoutTrailingDelimiters + delimiterString;

	return "";
}


std::string ESDUtilities::GetPluginPath()
{
	static std::string sPluginPath;
	
	if(sPluginPath.empty())
	{
		char path[MAX_PATH] = { 0 };
		DWORD result = GetModuleFileNameA(NULL, path, MAX_PATH);
		if (result > 0)
		{
			std::string pathString(path);

			while (!pathString.empty())
			{
				if (pathString == "/" || HasSuffix(pathString, ":\\"))
				{
					break;
				}

				std::string pathExtension = GetExtension(pathString);
				if (pathExtension == ".sdPlugin")
				{
					sPluginPath = pathString;
					break;
				}

				std::string parentPath = GetFolderPath(pathString);
				if (parentPath != pathString)
				{
					pathString = parentPath;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			DebugPrint("Could not get path.\n");
		}
	}

	return sPluginPath;
}
