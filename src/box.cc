/** \file box.cc
   Implements boxes containing goodies (points or other effects), currently not finished due to physics problems.
*/
/*
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
#include "animated.h"
#include "box.h"
#include "game.h"
#include "map.h"
#include "sign.h"
#include "scoreSign.h"

using namespace std;

Box::Box(Real ct, int pnt) :Animated(), crashTolerance(ct), points(pnt),size(0.4) {}
void Box::has_moved() {
  Map *map = Game::current->map;
  double h1 = max(map->getHeight(position[0]-size/2,position[1]-size/2),
				  map->getHeight(position[0]-size/2,position[1]+size/2));
  double h2 = max(map->getHeight(position[0]+size/2,position[1]-size/2),
				  map->getHeight(position[0]+size/2,position[1]+size/2));
  position[2] = max(h1,h2);  
}
void Box::crash(Real speed) {
  if(speed > crashTolerance) {
	new ScoreSign(points,position);
	this->remove();
	/* TODO: Debris */
  }
}
void Box::draw() {
  
}
