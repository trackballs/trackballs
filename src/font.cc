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

#include <string.h>
#include <math.h>
#include "general.h"
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "font.h"

using namespace std;

void Font::init() {}

// returns the effective width of the text
int Font::getTextWidth(const char *text, float sz) {
  if ((text == NULL) || (strlen(text) == 0)) return (0);

  int w, h;
  TTF_Font *active = menuFontForSize((int)sz);
  TTF_SetFontOutline(active, 2);
  TTF_SizeUTF8(active, text, &w, &h);
  TTF_SetFontOutline(active, 0);
  return w;
}

void Font::tick(Real t) { update2DStringCache(); }

int Font::drawSimpleText(const char *text, int x, int y, float sz, float r, float g, float b,
                         float a) {
  if ((text == NULL) || (strlen(text) == 0)) return (0);

  int size = (int)sz;
  TTF_Font *active = menuFontForSize(size);
  draw2DString(active, text, x, y, r, g, b, a, sz > 10);

  return 0;
}

int Font::drawRightSimpleText(const char *text, int x, int y, float sz, float r, float g,
                              float b, float a) {
  return drawSimpleText(text, x - getTextWidth(text, sz), y, sz, r, g, b, a);
}

int Font::drawCenterSimpleText(const char *text, int x, int y, float sz, float r, float g,
                               float b, float a) {
  return drawSimpleText(text, x - getTextWidth(text, sz) / 2, y, sz, r, g, b, a);
}
