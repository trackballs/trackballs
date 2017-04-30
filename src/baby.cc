/** \file baby.cc
   Small "baby" Mr Black, ie balls that can be killed by the player by rolling over them scoring a few points.
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
#include "ball.h"
#include "black.h"
#include "baby.h"
#include "game.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"
#include "map.h"

using namespace std;


Baby::Baby(double x,double y) :Black(x,y) {
  acceleration = 1.0;
  ballResolution=BALL_LORES;
  realRadius = 0.1; radius=realRadius;
  crashTolerance = 3;
  horizon=3.0;
  position[2] = Game::current->map->getHeight(position[0],position[1]) + radius;
  modTimeLeft[MOD_DIZZY] = -1.0;
  likesPlayer=-1;

  setReflectivity(0.0,1);
}

void Baby::die() {
  if(alive) {
    new ScoreSign(50,position);
    playEffect(SFX_BLACK_DIE);
    remove();
  }
}

Baby::~Baby() { }

Boolean Baby::crash(Real speed,Ball *ball) {
  if(ball == (Ball*) Game::current->player1) {
	die();
	return false;
  }
  return Ball::crash(speed);
}
