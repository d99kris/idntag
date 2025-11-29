// editor.cpp
//
// Copyright (c) 2025 Kristofer Berggren
// All rights reserved.
//
// idntag is distributed under the MIT license, see LICENSE for details.

#include "editor.h"

#include <clocale>
#include <cwchar>
#include <cwctype>
#include <vector>

#include <ncurses.h>

#include "acoustid.h"

#define KEY_CTRLA 1
#define KEY_CTRLC 3
#define KEY_CTRLD 4
#define KEY_CTRLE 5
#define KEY_CTRLK 11
#define KEY_CTRLU 21
#define KEY_CTRLX 24

// --- UTF-8 helper functions ----------------------------------------------
//
// We keep the field buffer as UTF-8 (std::string) but make the cursor and
// drawing logic aware of multi-byte and double-width characters, using
// wcwidth() to compute terminal cell width.

// Convert a single wide character to UTF-8
static std::string WCharToUtf8(wchar_t wc)
{
  std::string out;
  char buf[8];
  mbstate_t st{ };
  size_t len = wcrtomb(buf, wc, &st);
  if (len != static_cast<size_t>(-1) && len > 0)
  {
    out.assign(buf, buf + len);
  }
  return out;
}

// Return number of bytes for the UTF-8 character starting at pos.
static size_t Utf8CharLen(const std::string& s, size_t pos)
{
  if (pos >= s.size())
  {
    return 0;
  }

  unsigned char c = static_cast<unsigned char>(s[pos]);
  if (c < 0x80)
  {
    return 1;
  }
  else if ((c & 0xE0) == 0xC0)
  {
    return ((pos + 1) < s.size()) ? 2 : 1;
  }
  else if ((c & 0xF0) == 0xE0)
  {
    return ((pos + 2) < s.size()) ? 3 : 1;
  }
  else if ((c & 0xF8) == 0xF0)
  {
    return ((pos + 3) < s.size()) ? 4 : 1;
  }

  // Invalid leading byte, treat as single byte to avoid getting stuck
  return 1;
}

// Find the start index (byte offset) of the previous UTF-8 character.
static size_t Utf8PrevChar(const std::string& s, size_t pos)
{
  if (pos == 0 || pos > s.size())
  {
    return 0;
  }

  size_t i = pos;

  do
  {
    --i;
    unsigned char c = static_cast<unsigned char>(s[i]);
    // UTF-8 continuation bytes are 10xxxxxx; first byte is anything else.
    if ((c & 0xC0) != 0x80)
    {
      return i;
    }
  }
  while (i > 0);

  return 0;
}

// Compute wcwidth() for UTF-8 sequence at [pos, pos+len).
static int Utf8CharWidth(const std::string& s, size_t pos, size_t len)
{
  if (len == 0 || pos >= s.size())
  {
    return 0;
  }

  mbstate_t st{ };
  wchar_t wc = 0;
  int ret = static_cast<int>(mbrtowc(&wc, s.c_str() + pos, len, &st));
  if (ret <= 0)
  {
    // Fallback: treat as width 1 on decode failure
    return 1;
  }

  int w = wcwidth(wc);
  if (w < 0)
  {
    w = 1;
  }

  return w;
}

// Total display width (columns) of a full UTF-8 string.
static int Utf8DisplayWidth(const std::string& s)
{
  int width = 0;
  size_t pos = 0;
  while (pos < s.size())
  {
    size_t len = Utf8CharLen(s, pos);
    int w = Utf8CharWidth(s, pos, len);
    width += w;
    pos += len;
  }

  return width;
}

