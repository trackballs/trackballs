/** \file animated.cc
	The base class for all Animated (ie. drawable) objects.
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
#include "animated.h"
#include "game.h"
#include "map.h"

using namespace std;

Animated::Animated() :GameHook() {
  Game::current->add(this);
  zero(position);
  for(int i=0;i<3;i++) {
    specularColor[i] = 0.0;
    primaryColor[i] = 0.8;
    secondaryColor[i] = 0.3;
  }
  specularColor[3]=primaryColor[3]=secondaryColor[3]=1.0;
  texture=0;
  // Don't call any inherited version of the computeBoundBox yet to avoid problems
  // with uninitialized variables.
  Animated::computeBoundingBox();
}
Animated::~Animated() {
}
void Animated::has_moved() {
  position[2] = Game::current->map->getHeight(position[0],position[2]);
}
void Animated::onRemove() {
  GameHook::onRemove();
  Game::current->remove(this);
}
void Animated::draw() {}
void Animated::draw2() {}
void Animated::computeBoundingBox() {
  /* Use a default size 2x2x2 boundingbox around object */
  boundingBox[0][0]=-1.0;
  boundingBox[0][1]=-1.0;
  boundingBox[0][2]=-1.0;
  boundingBox[1][0]=+1.0;
  boundingBox[1][1]=+1.0;
  boundingBox[1][2]=+1.0;
}
