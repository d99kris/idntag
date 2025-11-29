Idntag - Identify and tag audio files
=====================================

| **Linux** | **Mac** |
|-----------|---------|
| [![Linux](https://github.com/d99kris/idntag/workflows/Linux/badge.svg)](https://github.com/d99kris/idntag/actions?query=workflow%3ALinux) | [![macOS](https://github.com/d99kris/idntag/workflows/macOS/badge.svg)](https://github.com/d99kris/idntag/actions?query=workflow%3AmacOS) |

Idntag is a command-line tool that identifies artist and title of
specified audio files and updates their ID3-tag meta-data with correct data,
and renames the files on format Artist_Name-Track_Name.

**Warning:** This tool modifies and renames its input files. The quality of song
identification is not perfect and may have some false detections. It is
therefore recommended to first make a copy of the files to be identified, so
there is a backup in case the results are not good.

Usage
=====
Usage:

    idntag [OPTIONS] PATHS...

Command-line options:

    -c, --clear            clear tags
    -d, --detect           detect / identify audio
    -e, --edit             edit / confirm detected tags
    -r, --rename           rename file based on tags

    -R, --report           specify report format
    -h, --help             display help
    -v, --verbose          enable verbose debug output
    -V, --version          display version information
    PATHS                  files or directories to process

Output format fields:

    %i          input file name
    %o          output file name
    %r          result (PASS or FAIL)

Interactive editor commands:

    Enter       next field / save
    Tab         next field
    Sh-Tab      previous field
    Ctrl-c      cancel
    Ctrl-d      detect / perform identification
    Ctrl-x      save

Interactive editor text input commands:

    Ctrl-a      move cursor to start of line
    Ctrl-e      move cursor to end of line
    Ctrl-k      delete from cursor to end of line
    Ctrl-u      delete from cursor to start of line

Example Usage
=============

    $ idntag -d -r tests/song_en.mp3
    tests/song_en.mp3 : PASS : tests/Broke_For_Free-Night_Owl.mp3
    $ ls tests/Broke*
    Broke_For_Free-Night_Owl.mp3
    $ ffprobe tests/Broke_For_Free-Night_Owl.mp3 2>&1 | grep -e artist -e title
    artist          : Broke For Free
    title           : Night Owl

Supported Platforms
===================
Idntag is developed and tested on Linux and macOS. Current version has been
tested on:

- macOS Sequoia 15.6
- Ubuntu 24.04 LTS

Build from Source
=================
**Get Source**

    git clone https://github.com/d99kris/idntag && cd idntag

Using make.sh script
--------------------
If using macOS or Ubuntu, one can use the `make.sh` script provided.

**Dependencies**

    ./make.sh deps

**Build / Install**

    ./make.sh build && ./make.sh install

Manually
--------
**Dependencies**

macOS Brew

    brew install cmake pkg-config help2man ncurses libtag curl nlohmann-json chromaprint mp3info gsed

macOS Ports

    sudo port install cmake help2man ncurses libtag curl nlohmann-json chromaprint mp3info gsed

Ubuntu

    sudo apt install build-essential cmake pkg-config help2man libncurses-dev libncursesw5-dev libtag1-dev libcurl4-openssl-dev nlohmann-json3-dev libchromaprint-tools mp3info

**Build**

    mkdir -p build && cd build && cmake .. && make -s

**Install**

    sudo make install

Install using Package Manager
=============================
Disclaimer: The following packages are not maintained nor reviewed by the
author of `idntag`.

**Fedora**

[Idntag](https://www.nosuchhost.net/~cheese/fedora/packages/36/x86_64/idntag.html)

License
=======
Idntag is distributed under the MIT license. See LICENSE file.

Keywords
========
linux, macos, fingerprint, music, mp3, automatically tag, acoustid.
