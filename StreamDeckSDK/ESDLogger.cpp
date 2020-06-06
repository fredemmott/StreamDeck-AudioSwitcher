// @copyright  (c) 2018, Corsair Memory, Inc.
// @copyright  (c) 2020, Frederick Emmott
// This source code is licensed under the MIT-style license found in
// the LICENSE file.

#include "ESDLogger.h"

#include "ESDConnectionManager.h"
#include "ESDUtilities.h"

#ifdef __APPLE__
#include <cstdio>
#include <os/log.h>
#else
#include <strsafe.h>
#include <Windows.h>
#endif

namespace {
  ESDLogger* sLogger = nullptr;
}

ESDLogger* ESDLogger::Get() {
  if (!sLogger) {
    sLogger = new ESDLogger();
  }
  return sLogger;
}

ESDLogger::ESDLogger() {
}

void ESDLogger::SetWin32DebugPrefix(const std::string& prefix) {
  mPrefix = prefix;
}

void ESDLogger::SetConnectionManager(ESDConnectionManager* conn) {
  mConnectionManager = conn;
}

void ESDLogger::LogToStreamDeckSoftware(const std::string& message) {
  if (!mConnectionManager) {
    return;
  }
  mConnectionManager->LogMessage(message);
}

void ESDLogger::LogMessage(const char* file, ESDLOGGER_FORMAT_STRING(format), ...) {
  va_list args;
  va_start(args, format);
  char buf[1024];
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  std::string message(ESDUtilities::GetFileName(file)+": "+buf);
  this->LogToStreamDeckSoftware(message);
#ifndef NDEBUG
  this->LogToSystem(message);
#endif
}

void ESDLogger::LogToSystem(const std::string& message) {
#ifdef __APPLE__
  os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_DEFAULT, "%s", message.c_str());
#else
  OutputDebugStringA((mPrefix + message).c_str());
#endif
}
