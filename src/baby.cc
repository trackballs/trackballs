/** \file baby.cc
   Small "baby" Mr Black, ie balls that can be killed by the player by rolling over them
   scoring a few points.
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

#include "baby.h"
#include "game.h"
#include "map.h"
#include "sound.h"

Baby::Baby(Game& g, Real x, Real y) : Black(g, x, y) {
  acceleration = 1.0;
  ballResolution = BALL_LORES;
  realRadius = 0.1;
  radius = realRadius;
  crashTolerance = 3;
  horizon = 3.0;
  position[2] = game.map->getHeight(position[0], position[1]) + radius;
  modTimeLeft[MOD_DIZZY] = -1.0;
  likesPlayer = -1;
  scoreOnDeath = Game::defaultScores[SCORE_BABY][0];
  timeOnDeath = Game::defaultScores[SCORE_BABY][1];

  setReflectivity(0.0, 1);
}

void Baby::die(int how) {
  Ball::die(how);
  playEffect(SFX_BLACK_DIE);
  remove();
}

Baby::~Baby() {}
