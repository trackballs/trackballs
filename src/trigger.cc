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

#include "trigger.h"

#include "game.h"
#include "guile.h"
#include "player.h"

Trigger::Trigger(Real x, Real y, Real radius, SCM expr)
    : GameHook(Role_GameHook), x(x), y(y), radius(radius), expr(expr) {
  if (expr) scm_gc_protect_object(expr);
}
Trigger::~Trigger() { Trigger::releaseCallbacks(); }
void Trigger::releaseCallbacks() {
  if (expr) scm_gc_unprotect_object(expr);
  expr = NULL;
}
void Trigger::tick(Real /*t*/) {
  if (!is_on) return;
  Player *ply = Game::current->player1;
  double dx = ply->position[0] - x;
  double dy = ply->position[1] - y;
  if (dx * dx + dy * dy < radius * radius) Game::current->queueCall(expr);
}
