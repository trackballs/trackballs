/* gameHook.h
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

#ifndef GAMEHOOK_H
#define GAMEHOOK_H

#include <libguile.h>
#include "general.h"

typedef enum {
  GameHookEvent_Death = 0,
  GameHookEvent_Spawn,
  GameHookEvent_Tick,
  GameHookEvent_MaxHooks
} GameHookEvent;

enum {
  Role_Dead = 0,      /* collects entities for cleanup */
  Role_GameHook,      /* pure GameHooks (will not be rendered) */
  Role_OtherAnimated, /* animated: misc types */
  Role_Ball,          /* animated: all types inheriting from Ball, except */
  Role_Player,        /* animated: ... player balls */
  Role_Pipe,          /* animated: pipes */
  Role_PipeConnector, /* animated: pipe connectors */
  Role_Forcefield,    /* animated: forcefields */
  Role_Flag,          /* animated: flags */
  Role_MaxTypes
};

class Game;

class GameHook {
 public:
  explicit GameHook(Game& game, int role);
  virtual ~GameHook();
  virtual void tick(Real);
  virtual void doExpensiveComputations();

  /** Trigger any guile hooks which have been registered for this
      event */
  void triggerHook(GameHookEvent event, SCM arg);

  /** Register a hook which can be triggered on various events */
  void registerHook(GameHookEvent event, SCM hook);

  /** Returns the hook currently registered to event, or NULL */
  SCM getHook(GameHookEvent event);

  /** Marks the object for removal from the world. */
  void remove();

  /** Unregisters all hooks */
  virtual void releaseCallbacks();

  Game& game; /* A pointer to the game to which the hook is attached */
  int entity_role;
  bool is_on;   /* for switches, forcefields, objects with toggle state */
  bool invalid; /* if true, object is slated for cleanup */

 private:
  SCM hooks[GameHookEvent_MaxHooks];
};

#endif
