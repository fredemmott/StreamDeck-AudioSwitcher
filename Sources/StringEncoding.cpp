/* Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#include "StringEncoding.h"

#include "Windows.h"

namespace FredEmmott::Encoding {

std::string Utf16ToUtf8(const std::wstring& utf16) {
  if (utf16.empty()) {
    return std::string();
  }
  int utf8_len
    = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), utf16.size(), 0, 0, 0, 0);
  std::string buf(utf8_len, 0);
  WideCharToMultiByte(CP_UTF8, 0, utf16.data(), -1, buf.data(), utf8_len, 0, 0);
  return buf;
}

std::wstring Utf8ToUtf16(const std::string& utf8) {
  if (utf8.empty()) {
    return std::wstring();
  }
  int wchar_len
    = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(), 0, 0);
  std::wstring buf(wchar_len, 0);
  MultiByteToWideChar(
    CP_UTF8, 0, utf8.data(), utf8.size(), buf.data(), wchar_len);
  return buf;
}
}// namespace FredEmmott::Encoding
