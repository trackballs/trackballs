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

#ifndef BALL_H
#include "ball.h"
#endif

class Player : public Ball {
 public:
  Player(class Gamer *gamer);
  virtual ~Player();
  
  void draw();
  void tick(Real time);
  void key(int k);
  void die(int);
  void newLevel();
  void setStartVariables();        /**< Setup all default variables when starting a level */
  void restart(Coord3d);      /**< Restart the ball at given position */
  void mouse(int state,int x,int y);
  void setHealth(Real);
  virtual Boolean crash(Real speed);

  double health;
  double oxygen;
  double moveBurst;
  int lives,timeLeft,score,hasWon;
  Boolean playing;
 private:
  void jump();

  int lastJoyX, lastJoyY;
};

#endif
