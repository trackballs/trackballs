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
#include "gameHook.h"
#include "trampoline.h"

using namespace std;

Trampoline::Trampoline(Cell& c) :GameHook(), cell(c) {}
void Trampoline::tick(Real t) {
  Real dh = min(0.5 * t,cell.sunken-1e-5);
  for(int i=0;i<5;i++)
	cell.heights[i] += dh;
  cell.sunken -= dh;
}
