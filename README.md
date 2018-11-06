Idntag
======

| **Linux** |
|-----------|
| [![Build status](https://travis-ci.org/d99kris/idntag.svg?branch=master)](https://travis-ci.org/d99kris/idntag) |

Idntag is a command-line tool that identifies artist and song name in specified audio files and
updates their ID3-tag meta-data with correct data, and renames the files on format
Artist_Name-Track_Name.

Example Usage
=============

    $ idntag ./tests/song.mp3
    ./tests/song.mp3: OK
    $ ls tests/
    Broke_For_Free-Night_Owl.mp3
    $ ffprobe tests/Broke_For_Free-Night_Owl.mp3 2>&1 | grep -e artist -e title
    artist          : Broke For Free
    title           : Night Owl

Supported Platforms
===================
Idntag is primarily developed and tested on Linux.

Installation
============
Pre-requisites (Ubuntu):

    sudo apt install python3-pip
    pip3 install pyacoustid
    pip3 install pytaglib

Download the source code:

    git clone https://github.com/d99kris/idntag && cd idntag

Generate Makefile and build:

    mkdir -p build && cd build && cmake .. && make -s

Optionally install in system:

    sudo make install

Usage
=====

General usage syntax:

    idntag [-h] [-k] [-v] path [path ...]

Options:

    path            path of a file or directory
    -h, --help      show this help message and exit
    -k, --keepname  keep original filename
    -v, --version   show program's version number and exit

License
=======
Idntag is distributed under the MIT license. See LICENSE file.

Keywords
========
linux, fingerprint, music, mp3, automatically tag.
