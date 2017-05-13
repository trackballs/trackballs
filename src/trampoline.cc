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

#include "general.h"
#include "map.h"
#include "game.h"
#include "gameHook.h"
#include "trampoline.h"

using namespace std;

Trampoline::Trampoline(int x, int y) : GameHook(), cx(x), cy(y) {}
void Trampoline::tick(Real t) {
  Cell& c = Game::current->map->cell(cx, cy);
  Real dh = min(0.5 * t, c.sunken - 1e-5);
  for (int i = 0; i < 5; i++) c.heights[i] += dh;
  c.sunken -= dh;
  Game::current->map->markCellUpdated(cx, cy);
}
