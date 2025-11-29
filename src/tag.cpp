// tag.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "tag.h"

#include <filesystem>
#include <regex>

#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>
#include <taglib/tag.h>

#include "log.h"

std::string Tag::MakePath(const std::string& p_FilePath, std::string& p_Artist,
                          std::string& p_Title)
{
  const std::filesystem::path oldPath(p_FilePath);
  const std::filesystem::path directory = oldPath.parent_path();

  std::string artist = SanitizeFileName(p_Artist);
  if (artist.empty())
  {
    artist = "Unknown";
  }

  std::string title = SanitizeFileName(p_Title);
  if (title.empty())
  {
    title = "Unknown";
  }

  const std::string baseName = artist + "-" + title;
  const std::string extension = ".mp3";

  // Start with the base filename
  std::filesystem::path outputPath = directory / (baseName + extension);

  // If it exists, append _1, _2, _3 ...
  int counter = 1;
  while (std::filesystem::exists(outputPath))
  {
    const std::string numbered = baseName + "_" + std::to_string(counter) + extension;
    outputPath = directory / numbered;
    ++counter;
  }

  return outputPath.string();
}

bool Tag::Read(const std::string& p_FilePath, std::string& p_Artist, std::string& p_Title)
{
  TagLib::MPEG::File file(p_FilePath.c_str());
  if (!file.isValid())
  {
    return false;
  }

  // Prefer ID3v2 tag; if not present, fall back to generic tag (e.g. ID3v1)
  TagLib::Tag* tag = file.ID3v2Tag(false);
  if (!tag)
  {
    tag = file.tag();
  }

  if (!tag)
  {
    return false;
  }

  const TagLib::String artistStr = tag->artist();
  const TagLib::String titleStr = tag->title();

  // true => return UTF-8 in Unicode-aware builds
  p_Artist = artistStr.to8Bit(true);
  p_Title = titleStr.to8Bit(true);

  if (p_Artist.empty() || p_Title.empty())
  {
    return false;
  }

  return true;
}

bool Tag::Write(const std::string& p_FilePath, const std::string& p_Artist,
                const std::string& p_Title)
{
  TagLib::MPEG::File file(p_FilePath.c_str());
  if (!file.isValid())
  {
    return false;
  }

  TagLib::Tag* tag = file.ID3v2Tag(true);
  if (!tag)
  {
    return false;
  }

  tag->setArtist(TagLib::String(p_Artist, TagLib::String::UTF8));
  tag->setTitle(TagLib::String(p_Title, TagLib::String::UTF8));

  if (!file.save())
  {
    return false;
  }

  return true;
}

bool Tag::Clear(const std::string& p_FilePath)
{
  TagLib::MPEG::File file(p_FilePath.c_str());
  if (!file.isValid())
  {
    return false;
  }

  // Remove all tag types
  file.strip(TagLib::MPEG::File::AllTags);

  if (!file.save())
  {
    return false;
  }

  return true;
}

static std::u32string Utf8ToUtf32(const std::string& s)
{
  std::u32string out;
  size_t i = 0;

  while (i < s.size())
  {
    unsigned char c = s[i];

    if (c < 0x80)
    {
      // 1-byte ASCII
      out.push_back(c);
      i++;
    }
    else if ((c >> 5) == 0x6 && i + 1 < s.size())
    {
      // 2 bytes
      char32_t cp =
        ((c & 0x1F) << 6) |
        (s[i+1] & 0x3F);
      out.push_back(cp);
      i += 2;
    }
    else if ((c >> 4) == 0xE && i + 2 < s.size())
    {
      // 3 bytes
      char32_t cp =
        ((c & 0x0F) << 12) |
        ((s[i+1] & 0x3F) << 6) |
        (s[i+2] & 0x3F);
      out.push_back(cp);
      i += 3;
    }
    else if ((c >> 3) == 0x1E && i + 3 < s.size())
    {
      // 4 bytes
      char32_t cp =
        ((c & 0x07) << 18) |
        ((s[i+1] & 0x3F) << 12) |
        ((s[i+2] & 0x3F) << 6) |
        (s[i+3] & 0x3F);
      out.push_back(cp);
      i += 4;
    }
    else
    {
      // malformed UTF-8 byte → skip
      i++;
    }
  }

  return out;
}

static std::string Utf32ToUtf8(const std::u32string& s)
{
  std::string out;

  for (char32_t cp : s)
  {
    if (cp < 0x80)
    {
      out.push_back((char)cp);
    }
    else if (cp < 0x800)
    {
      out.push_back((char)(0xC0 | (cp >> 6)));
      out.push_back((char)(0x80 | (cp & 0x3F)));
    }
    else if (cp < 0x10000)
    {
      out.push_back((char)(0xE0 | (cp >> 12)));
      out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back((char)(0x80 | (cp & 0x3F)));
    }
    else
    {
      out.push_back((char)(0xF0 | (cp >> 18)));
      out.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
      out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back((char)(0x80 | (cp & 0x3F)));
    }
  }

  return out;
}

std::string Tag::SanitizeFileName(const std::string& p_FileName)
{
  // UTF-8 → UTF-32
  std::u32string u32 = Utf8ToUtf32(p_FileName);

  std::u32string out32;
  for (char32_t c : u32)
  {
    // Strip control chars and path separators
    if ((c >= 0 && c < 0x20) || c == U'/' || c == U'\\' || c == 0x7F)
    {
      continue;
    }

    // Allow some nice ASCII punctuation explicitly
    if (c == U' ' || c == U'.' || c == U',' ||
        c == U'-' || c == U'_' || c == U'\'')
    {
      out32.push_back(c);
      continue;
    }

    // On POSIX (Linux/macOS) we can safely keep all other Unicode codepoints.
    // This preserves CJK, accents, etc. directly in the filename.
    out32.push_back(c);
  }

  // UTF-32 → UTF-8
  std::string out = Utf32ToUtf8(out32);

  // Whitespace and formatting cleanup
  out = std::regex_replace(out, std::regex(" +"), "_");
  out = std::regex_replace(out, std::regex("_+"), "_");
  out = std::regex_replace(out, std::regex("^_+|_+$"), "");

  if (out.empty())
    out = "unnamed";

  return out;
}
