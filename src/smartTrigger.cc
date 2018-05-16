/* smartTrigger.cc
   triggers a scheme expression when player is within radius

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

#include "smartTrigger.h"

#include "game.h"
#include "guile.h"
#include "player.h"

SmartTrigger::SmartTrigger(Real x, Real y, Real radius, SCM entering, SCM leaving)
    : GameHook(Role_GameHook),
      x(x),
      y(y),
      radius(radius),
      wasIn(false),
      entering(entering),
      leaving(leaving) {
  if (entering) scm_gc_protect_object(entering);
  if (leaving) scm_gc_protect_object(leaving);
}

SmartTrigger::~SmartTrigger() { SmartTrigger::releaseCallbacks(); }
void SmartTrigger::releaseCallbacks() {
  if (entering) scm_gc_unprotect_object(entering);
  if (leaving) scm_gc_unprotect_object(leaving);
  entering = NULL;
  leaving = NULL;
}
void SmartTrigger::tick(Real t) {
  GameHook::tick(t);

  if (!is_on) return;
  Player *ply = Game::current->player1;
  double dx = ply->position[0] - x;
  double dy = ply->position[1] - y;
  if (dx * dx + dy * dy < radius * radius) {
    if (!wasIn && entering) Game::current->queueCall(entering);
    wasIn = true;
  } else {
    if (wasIn && leaving) Game::current->queueCall(leaving);
    wasIn = false;
  }
}
