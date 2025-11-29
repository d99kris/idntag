// log.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "log.h"

#include <cstdarg>
#include <cstdio>

bool Log::m_Verbose = false;

void Log::Debug(const char* p_Format, ...)
{
  if (m_Verbose)
  {
    va_list vaList;
    va_start(vaList, p_Format);
    vprintf(p_Format, vaList);
    va_end(vaList);
    printf("\n");
  }
}

void Log::SetVerbose(bool p_Verbose)
{
  m_Verbose = p_Verbose;
}
