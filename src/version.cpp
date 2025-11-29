// version.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "version.h"

#define APP_VERSION "2.00"

std::string Version::GetAppName(bool p_WithVersion)
{
  return std::string("idntag") + (p_WithVersion ? (" " + GetAppVersion()) : "");
}

std::string Version::GetAppVersion()
{
  static std::string version = APP_VERSION;
  return version;
}
