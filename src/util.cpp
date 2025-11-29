// util.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "util.h"

#include <algorithm>
#include <filesystem>
#include <sstream>

Util::RateLimiter::RateLimiter(std::chrono::milliseconds minInterval)
  : m_MinInterval(minInterval)
  , m_LastCall(std::chrono::steady_clock::time_point::min())
{
}

void Util::RateLimiter::Wait()
{
  auto now = std::chrono::steady_clock::now();

  if (m_LastCall != std::chrono::steady_clock::time_point::min())
  {
    auto nextAllowed = m_LastCall + m_MinInterval;
    if (now < nextAllowed)
    {
      std::this_thread::sleep_for(nextAllowed - now);
    }
  }

  m_LastCall = std::chrono::steady_clock::now();
}

bool Util::Exists(const std::string& p_Path)
{
  return std::filesystem::exists(p_Path) &&
         (std::filesystem::is_regular_file(p_Path) || std::filesystem::is_directory(p_Path));
}

std::string Util::GetFileExt(const std::string& p_Path)
{
  size_t lastPeriod = p_Path.find_last_of(".");
  if (lastPeriod == std::string::npos) return "";

  return p_Path.substr(lastPeriod);
}

void Util::ListFiles(const std::string& p_Path, std::set<std::string>& p_Paths)
{
  std::filesystem::path path(p_Path);
  if (std::filesystem::is_regular_file(path))
  {
    p_Paths.insert(std::filesystem::canonical(path).string());
  }
  else if (std::filesystem::is_directory(path))
  {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
    {
      if (entry.is_regular_file())
      {
        p_Paths.insert(std::filesystem::canonical(entry.path()).string());
      }
    }
  }
}

std::string Util::MakeReport(const std::string& p_Format, const std::string& p_InFilePath,
                             const std::string& p_OutFilePath, bool p_Result)
{
  std::string report = p_Format;

  Replace(report, "%i", p_InFilePath);
  Replace(report, "%o", p_OutFilePath);
  Replace(report, "%r", p_Result ? "PASS" : "FAIL");

  return report;
}

void Util::Replace(std::string& p_Str, const std::string& p_Search,
                   const std::string& p_Replace)
{
  size_t pos = 0;
  while ((pos = p_Str.find(p_Search, pos)) != std::string::npos)
  {
    p_Str.replace(pos, p_Search.length(), p_Replace);
    pos += p_Replace.length();
  }
}

bool Util::Rename(const std::string& p_OldPath, const std::string& p_NewPath)
{
  try
  {
    std::filesystem::rename(std::filesystem::path(p_OldPath), std::filesystem::path(p_NewPath));
    return true;
  }
  catch (const std::filesystem::filesystem_error&)
  {
    return false;
  }
}

std::string Util::RunCommand(const std::string& p_Cmd)
{
  FILE* fp = popen(p_Cmd.c_str(), "r");
  if (fp == nullptr)
  {
    return "";
  }

  char buf[4096];
  std::string out;
  while (size_t n = fread(buf, 1, sizeof(buf), fp))
  {
    out.append(buf, n);
  }

  int rc = pclose(fp);
  if (rc == -1)
  {
    return "";
  }

  return out;
}

std::string Util::StrFromHex(const std::string& p_String)
{
  std::string result;
  std::istringstream iss(p_String);
  char buf[3] = { 0 };
  while (iss.read(buf, 2))
  {
    result += static_cast<char>(strtol(buf, NULL, 16) & 0xff);
  }

  return result;
}

std::string Util::ToLower(const std::string& p_Str)
{
  std::string lower = p_Str;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  return lower;
}
