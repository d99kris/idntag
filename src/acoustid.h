// acoustid.h
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <string>
#include <vector>

class AcoustId
{
private:
  struct Fingerprint
  {
    std::string fp;
    int duration_sec = 0;
  };

  struct Match
  {
    std::string title;
    std::string artist;
    double score = 0.0;
  };

public:
  static bool Identify(const std::string& p_FilePath, std::string& p_Artist,
                       std::string& p_Title);

private:
  static bool GetFingerprint(const std::string& p_FilePath, Fingerprint& p_Fingerprint);
  static bool LookupFingerprint(const Fingerprint& p_Fingerprint,
                                std::vector<Match>& p_Matches);
  static bool GetBestMatch(const std::vector<Match>& p_Matches, Match& p_BestMatch);
  static size_t CurlWriteString(void* ptr, size_t size, size_t nmemb, void* userdata);
};
