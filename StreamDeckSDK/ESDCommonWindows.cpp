//==============================================================================
/**
@file       pch.cpp

@brief		Precompiled header

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/
//==============================================================================

#include "ESDCommonWindows.h"

// Include order matters
// clang-format off
#include <strsafe.h>
#include <Windows.h>
// clang-format on

void __cdecl dbgprintf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);

  char msg[1024];
  vsnprintf(msg, sizeof(msg), format, arg);
  OutputDebugStringA(msg);

  va_end(arg);
}
