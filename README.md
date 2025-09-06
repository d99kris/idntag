Idntag
======

| **Linux** |
|-----------|
| [![Linux](https://github.com/d99kris/idntag/workflows/Linux/badge.svg)](https://github.com/d99kris/idntag/actions?query=workflow%3ALinux) |

Idntag is a command-line tool that identifies artist and song name in
specified audio files and updates their ID3-tag meta-data with correct data,
and renames the files on format Artist_Name-Track_Name.

**Warning:** This tool modifies and renames its input files. The quality of song
identification is not perfect and may have some false detections. It is
therefore recommended to first make a copy of the files to be identified, so
there is a backup in case the results are not good.

Example Usage
=============

    $ idntag tests/song.mp3
    tests/song.mp3 : OK : tests/Broke_For_Free-Night_Owl.mp3
    $ ls tests/
    Broke_For_Free-Night_Owl.mp3
    $ ffprobe tests/Broke_For_Free-Night_Owl.mp3 2>&1 | grep -e artist -e title
    artist          : Broke For Free
    title           : Night Owl

Supported Platforms
===================
Idntag is developed and tested on Linux.

Installation
============
Pre-requisites Ubuntu:

    sudo apt install git cmake mp3info python3-pip libtag1-dev libchromaprint-dev libchromaprint-tools ubuntu-restricted-extras ffmpeg

Pre-requisites Debian (incl. Debian-based like Raspbian):

    sudo apt install git cmake mp3info python3-pip libtag1-dev libchromaprint-dev libchromaprint-tools ffmpeg

Download the source code:

    git clone https://github.com/d99kris/idntag && cd idntag

Generate Makefile and build:

    mkdir -p build && cd build && cmake .. && make -s

Optionally run tests:

    ctest --output-on-failure

Optionally install in system:

    sudo make install

Installation - Third-Party Packages
===================================
Disclaimer: The following packages are not maintained nor reviewed by the
author of `idntag`.

Fedora
------
[Idntag](https://www.nosuchhost.net/~cheese/fedora/packages/36/x86_64/idntag.html)

Usage
=====

General usage syntax:

    idntag [-k] [-l] path [path ...]
    idntag -h
    idntag -v

Options:

    path            path of a file or directory
    -h, --help      show this help message and exit
    -k, --keepname  keep original filename
    -l, --uselib    use chromaprint lib instead of tools
    -v, --version   show program's version number and exit

License
=======
Idntag is distributed under the MIT license. See LICENSE file.

Keywords
========
linux, fingerprint, music, mp3, automatically tag.
