/* sideSpike.h
   Dangerous spikes shooting from the sides

   Copyright (C) 2000  Mathias Broxvall
   		       Yannick Perret

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

#ifndef SIDESPIKE_H
#define SIDESPIKE_H
#ifndef ANIMATED_H
#include "animated.h"
#endif

class SideSpike : public Animated {
public:
  SideSpike(Coord3d position,Real speed,Real phase,int side);
  void draw();
  void tick(Real t);

  Real speed,phase;
  int side;
 private:
  int soundDone;
  Real offset;
};


#endif
