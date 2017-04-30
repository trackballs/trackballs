/* modPill.h
   A "pill" which gives the player mods when taken

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

#ifndef MODPILL_H
#define MODPILL_H

#ifndef BALL_H
#include "ball.h"
#endif

class ModPill : public Ball {
 public:
  /* resurrecting: 0  - Not resurrected
	              -1  - Resurrected when player is restarted
				  >0  - Time between resurrections */
  ModPill(Coord3d position, int kind, int length, int resurrecting);
  ~ModPill();
  void draw();
  void draw2();
  void tick(Real);
  void playerRestarted();
  void die(int);

 private:
  int kind,resurrecting,state,time;
  Real timeLeft;
  Real clock; // used for animation
};

#endif
