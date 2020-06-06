//==============================================================================
/**
@file       ESDUtilities.cpp

@brief      Various filesystem and other utility functions

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "ESDUtilities.h"
#include <CoreFoundation/CoreFoundation.h>


static std::string CFStringGetStdString(CFStringRef inStringRef, CFStringEncoding inEncoding)
{
    if (inStringRef == NULL)
        return "";

    std::string outString;

    CFRange range = CFRangeMake(0, CFStringGetLength(inStringRef));
    while (range.length > 0)
    {
        UInt8 buffer[512];

        CFIndex produced;
        CFIndex consumed = CFStringGetBytes(inStringRef, range, inEncoding, 0, false, buffer, sizeof(buffer), &produced);
        if (consumed == 0)
            return "";

        outString += std::string((const char*)buffer, produced);

        range.location += consumed;
        range.length -= consumed;
    }

    return outString;
}

static bool HasSuffix(const std::string& inString, const std::string& inSuffix)
{
	return (inString.length() >= inSuffix.length()) && (inSuffix.length() > 0) && (inString.compare(inString.size() - inSuffix.size(), inSuffix.size(), inSuffix) == 0);
}

void ESDUtilities::DoSleep(int inMilliseconds)
{
	usleep(1000 * inMilliseconds);
}


static char GetFileSystemPathDelimiter()
{
	return '/';
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
	
	return result;
}

std::string ESDUtilities::GetFolderPath(const std::string& inPath)
{
	//
	// Use the platform specific delimiter
	//
	std::string delimiterString = std::string(1, GetFileSystemPathDelimiter());

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

	return "";
}

std::string ESDUtilities::GetPluginPath()
{
	static std::string sPluginPath;
	
	if(sPluginPath.empty())
	{
		CFBundleRef bundleRef = CFBundleGetMainBundle();
		if(bundleRef != NULL)
		{
			CFURLRef executableURL = CFBundleCopyExecutableURL(bundleRef);
			if(executableURL != NULL)
			{
				CFURLRef checkURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, executableURL);
				while(checkURL != NULL)
				{
					CFStringRef lastPathComponent = CFURLCopyLastPathComponent(checkURL);
					if(lastPathComponent == NULL || (CFStringCompare(lastPathComponent, CFSTR("/"), 0) == kCFCompareEqualTo) || (CFStringCompare(lastPathComponent, CFSTR(".."), 0) == kCFCompareEqualTo))
					{
						if(lastPathComponent != NULL)
						{
							CFRelease(lastPathComponent);
						}
						
						CFRelease(checkURL);
						checkURL = NULL;
						break;
					}
					
					CFRelease(lastPathComponent);
					
					CFStringRef pathExtension = CFURLCopyPathExtension(checkURL);
					if(pathExtension != NULL)
					{
						if(CFStringCompare(pathExtension, CFSTR("sdPlugin"), 0) == kCFCompareEqualTo)
						{
							CFStringRef path = CFURLCopyFileSystemPath(checkURL, kCFURLPOSIXPathStyle);
							if(path != NULL)
							{
								sPluginPath = CFStringGetStdString(path, kCFStringEncodingUTF8);
								CFRelease(path);
							}
						}
						
						CFRelease(pathExtension);
					}
					
					CFURLRef previousURL = checkURL;
					checkURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, previousURL);
					CFRelease(previousURL);
					
					if(!sPluginPath.empty())
					{
						CFRelease(checkURL);
						checkURL = NULL;
						break;
					}
				}
				
				CFRelease(executableURL);
			}
		}
	}
	
	return sPluginPath;
}

