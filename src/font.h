/* font.h
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

#ifndef FONT_H
#define FONT_H

#include "general.h"

class Font {
 public:
  static void init();

  /* functions with a global access (not object-dependant)
   * can be called anyway, after 'init'.
   */
  static void tick(Real t);

  /* Return value is always the width of the rendered text */
  static int drawSimpleText(const char *text, int x, int y, float sz, float r, float g,
                            float b, float a);
  static int drawRightSimpleText(const char *text, int x, int y, float sz, float r, float g,
                                 float b, float a);
  static int drawCenterSimpleText(const char *text, int x, int y, float sz, float r, float g,
                                  float b, float a);
  static int getTextWidth(const char *text, float sz);
};

#endif
