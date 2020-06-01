//==============================================================================
/**
@file ESDMain.h

@brief		Parse arguments and start the plugin

@copyright  (c) 2018, Corsair Memory, Inc.
      This source code is licensed under the MIT-style license found in the
LICENSE file.

**/

#pragma once

class ESDBasePlugin;

int esd_main(int argc, const char** argv, ESDBasePlugin* plugin);
