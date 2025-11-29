// acoustid.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "acoustid.h"

#include <sstream>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "log.h"
#include "util.h"

bool AcoustId::Identify(const std::string& p_FilePath, std::string& p_Artist,
                        std::string& p_Title)
{
  Fingerprint fingerprint;
  if (!GetFingerprint(p_FilePath, fingerprint))
  {
    return false;
  }

  std::vector<Match> matches;
  if (!LookupFingerprint(fingerprint, matches))
  {
    return false;
  }

  Match match;
  if (!GetBestMatch(matches, match))
  {
    return false;
  }

  p_Artist = match.artist;
  p_Title = match.title;

  return true;
}

bool AcoustId::GetFingerprint(const std::string& p_FilePath, Fingerprint& p_Fingerprint)
{
  const std::string cmd = "fpcalc -json \"" + p_FilePath + "\"";
  const std::string jsonStr = Util::RunCommand(cmd);
  if (jsonStr.empty())
  {
    Log::Debug("command '%s' failed", cmd.c_str());
    return false;
  }

  nlohmann::json jsonDoc = nlohmann::json::parse(jsonStr);
  const std::string fp = jsonDoc.at("fingerprint").get<std::string>();
  const int dur = jsonDoc.at("duration").get<int>();
  if (fp.empty() || (dur == 0))
  {
    Log::Debug("empty fingerprint in json: %s", jsonStr.c_str());
    return false;
  }

  p_Fingerprint.fp = fp;
  p_Fingerprint.duration_sec = dur;

  return true;
}

bool AcoustId::LookupFingerprint(const Fingerprint& p_Fingerprint,
                                 std::vector<Match>& p_Matches)
{
  static Util::RateLimiter rateLimiter(std::chrono::milliseconds(1000 / 3));
  rateLimiter.Wait();

  CURL* curl = curl_easy_init();
  if (!curl)
  {
    Log::Debug("curl init failed");
    return false;
  }

  std::string response;
  curl_easy_setopt(curl, CURLOPT_URL, "https://api.acoustid.org/v2/lookup");
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteString);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  static std::string api_key = Util::StrFromHex("486536493641594B4E31");

  std::ostringstream body;
  body << "client=" << curl_easy_escape(curl, api_key.c_str(), 0)
       << "&fingerprint=" << curl_easy_escape(curl, p_Fingerprint.fp.c_str(), 0)
       << "&duration=" << p_Fingerprint.duration_sec
       << "&meta=recordings+releasegroups+compress"
       << "&format=json";
  const std::string postfields = body.str();
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());

  CURLcode rc = curl_easy_perform(curl);
  if (rc != CURLE_OK)
  {
    Log::Debug("curl request failed");
    curl_easy_cleanup(curl);
    return false;
  }

  curl_easy_cleanup(curl);

  if (response.empty())
  {
    Log::Debug("curl response empty");
    return false;
  }

  nlohmann::json jsonDoc = nlohmann::json::parse(response);
  if (!jsonDoc.contains("results") || jsonDoc["results"].empty())
  {
    Log::Debug("acoustid no results (%s)", response.c_str());
    return false;
  }

  const nlohmann::json& results = jsonDoc["results"];
  for (const nlohmann::json& result : results)
  {
    Match match;

    if (!result.contains("recordings") || result["recordings"].empty()) continue;

    const nlohmann::json& recording = result["recordings"][0];

    if (recording.contains("artists") && !recording["artists"].empty())
    {
      match.artist = recording["artists"][0].value("name", "");
    }

    if (recording.contains("title") && !recording["title"].empty())
    {
      match.title = recording["title"];
    }

    if (match.artist.empty() || match.title.empty())
    {
      continue;
    }

    match.score = result.value("score", 0.0);
    p_Matches.push_back(match);
  }

  if (p_Matches.empty())
  {
    Log::Debug("acoustid no valid matches");
    return false;
  }

  return true;
}

bool AcoustId::GetBestMatch(const std::vector<Match>& p_Matches, Match& p_BestMatch)
{
  if (p_Matches.empty())
  {
    return false;
  }

  p_BestMatch = p_Matches[0];
  for (const auto& match : p_Matches)
  {
    if (match.score > p_BestMatch.score)
    {
      p_BestMatch = match;
    }
  }

  return true;
}

size_t AcoustId::CurlWriteString(void* ptr, size_t size, size_t nmemb, void* userdata)
{
  auto* s = static_cast<std::string*>(userdata);
  s->append(static_cast<char*>(ptr), size*nmemb);
  return size*nmemb;
}
