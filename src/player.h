/* player.h
   Represent The(/A?) player.

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

#ifndef PLAYER_H
#define PLAYER_H

#include "ball.h"

class Player : public Ball {
 public:
  Player();
  virtual ~Player();

  void tick(Real time);
  void die(int);
  void newLevel();
  void setStartVariables();     /**< Setup all default variables when starting a level */
  void restart(const Coord3d&); /**< Restart the ball at given position */
  void handleUserInput();       /**< Called by main loop when there is new data */
  void mouse(int state, int x, int y);

  void setHealth(Real);
  virtual bool crash(Real speed);

  double health;
  double oxygen;
  double moveBurst;
  int lives, timeLeft, score, hasWon;
  bool playing;

  void requestJump() { controlJump = true; }

 private:
  double controldx, controldy;
  bool controlPrecise, controlJump;
};

#endif
