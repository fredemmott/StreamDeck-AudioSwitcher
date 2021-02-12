/* Copyright (c) 2020-present, Fred Emmott
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file.
 */

#pragma once

#include <string>

namespace FredEmmott::Encoding {
  std::string Utf16ToUtf8(const std::wstring& utf16);
  std::wstring Utf8ToUtf16(const std::string& utf8);
}
