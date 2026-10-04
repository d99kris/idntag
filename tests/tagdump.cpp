// tagdump.cpp
//
// Copyright (c) 2026 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

// Test helper printing a field from a specific tag type of an mp3 file.
// Prints nothing if the file does not have the specified tag type.

#include <iostream>
#include <string>

#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/mpegfile.h>

static int Usage()
{
  std::cerr << "usage: tagdump <id3v1|id3v2> <artist|title|album|year> FILE\n";
  return 1;
}

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    return Usage();
  }

  const std::string tagType = argv[1];
  const std::string field = argv[2];
  TagLib::MPEG::File file(argv[3]);
  if (!file.isValid())
  {
    std::cerr << "tagdump: invalid file " << argv[3] << "\n";
    return 1;
  }

  TagLib::Tag* tag = nullptr;
  if (tagType == "id3v1")
  {
    tag = file.hasID3v1Tag() ? file.ID3v1Tag() : nullptr;
  }
  else if (tagType == "id3v2")
  {
    tag = file.hasID3v2Tag() ? file.ID3v2Tag() : nullptr;
  }
  else
  {
    return Usage();
  }

  TagLib::String value;
  if (field == "artist")
  {
    value = tag ? tag->artist() : TagLib::String();
  }
  else if (field == "title")
  {
    value = tag ? tag->title() : TagLib::String();
  }
  else if (field == "album")
  {
    value = tag ? tag->album() : TagLib::String();
  }
  else if (field == "year")
  {
    value = (tag && (tag->year() > 0)) ? TagLib::String::number(tag->year()) : TagLib::String();
  }
  else
  {
    return Usage();
  }

  std::cout << value.to8Bit(true) << "\n";
  return 0;
}
