//==============================================================================
/**
@file       pch.h

@brief		Precompiled header

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#pragma once

//-------------------------------------------------------------------
// C++ headers
//-------------------------------------------------------------------

// Include order matters
// clang-format off
#include <strsafe.h>
#include <winsock2.h>
#include <Windows.h>
// clang-format on

#include <set>
#include <string>
#include <thread>

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
