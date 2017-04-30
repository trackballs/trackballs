/* teleport.h
   teleport the ball to the destination

   Copyright (C) 2000-2003  Mathias Broxvall
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

#ifndef TELEPORT_H
#define TELEPORT_H
#ifndef ANIMATED_H
#include "animated.h"
#endif

class Teleport : public Animated {
public:
  Teleport(int x,int y,int dx,int dy,Real radius);
  void draw();
  void draw2();
  void tick(Real t);

 protected:
  int x,y,dx,dy;
  Real radius;
};


#endif
