/* gameMode.cc
   Generic implementation of the current gamemode.

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
#include "gameMode.h"

using namespace std;

GameMode *GameMode::current;

GameMode::GameMode() { keyUpReceived=1; }
GameMode::~GameMode() {
}
void GameMode::activate(GameMode* gm) { 
  if(current) current->deactivated();
  current = gm; 
  if(gm) gm->activated();
}
void GameMode::display(){}
void GameMode::key(int){}
void GameMode::keyUp(int){ keyUpReceived=1; }
void GameMode::special(int key,int x,int y){}
void GameMode::idle(Real td){}
void GameMode::mouse(int state,int x,int y){}
void GameMode::mouseDown(int button,int x,int y){}
void GameMode::activated(){}
void GameMode::deactivated(){}
void GameMode::doExpensiveComputations() {}
