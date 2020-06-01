//==============================================================================
/**
@file       pch.h

@brief		Precompiled header

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#ifndef PCH_H
#define PCH_H

//-------------------------------------------------------------------
// C++ headers
//-------------------------------------------------------------------

#include <strsafe.h>
#include <winsock2.h>
#include <set>
#include <string>
#include <thread>
#include <Windows.h>

//-------------------------------------------------------------------
// Debug logging
//-------------------------------------------------------------------

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

void __cdecl dbgprintf(const char *format, ...);

#if DEBUG
#define DebugPrint dbgprintf
#else
#define DebugPrint(...) while (0)
#endif

//-------------------------------------------------------------------
// json
//-------------------------------------------------------------------

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#endif// PCH_H
