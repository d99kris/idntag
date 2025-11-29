// tag.h
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <string>

class Tag
{
public:
  static std::string MakePath(const std::string& p_FilePath, std::string& p_Artist,
                              std::string& p_Title);
  static bool Read(const std::string& p_FilePath, std::string& p_Artist,
                   std::string& p_Title);
  static bool Write(const std::string& p_FilePath, const std::string& p_Artist,
                    const std::string& p_Title);
  static bool Clear(const std::string& p_FilePath);

private:
  static std::string SanitizeFileName(const std::string& p_FileName);
};
