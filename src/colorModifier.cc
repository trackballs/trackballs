/* colorModifier.cc
   allow to modify the color of a cell

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

#include "colorModifier.h"
#include "game.h"
#include "map.h"

ColorModifier::ColorModifier(Game& g, int col, int x, int y, Real min, Real max, Real freq,
                             Real phase)
    : GameHook(g, Role_GameHook) {
  this->x = x;
  this->y = y;
  this->min = min;
  this->max = max;
  this->freq = freq;
  this->phase = phase;
  if ((col < 0) || (col > 3)) {
    warning("erroneous color ID (%d) of ColorModifier (%d,%d). Using '0' instead.", col, x, y);
    col = 0;
  }
  this->colors = col;
  is_on = true;
}

void ColorModifier::tick(Real t) {
  GameHook::tick(t);

  double tt = game.gameTime;
  Cell& c = game.map->cell(x, y);
  if (!is_on) return;
  float fv = min + (max - min) * (1. + std::cos(phase + (tt * freq) * 2. * M_PI)) / 2.;
  uint16_t v = std::max(0.f, std::min(1.f, fv)) * MAX_SRGB_VAL;
  switch (colors) {
  case 0:
    for (int i = 0; i < 5; i++) {
      c.colors[i].w[0] = v;
      c.colors[i].w[1] = v;
      c.colors[i].w[2] = v;
    }
    break;
  case 1:
    for (int i = 0; i < 5; i++) c.colors[i].w[0] = v;
    break;
  case 2:
    for (int i = 0; i < 5; i++) c.colors[i].w[1] = v;
    break;
  case 3:
    for (int i = 0; i < 5; i++) c.colors[i].w[2] = v;
    break;
  }
  game.map->markCellsUpdated(x, y, x, y, false);
}
