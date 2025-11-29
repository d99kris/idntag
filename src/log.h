// log.h
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#pragma once

class Log
{
public:
  static void Debug(const char* p_Format, ...);
  static void SetVerbose(bool p_Verbose);

private:
  static bool m_Verbose;
};
