#!/usr/bin/env bash

# make.sh
#
# Copyright (C) 2020-2025 Kristofer Berggren
# All rights reserved.
#
# See LICENSE for redistribution information.

# helpers
exiterr()
{
  >&2 echo "${1}"
  exit 1
}

show_usage()
{
  echo "usage: make.sh [OPTIONS] ACTION"
  echo ""
  echo "Options:"
  echo "  --yes,-y        - non-interactive mode, assume yes"
  echo ""
  echo "Actions:"
  echo "  deps            - perform dependency installation"
  echo "  build           - perform build"
  echo "  debug           - perform debug build"
  echo "  tests           - perform build and run tests"
  echo "  doc             - perform build and generate doc"
  echo "  install         - perform build and install"
  echo "  all             - perform deps, build, tests, doc and install"
  echo "  src             - perform source code reformatting"
  echo "  bump            - perform version bump"
  echo ""
}

# arguments
DEPS="0"
BUILD="0"
DEBUG="0"
TESTS="0"
DOC="0"
INSTALL="0"
SRC="0"
BUMP="0"
YES=""

if [[ "${#}" == "0" ]]; then
  show_usage
  exit 1
fi

while [[ ${#} -gt 0 ]]; do
  case "${1%/}" in
    deps)
      DEPS="1"
      ;;

    build)
      BUILD="1"
      ;;

    debug)
      DEBUG="1"
      ;;

    test*)
      BUILD="1"
      TESTS="1"
      ;;

    doc)
      BUILD="1"
      DOC="1"
      ;;

    install)
      BUILD="1"
      INSTALL="1"
      ;;

    src)
      SRC="1"
      ;;

    bump)
      BUMP="1"
      ;;

    all)
      DEPS="1"
      BUILD="1"
      TESTS="1"
      DOC="1"
      INSTALL="1"
      ;;

    -y)
      YES="-y"
      ;;

    --yes)
      YES="-y"
      ;;

    *)
      show_usage
      exit 1
      ;;
  esac
  shift
done

# detect os / distro
OS="$(uname)"
if [ "${OS}" == "Linux" ]; then
  unset NAME
  eval $(grep "^NAME=" /etc/os-release 2> /dev/null)
  if [[ "${NAME}" != "" ]]; then
    DISTRO="${NAME}"
  else
    if [[ "${TERMUX_VERSION}" != "" ]]; then
      DISTRO="Termux"
    fi
  fi
fi

# deps
if [[ "${DEPS}" == "1" ]]; then
  if [ "${OS}" == "Linux" ]; then
    if [[ "${DISTRO}" == "Ubuntu" ]]; then
      sudo apt update && sudo apt ${YES} install build-essential cmake pkg-config help2man libncurses-dev libncursesw5-dev libtag1-dev libcurl4-openssl-dev nlohmann-json3-dev libchromaprint-tools mp3info || exiterr "deps failed (${DISTRO}), exiting."
    else
      exiterr "deps failed (unsupported linux distro ${DISTRO}), exiting."
    fi
  elif [ "${OS}" == "Darwin" ]; then
    if command -v brew &> /dev/null; then
      HOMEBREW_NO_INSTALL_UPGRADE=1 HOMEBREW_NO_AUTO_UPDATE=1 brew install cmake pkg-config help2man ncurses libtag curl nlohmann-json chromaprint mp3info gsed || exiterr "deps failed (${OS} brew), exiting."
    elif command -v port &> /dev/null; then
      sudo port -N install cmake help2man ncurses libtag curl nlohmann-json chromaprint mp3info gsed || exiterr "deps failed (${OS} port), exiting."
    else
      exiterr "deps failed (${OS} missing brew and port), exiting."
    fi
  else
    exiterr "deps failed (unsupported os ${OS}), exiting."
  fi
fi

