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

#include "general.h"
#include "gameHook.h"
#include "game.h"

using namespace std;

int GameHook::nextId = 0;
std::set<GameHook *> *GameHook::deadObjects;

void GameHook::init() { deadObjects = new set<GameHook *>(); }
GameHook::GameHook() {
  alive = 1;
  id = nextId++;
  is_on = 1;

  if (Game::current)
    Game::current->add(this);
  else
    throw "GameHook::GameHook() - no game loaded\n";
}

GameHook::~GameHook() {}

void GameHook::remove() {
  if (alive) {
    deadObjects->insert(this);
    alive = 0;
    this->onRemove();
  }
}
void GameHook::onRemove() { Game::current->remove(this); }
void GameHook::deleteDeadObjects() {
  set<GameHook *>::iterator iter = deadObjects->begin();
  set<GameHook *>::iterator end = deadObjects->end();
  for (; iter != end; iter++) {
    GameHook *a = *iter;
    delete a;
  }
  deadObjects->clear();
}
void GameHook::playerRestarted() {}
void GameHook::doExpensiveComputations() {}
