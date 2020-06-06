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
// Debug logging
//-------------------------------------------------------------------

void __cdecl dbgprintf(const char *format, ...);

#if DEBUG
#define DebugPrint dbgprintf
#else
#define DebugPrint(...) while (0)
#endif
