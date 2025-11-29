// editor.h
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#pragma once

#include <string>
#include <vector>

class Editor
{
private:
  struct Field
  {
    std::string label;
    std::string buf;
    int cursor = 0;
    int y = 0;
    int x = 0;
    int w = 0;
  };

  struct Layout
  {
    int field_w;
    int start_y;
    int start_x;
  };

public:
  static bool Edit(const std::string& p_FilePath, std::string& p_Artist,
                   std::string& p_Title);

private:
  static void DrawStaticFrame(int p_Rows, int p_Cols, Field& p_Artist, Field& p_Title);
  static void DrawFieldLine(const Field& p_Field, bool p_IsActive);
  static void PlaceCursor(const Field& p_Field, bool p_IsActive);
  static Layout ComputeLayout(int p_Rows, int p_Cols);
};
