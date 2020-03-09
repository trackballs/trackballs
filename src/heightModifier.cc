/* heightModifier.cc
   allow to modify the height of a cell's corner

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

#include "heightModifier.h"

#include "game.h"
#include "map.h"

HeightModifier::HeightModifier(Game& g, int corner, int x, int y, Real min, Real max,
                               Real freq, Real phase, int not1, int not2, int not3)
    : GameHook(g, Role_GameHook) {
  this->x = x;
  this->y = y;
  this->min = min;
  this->max = max;
  this->freq = freq;
  this->phase = phase;
  if (corner >= 10) {
    this->avg_center = 1;
    corner -= 10;
  } else
    this->avg_center = 0;
  if ((corner < 0) || (corner > 3)) {
    warning("Warning: erroneous corner ID (%d) of HeightModifier (%d,%d). Using '0' instead.",
            corner, x, y);
    corner = 0;
  }
  this->corner = corner;
  this->not1 = not1;
  this->not2 = not2;
  this->not3 = not3;
}

void HeightModifier::tick(Real t) {
  GameHook::tick(t);

  Cell& c = game.map->cell(x, y);
  int x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
  int cor1 = 0, cor2 = 0, cor3 = 0;

  float v =
      min + (max - min) * (1. + std::cos(phase + (game.gameTime * freq) * 2. * M_PI)) / 2.;

  if (corner == 4) {
    c.heights[4] = v;
    game.map->markCellsUpdated(x, y, x, y, true);
    return;
  }

  int xlow, ylow;
  switch (corner) {
  case 0:
    xlow = x - 1;
    ylow = y - 1;
    x1 = x - 1;
    y1 = y;
    x2 = x;
    y2 = y - 1;
    x3 = x - 1;
    y3 = y - 1;
    cor1 = 2;
    cor2 = 1;
    cor3 = 3;
    break;
  case 1:
    xlow = x - 1;
    ylow = y;
    x1 = x;
    y1 = y + 1;
    x2 = x - 1;
    y2 = y;
    x3 = x - 1;
    y3 = y + 1;
    cor1 = 0;
    cor2 = 3;
    cor3 = 2;
    break;
  case 2:
    xlow = x;
    ylow = y - 1;
    x1 = x + 1;
    y1 = y;
    x2 = x;
    y2 = y - 1;
    x3 = x + 1;
    y3 = y - 1;
    cor1 = 0;
    cor2 = 3;
    cor3 = 1;
    break;
  case 3:
    xlow = x;
    ylow = y;
    x1 = x + 1;
    y1 = y;
    x2 = x;
    y2 = y + 1;
    x3 = x + 1;
    y3 = y + 1;
    cor1 = 1;
    cor2 = 2;
    cor3 = 0;
    break;
  }

  Cell& c1 = game.map->cell(x1, y1);
  Cell& c2 = game.map->cell(x2, y2);
  Cell& c3 = game.map->cell(x3, y3);

  c.heights[corner] = v;
  if ((cor1 != not1) && (cor1 != not2) && (cor1 != not3)) c1.heights[cor1] = v;
  if ((cor2 != not1) && (cor2 != not2) && (cor2 != not3)) c2.heights[cor2] = v;
  if ((cor3 != not1) && (cor3 != not2) && (cor3 != not3)) c3.heights[cor3] = v;

  if (avg_center) {
    c.heights[4] = (c.heights[0] + c.heights[1] + c.heights[2] + c.heights[3]) / 4.;
    if ((cor1 != not1) && (cor1 != not2) && (cor1 != not3))
      c1.heights[4] = (c1.heights[0] + c1.heights[1] + c1.heights[2] + c1.heights[3]) / 4.;
    if ((cor2 != not1) && (cor2 != not2) && (cor2 != not3))
      c2.heights[4] = (c2.heights[0] + c2.heights[1] + c2.heights[2] + c2.heights[3]) / 4.;
    if ((cor3 != not1) && (cor3 != not2) && (cor3 != not3))
      c3.heights[4] = (c3.heights[0] + c3.heights[1] + c3.heights[2] + c3.heights[3]) / 4.;
  }

  game.map->markCellsUpdated(xlow, ylow, xlow + 1, ylow + 1, true);
}
