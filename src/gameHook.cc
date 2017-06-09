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
#include <set>
#include "animated.h"
#include "game.h"
#include "guile.h"

int GameHook::nextId = 0;

static std::set<GameHook *> *deadObjects;

void GameHook::init() { deadObjects = new std::set<GameHook *>(); }
GameHook::GameHook() {
  int i;

  alive = 1;
  id = nextId++;
  is_on = 1;

  if (Game::current)
    Game::current->add(this);
  else
    throw "GameHook::GameHook() - no game loaded\n";

  for (i = 0; i < GameHookEvent_MaxHooks; i++) hooks[i] = NULL;
}

GameHook::~GameHook() {}

void GameHook::remove() {
  if (alive) {
    deadObjects->insert(this);
    alive = 0;
    this->onRemove();
  }
}
void GameHook::onRemove() {
  int event;

  /* Remove all registered guile hooks */
  for (event = 0; event < GameHookEvent_MaxHooks; event++)
    if (hooks[event]) scm_gc_unprotect_object(hooks[event]);

  Game::current->remove(this);
}
void GameHook::deleteDeadObjects() {
  std::set<GameHook *>::iterator iter = deadObjects->begin();
  std::set<GameHook *>::iterator end = deadObjects->end();
  for (; iter != end; iter++) {
    GameHook *a = *iter;
    delete a;
  }
  deadObjects->clear();
}
void GameHook::playerRestarted() {}
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
    scm_catch_apply_2(hooks[event], subject, object ? object : SCM_BOOL_F);
  }
}

SCM GameHook::getHook(GameHookEvent event) {
  if (event < 0 || event >= GameHookEvent_MaxHooks) return NULL;
  return hooks[event];
}

void GameHook::tick(Real /*dt*/) { triggerHook(GameHookEvent_Tick, NULL); }