bool Editor::Edit(const std::string& p_FilePath, std::string& p_Artist, std::string& p_Title)
{
  (void)p_FilePath; // unused for now

  setlocale(LC_ALL, "");

  Field artist{ "Artist", p_Artist };
  Field title { "Title", p_Title };

  artist.cursor = static_cast<int>(artist.buf.size());
  title.cursor = static_cast<int>(title.buf.size());

  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);
  curs_set(1);

  int rows = 0;
  int cols = 0;
  getmaxyx(stdscr, rows, cols);

  // 0 = artist, 1 = title
  int active = 0;

  // Draw static frame once at startup
  DrawStaticFrame(rows, cols, artist, title);

  DrawFieldLine(artist, active == 0);
  DrawFieldLine(title, active == 1);
  PlaceCursor(active == 0 ? artist : title, true);

  bool done = false;
  while (!done)
  {
    bool need_redraw_fields = false;

    wint_t wch;
    int rc = get_wch(&wch);

    // Handle function keys (arrows, resize, F-keys, etc.)
    if (rc == KEY_CODE_YES)
    {
      int key = static_cast<int>(wch);
      Field& cur = (active == 0) ? artist : title;

      if (key == KEY_RESIZE)
      {
        // On resize: recompute layout, redraw static frame and fields once
        getmaxyx(stdscr, rows, cols);
        DrawStaticFrame(rows, cols, artist, title);
        DrawFieldLine(artist, active == 0);
        DrawFieldLine(title, active == 1);
        PlaceCursor(active == 0 ? artist : title, true);
        continue;
      }

      switch (key)
      {
        case KEY_LEFT:
          if (cur.cursor > 0)
          {
            size_t newPos = Utf8PrevChar(cur.buf,
                                         static_cast<size_t>(cur.cursor));
            cur.cursor = static_cast<int>(newPos);
            need_redraw_fields = true;
          }
          break;

        case KEY_RIGHT:
          if (cur.cursor < static_cast<int>(cur.buf.size()))
          {
            cur.cursor += static_cast<int>(
              Utf8CharLen(cur.buf, static_cast<size_t>(cur.cursor)));
            need_redraw_fields = true;
          }
          break;

        case KEY_UP:
        case KEY_BTAB:
          active = (active + 1) % 2;
          (active == 0 ? artist : title).cursor =
            (active == 0
             ? static_cast<int>(artist.buf.size())
             : static_cast<int>(title.buf.size()));
          need_redraw_fields = true;
          break;

        case KEY_DOWN:
        case '\t':
          active = (active + 1) % 2;
          (active == 0 ? artist : title).cursor =
            (active == 0
             ? static_cast<int>(artist.buf.size())
             : static_cast<int>(title.buf.size()));
          need_redraw_fields = true;
          break;

        case KEY_BACKSPACE:
          if (cur.cursor > 0)
          {
            size_t curPos = static_cast<size_t>(cur.cursor);
            size_t start = Utf8PrevChar(cur.buf, curPos);
            size_t len = curPos - start;

            cur.buf.erase(start, len);
            cur.cursor = static_cast<int>(start);
            need_redraw_fields = true;
          }
          break;

        case KEY_DC:
          if (cur.cursor < static_cast<int>(cur.buf.size()))
          {
            size_t curPos = static_cast<size_t>(cur.cursor);
            size_t len = Utf8CharLen(cur.buf, curPos);

            cur.buf.erase(curPos, len);
            need_redraw_fields = true;
          }
          break;

        case KEY_F(10):
          done = true;
          break;

        default:
          // Unhandled function key
          break;
      }
    }
    else  // rc == OK: regular character (including control chars & Unicode)
    {
      Field& cur = (active == 0) ? artist : title;

      switch (wch)
      {
        case KEY_CTRLC:
          // Abort on CTRL-C
          done = false;
          endwin();
          return false;

        case KEY_CTRLD:
          {
            // Detect / identify
            std::string idArtist;
            std::string idTitle;
            if (AcoustId::Identify(p_FilePath, idArtist, idTitle))
            {
              artist.buf = idArtist;
              title.buf = idTitle;
              artist.cursor = static_cast<int>(artist.buf.size());
              title.cursor = static_cast<int>(title.buf.size());
              active = 0;
            }
            need_redraw_fields = true;
            break;
          }

        case KEY_CTRLX:
          done = true;
          break;

        case KEY_CTRLA:
        case KEY_HOME:
          cur.cursor = 0;
          need_redraw_fields = true;
          break;

        case KEY_CTRLE:
        case KEY_END:
          cur.cursor = static_cast<int>(cur.buf.size());
          need_redraw_fields = true;
          break;

        case KEY_CTRLK:
          if (cur.cursor < static_cast<int>(cur.buf.size()))
          {
            size_t curPos = static_cast<size_t>(cur.cursor);
            cur.buf.erase(curPos);          // erase to end
            need_redraw_fields = true;
          }
          break;

        case KEY_CTRLU:
          if (cur.cursor > 0)
          {
            size_t curPos = static_cast<size_t>(cur.cursor);
            cur.buf.erase(0, curPos);       // erase from start
            cur.cursor = 0;
            need_redraw_fields = true;
          }
          break;

        case 127:   // DEL as Backspace
        case 8:     // Ctrl-H as Backspace (some terminals)
          if (cur.cursor > 0)
          {
            size_t curPos = static_cast<size_t>(cur.cursor);
            size_t start = Utf8PrevChar(cur.buf, curPos);
            size_t len = curPos - start;

            cur.buf.erase(start, len);
            cur.cursor = static_cast<int>(start);
            need_redraw_fields = true;
          }
          break;

        case L'\n':
        case L'\r':
          if (active == 1)
          {
            done = true;
          }
          else
          {
            active = 1;
            title.cursor = static_cast<int>(title.buf.size());
            need_redraw_fields = true;
          }
          break;

        case L'\t':
          active = (active + 1) % 2;
          (active == 0 ? artist : title).cursor =
            (active == 0
             ? static_cast<int>(artist.buf.size())
             : static_cast<int>(title.buf.size()));
          need_redraw_fields = true;
          break;

        default:
          // Text input: accept any printable wide character (Unicode)
          if (iswprint(static_cast<wint_t>(wch)))
          {
            int curWidth = Utf8DisplayWidth(cur.buf);

            int charWidth = wcwidth(static_cast<wchar_t>(wch));
            if (charWidth < 0) charWidth = 1;

            if (curWidth + charWidth <= cur.w)
            {
              std::string utf8 = WCharToUtf8(static_cast<wchar_t>(wch));
              size_t pos = static_cast<size_t>(cur.cursor);

              cur.buf.insert(pos, utf8);
              cur.cursor += static_cast<int>(utf8.size());

              need_redraw_fields = true;
            }
          }
          break;
      }
    }

    if (need_redraw_fields)
    {
      // Only redraw the two field regions (no border refresh)
      DrawFieldLine(artist, active == 0);
      DrawFieldLine(title, active == 1);
      PlaceCursor(active == 0 ? artist : title, true);
    }
  }

  endwin();

  if (done)
  {
    p_Artist = artist.buf;
    p_Title = title.buf;
  }

  return done;
}

