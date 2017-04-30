/* debris.h
   Basic debri

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

#ifndef DEBRIS_H
#define DEBRIS_H

#ifndef BALL_H
#include "ball.h"
#endif

class Debris : public Ball {
 public:
  Debris(Ball*,Coord3d pos,Coord3d vel,double duration);
  void calcRadius();         // Use this after modifying the parameters below
  void tick(Real);
  void draw();
  void die(int);

  int floatingUp;
  double initialSize, endingSize, duration, elapsedTime;
 private:

};

#endif
