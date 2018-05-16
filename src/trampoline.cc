/* trampoline.cc


   Copyright (C) 2000  Mathias Broxvall

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

#include "trampoline.h"

#include "game.h"
#include "map.h"

Trampoline::Trampoline(int x, int y) : GameHook(Role_GameHook), cx(x), cy(y) {}
void Trampoline::tick(Real t) {
  GameHook::tick(t);

  Map* map = Game::current->map;
  Cell& c = map->cell(cx, cy);
  Real dh = std::min(0.5 * t, c.sunken - 1e-5);
  for (int i = 0; i < 5; i++) c.heights[i] += dh;
  c.sunken -= dh;
  if (abs(c.sunken) < 2e-5 && abs(dh) < 1e-10) {
    remove();
    // Reset cell to no-trampoline-active state
    for (int i = 0; i < 5; i++) c.heights[i] += c.sunken;
    c.sunken = 0.;
  }
  map->markCellsUpdated(cx, cy, cx, cy, true);
}