void Editor::DrawStaticFrame(int p_Rows, int p_Cols, Field& p_Artist, Field& p_Title)
{
  clear();

  wborder(stdscr, 0, 0, 0, 0, 0, 0, 0, 0);

  Layout L = ComputeLayout(p_Rows, p_Cols);

  int space = (p_Rows > 8) ? 1 : 0;

  p_Artist.y = L.start_y + 1;
  p_Artist.x = L.start_x;
  p_Artist.w = L.field_w;

  p_Title.y = L.start_y + 3 + space;
  p_Title.x = L.start_x;
  p_Title.w = L.field_w;

  mvprintw(0, std::max(0, (p_Cols - 14)/2), " Edit ID3 tag ");
  mvprintw(L.start_y - 0, L.start_x - 0, "Artist:");
  mvprintw(L.start_y + 2 + space, L.start_x - 0, "Title:");

  refresh();
}

void Editor::DrawFieldLine(const Field& p_Field, bool p_IsActive)
{
  // Prepare content clipped/padded to visible width in *terminal cells*,
  // not bytes. This keeps double-width characters (CJK, etc.) aligned and
  // avoids splitting multi-byte sequences.
  const std::string& src = p_Field.buf;
  std::string content;
  content.reserve(static_cast<size_t>(p_Field.w) * 4);

  int width = 0;
  size_t pos = 0;
  while (pos < src.size())
  {
    size_t len = Utf8CharLen(src, pos);
    int w = Utf8CharWidth(src, pos, len);

    if (width + w > p_Field.w)
    {
      break;
    }

    content.append(src, pos, len);
    width += w;
    pos += len;
  }

  // Pad with spaces to fill the entire field width
  if (width < p_Field.w)
  {
    content.append(static_cast<size_t>(p_Field.w - width), ' ');
  }

  if (p_IsActive)
  {
    attron(A_REVERSE);
  }

  // mvaddnstr count parameter is in bytes for the narrow version.
  mvaddnstr(p_Field.y, p_Field.x, content.c_str(),
            static_cast<int>(content.size()));

  if (p_IsActive)
  {
    attroff(A_REVERSE);
  }
}

void Editor::PlaceCursor(const Field& p_Field, bool p_IsActive)
{
  if (!p_IsActive) return;

  // Compute display column of the cursor by walking UTF-8 characters
  // up to the byte offset p_Field.cursor and summing their wcwidth().
  int col = 0;
  size_t pos = 0;
  size_t cursorPos = static_cast<size_t>(
    std::min(p_Field.cursor, static_cast<int>(p_Field.buf.size())));

  while (pos < cursorPos)
  {
    size_t len = Utf8CharLen(p_Field.buf, pos);
    int w = Utf8CharWidth(p_Field.buf, pos, len);
    col += w;
    pos += len;
  }

  // Clamp to visible width
  if (col > p_Field.w)
  {
    col = p_Field.w;
  }

  int cx = p_Field.x + col;
  int cy = p_Field.y;
  move(cy, cx);
  refresh();
}

Editor::Layout Editor::ComputeLayout(int p_Rows, int p_Cols)
{
  Layout L;
  L.field_w = std::max(20, std::min(70, p_Cols - 5));
  L.start_y = std::max(1, p_Rows/2 - 4);
  L.start_x = (p_Cols - L.field_w) / 2;
  return L;
}
