/* gameHook.cc
   gameHook's lives on a map and provides means to take special actions every timeframe.
   They can be used to implement elevators (modifying the map characteristics),
   targetZones (level finished when player enters them), etc...

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

#include "gameHook.h"
#include "animated.h"
#include "game.h"
#include "guile.h"

GameHook::GameHook(int role) {
  invalid = false;
  is_on = true;
  entity_role = role;

  for (int i = 0; i < GameHookEvent_MaxHooks; i++) hooks[i] = NULL;
}

GameHook::~GameHook() { GameHook::releaseCallbacks(); }

void GameHook::remove() { invalid = true; }

void GameHook::doExpensiveComputations() {}

void GameHook::registerHook(GameHookEvent event, SCM hook) {
  if (event < 0 || event >= GameHookEvent_MaxHooks) return;

  /* Remove any previous hook */
  if (hooks[event]) scm_gc_unprotect_object(hooks[event]);
  /* Save this hook */
  if (hook) scm_gc_protect_object(hook);
  hooks[event] = hook;
}
void GameHook::triggerHook(GameHookEvent event, SCM object) {
  if (event < 0 || event >= GameHookEvent_MaxHooks) return;

  if (hooks[event]) {
    SCM subject;
    Animated *a = dynamic_cast<Animated *>(this);
    if (a)
      subject = smobAnimated_make(a);
    else
      subject = smobGameHook_make(this);
    Game::current->queueCall(hooks[event], subject, object ? object : SCM_BOOL_F);
  }
}

SCM GameHook::getHook(GameHookEvent event) {
  if (event < 0 || event >= GameHookEvent_MaxHooks) return NULL;
  return hooks[event];
}

void GameHook::releaseCallbacks() {
  for (int event = 0; event < GameHookEvent_MaxHooks; event++) {
    if (hooks[event]) scm_gc_unprotect_object(hooks[event]);
    hooks[event] = NULL;
  }
}

void GameHook::tick(Real /*dt*/) { triggerHook(GameHookEvent_Tick, NULL); }
