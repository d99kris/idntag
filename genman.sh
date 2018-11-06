#!/bin/bash

help2man -n "automatically identify, tag and rename audio files" -N -o idntag.1 ./idntag
exit ${?}