# src
if [[ "${SRC}" == "1" ]]; then
  uncrustify --update-config-with-doc -c etc/uncrustify.cfg -o etc/uncrustify.cfg && \
  uncrustify -c etc/uncrustify.cfg --replace --no-backup src/*.{cpp,h} || exiterr "unrustify failed, exiting."
fi

# bump
if [[ "${BUMP}" == "1" ]]; then
  CURRENT_VERSION=$(grep APP_VERSION src/version.cpp | head -1 | awk -F'"' '{print $2}') # ex: 1.14
  NEW_MAJ="$(echo ${CURRENT_VERSION} | cut -d'.' -f1)" # ex: 1
  let NEW_MIN=$(echo ${CURRENT_VERSION} | cut -d'.' -f2)+1 # ex: 15
  NEW_VERSION="${NEW_MAJ}.$(printf "%02d" ${NEW_MIN})" # ex: 1.15
  echo "Current:      ${CURRENT_VERSION}"
  echo "Bumped:       ${NEW_VERSION}"
  SED="sed"
  if [[ "$(uname)" == "Darwin" ]]; then
    SED="gsed"
  fi
  ${SED} -i "s/^#define APP_VERSION .*/#define APP_VERSION \"${NEW_VERSION}\"/g" src/version.cpp
fi

# make args
if [[ "${BUILD}" == "1" ]] || [[ "${DEBUG}" == "1" ]]; then
  if [[ "${OS}" == "Darwin" ]]; then
    CPU_MAX_THREADS="$(sysctl -n hw.ncpu)"
  else
    CPU_MAX_THREADS="$(nproc)"
  fi
  MAKEARGS="-j${CPU_MAX_THREADS}"
fi

# build
if [[ "${BUILD}" == "1" ]]; then
  echo "-- Using cmake ${CMAKEARGS}"
  echo "-- Using ${MAKEARGS} (${CPU_MAX_THREADS} cores)"
  mkdir -p build && cd build && cmake ${CMAKEARGS} .. && make -s ${MAKEARGS} && cd .. || exiterr "build failed, exiting."
fi

# debug
if [[ "${DEBUG}" == "1" ]]; then
  CMAKEARGS="-DCMAKE_BUILD_TYPE=Debug ${CMAKEARGS}"
  echo "-- Using cmake ${CMAKEARGS}"
  echo "-- Using ${MAKEARGS} (${CPU_MAX_THREADS} cores)"
  mkdir -p dbgbuild && cd dbgbuild && cmake ${CMAKEARGS} .. && make -s ${MAKEARGS} && cd .. || exiterr "debug build failed, exiting."
fi

# tests
if [[ "${TESTS}" == "1" ]]; then
  cd build && ctest --output-on-failure && cd .. || exiterr "tests failed, exiting."
fi

# doc
if [[ "${DOC}" == "1" ]]; then
  if [[ -x "$(command -v help2man)" ]]; then
    if [[ "$(uname)" == "Darwin" ]]; then
      SED="gsed -i"
    else
      SED="sed -i"
    fi
    APPNAME="idntag"
    APPDESC="identify and tag audio files"
    help2man -n "${APPDESC}" -N -o src/${APPNAME}.1 ./build/${APPNAME} && \
      ${SED} "s/\.\\\\\" DO NOT MODIFY THIS FILE\!  It was generated by help2man.*/\.\\\\\" DO NOT MODIFY THIS FILE\!  It was generated by help2man./g" src/${APPNAME}.1 || \
      exiterr "doc failed, exiting."
  fi
fi

# install
if [[ "${INSTALL}" == "1" ]]; then
  if [[ -z ${INSTALL_CMD+x} ]]; then
    if [[ "${OS}" == "Linux" ]]; then
      if [[ "${DISTRO}" != "Termux" ]]; then
        INSTALL_CMD="$(basename $(which sudo doas | head -1))"
      fi
    elif [[ "${OS}" == "Darwin" ]]; then
      if [[ "${GITHUB_ACTIONS}" == "true" ]]; then
        INSTALL_CMD="sudo"
      fi
    fi
  fi

  echo "-- Using ${INSTALL_CMD:+$INSTALL_CMD }make install"
  cd build && ${INSTALL_CMD} make install && cd .. || exiterr "install failed (${OS}), exiting."
fi

# exit
exit 0
