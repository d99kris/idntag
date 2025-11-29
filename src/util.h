// util.h
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <chrono>
#include <set>
#include <string>
#include <thread>

class Util
{
public:
  class RateLimiter
  {
  public:
    explicit RateLimiter(std::chrono::milliseconds minInterval);
    void Wait();

  private:
    std::chrono::milliseconds m_MinInterval;
    std::chrono::steady_clock::time_point m_LastCall;
  };

public:
  static bool Exists(const std::string& p_Path);
  static std::string GetFileExt(const std::string& p_Path);
  static void ListFiles(const std::string& p_Path, std::set<std::string>& p_Paths);
  static std::string MakeReport(const std::string& p_Format, const std::string& p_InFilePath,
                                const std::string& OutFilePath, bool p_Result);
  static void Replace(std::string& p_Str, const std::string& p_Search,
                      const std::string& p_Replace);
  static bool Rename(const std::string& p_OldPath, const std::string& p_NewPath);
  static std::string RunCommand(const std::string& p_Cmd);
  static std::string StrFromHex(const std::string& p_String);
  static std::string ToLower(const std::string& p_Str);
};
