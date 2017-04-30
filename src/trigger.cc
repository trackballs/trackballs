/* trigger.cc
   Triggers a scheme expression when player is within radius

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
#include "trigger.h"
#include "game.h"
#include "player.h"
#include "libguile.h"

using namespace std;

Trigger::Trigger(Real x,Real y,Real radius,SCM expr)
  :GameHook(), x(x), y(y), radius(radius), expr(expr) {
  scm_gc_protect_object(expr);
}
Trigger::~Trigger() {
  scm_gc_unprotect_object(expr);
  this->GameHook::~GameHook();
}
void Trigger::tick(Real t) {
  Player *ply = Game::current->player1;
  double dx = ply->position[0] - x;
  double dy = ply->position[1] - y;
  if(dx*dx+dy*dy < radius*radius) 
	scm_apply_0(expr,SCM_EOL);
}
