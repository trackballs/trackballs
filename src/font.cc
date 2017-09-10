/* font.cc
   Manage textured-font for drawing text

   Copyright (C) 2000  Mathias Broxvall
                       Yannick Perret

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "font.h"
#include "glHelp.h"

#include <SDL2/SDL_ttf.h>

void Font::init() {}

// returns the effective width of the text
int Font::getTextWidth(const char *text, int size, int maxwidth) {
  if ((text == NULL) || (strlen(text) == 0)) return (0);

  int w, h;
  TTF_Font *active = menuFontForSize(size);
  TTF_SetFontOutline(active, 2);
  TTF_SizeUTF8(active, text, &w, &h);
  TTF_SetFontOutline(active, 0);
  if (maxwidth > 0 && maxwidth < w) return maxwidth;
  return w;
}

void Font::tick(Real /*t*/) { update2DStringCache(); }

int Font::drawSimpleText(const char *text, int x, int y, int sz, float r, float g, float b,
                         float a, int maxwidth) {
  if ((text == NULL) || (strlen(text) == 0)) return 0;

  int size = (int)sz;
  TTF_Font *active = menuFontForSize(size);
  return draw2DString(active, text, x, y, r, g, b, a, sz > 10, 0, maxwidth);
}

int Font::drawRightSimpleText(const char *text, int x, int y, int sz, float r, float g,
                              float b, float a, int maxwidth) {
  if ((text == NULL) || (strlen(text) == 0)) return 0;

  int size = (int)sz;
  TTF_Font *active = menuFontForSize(size);
  return draw2DString(active, text, x, y, r, g, b, a, sz > 10, 2, maxwidth);
}

int Font::drawCenterSimpleText(const char *text, int x, int y, int sz, float r, float g,
                               float b, float a, int maxwidth) {
  if ((text == NULL) || (strlen(text) == 0)) return 0;

  int size = (int)sz;
  TTF_Font *active = menuFontForSize(size);
  return draw2DString(active, text, x, y, r, g, b, a, sz > 10, 1, maxwidth);
}
