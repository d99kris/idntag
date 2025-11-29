// main.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "main.h"

#include <iostream>
#include <set>
#include <string>

#include "acoustid.h"
#include "editor.h"
#include "log.h"
#include "tag.h"
#include "util.h"
#include "version.h"

static void ShowHelp(bool p_Verbose);
static void ShowVersion();

int main(int argc, char* argv[])
{
  bool clear = false;
  bool detect = false;
  bool edit = false;
  bool rename = false;
  std::string reportFormat = "%i : %r : %o";
  std::string invalidarg;
  std::set<std::string> filePaths;

  // Parse arguments
  std::vector<std::string> args(argv + 1, argv + argc);
  for (auto it = args.begin(); it != args.end(); ++it)
  {
    const std::string& arg = *it;
    const bool hasNextArg = (std::distance(it + 1, args.end()) > 0);
    if ((arg == "-c") || (arg == "--clear"))
    {
      clear = true;
    }
    else if ((arg == "-d") || (arg == "--detect"))
    {
      detect = true;
    }
    else if ((arg == "-e") || (arg == "--edit"))
    {
      edit = true;
    }
    else if ((arg == "-h") || (arg == "--help"))
    {
      ShowHelp(true /*p_Verbose*/);
      return 0;
    }
    else if ((arg == "-r") || (arg == "--rename"))
    {
      rename = true;
    }
    else if (((arg == "-R") || (arg == "--report")) && hasNextArg)
    {
      ++it;
      reportFormat = *it;
    }
    else if ((arg == "-v") || (arg == "--verbose"))
    {
      Log::SetVerbose(true);
    }
    else if ((arg == "-V") || (arg == "--version"))
    {
      ShowVersion();
      return 0;
    }
    else if (Util::Exists(arg))
    {
      Util::ListFiles(arg, filePaths);
    }
    else
    {
      invalidarg = arg;
      break;
    }
  }

  // Handle errors
  if (!invalidarg.empty())
  {
    std::cerr << "ERROR: Invalid argument '" << invalidarg << "'\n\n";
    ShowHelp(false /*p_Verbose*/);
    return 1;
  }
  else if (filePaths.empty())
  {
    std::cerr << "ERROR: No path(s) specified\n\n";
    ShowHelp(false /*p_Verbose*/);
    return 2;
  }
  else if (!clear && !detect && !edit && !rename)
  {
    std::cerr <<
      "ERROR: Requires at least one operation of:\n"
      "--clear, --detect, --edit or --rename\n\n";
    ShowHelp(false /*p_Verbose*/);
    return 3;
  }

  // Process input files
  bool resultAll = true;
  for (const auto& filePath : filePaths)
  {
    std::string artist;
    std::string title;
    bool result = true;

    if (Util::ToLower(Util::GetFileExt(filePath)) != ".mp3")
    {
      result = false;
    }

    if (result && clear)
    {
      result = Tag::Clear(filePath);
    }

    if (result && (detect || edit || rename))
    {
      Tag::Read(filePath, artist, title);
      result = detect || edit || (!artist.empty() && !title.empty());
    }

    if (result && detect)
    {
      result = AcoustId::Identify(filePath, artist, title);
    }

    if (result && edit)
    {
      result = Editor::Edit(filePath, artist, title);
    }

    std::string newFilePath = filePath;
    if (result && (detect || edit || rename))
    {
      result = Tag::Write(filePath, artist, title);

      if (result && rename)
      {
        newFilePath = Tag::MakePath(filePath, artist, title);
        result = Util::Rename(filePath, newFilePath);
      }
    }

    const std::string report = Util::MakeReport(reportFormat, filePath, newFilePath, result);
    if (!report.empty())
    {
      std::cout << report << "\n";
    }

    resultAll = resultAll && result;
  }

  return (resultAll ? 0 : 1);
}

void ShowHelp(bool p_Verbose)
{
  if (p_Verbose)
  {
    std::cout <<
      "idntag identifies, tags and renames audio files.\n"
      "\n";
  }

  {
    std::cout <<
      "Usage: idntag [OPTIONS] PATHS...\n"
      "\n"
      "Command-line options:\n"
      "    -c, --clear            clear tags\n"
      "    -d, --detect           detect / identify audio\n"
      "    -e, --edit             edit / confirm detected tags\n"
      "    -r, --rename           rename file based on tags\n"
      "\n"
      "    -R, --report           specify report format\n"
      "    -h, --help             display help\n"
      "    -v, --verbose          enable verbose debug output\n"
      "    -V, --version          display version information\n"
      "    PATHS                  files or directories to process\n"
      "\n";
  }

  if (p_Verbose)
  {
    std::cout <<
      "Output format fields:\n"
      "    %i          input file name\n"
      "    %o          output file name\n"
      "    %r          result (PASS or FAIL)\n"
      "\n"
      "Interactive editor commands:\n"
      "    Enter       next field / save\n"
      "    Tab         next field\n"
      "    Sh-Tab      previous field\n"
      "    Ctrl-c      cancel\n"
      "    Ctrl-d      detect / perform identification\n"
      "    Ctrl-x      save\n"
      "\n"
      "Interactive editor text input commands:\n"
      "    Ctrl-a      move cursor to start of line\n"
      "    Ctrl-e      move cursor to end of line\n"
      "    Ctrl-k      delete from cursor to end of line\n"
      "    Ctrl-u      delete from cursor to start of line\n"
      "\n"
      "Report bugs at https://github.com/d99kris/idntag\n"
      "\n";
  }
}

void ShowVersion()
{
  std::cout <<
    Version::GetAppName(true /*p_WithVersion*/) << "\n"
    "\n"
    "Copyright (c) 2018-2025 Kristofer Berggren\n"
    "\n"
    "idntag is distributed under the MIT license.\n"
    "\n"
    "Written by Kristofer Berggren.\n";
}
