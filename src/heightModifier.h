/* heightModifier.h
   allow to modify the height of a cell's corner

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

#ifndef HEIGHTMODIFIER_H
#define HEIGHTMODIFIER_H
#ifndef ANIMATED_H
#include "animated.h"
#endif

class HeightModifier : public Animated {
public:
  HeightModifier(int corner, int x, int y, Real min, Real max, Real freq, Real phase,
   		 int not1, int not2, int not3);
  void draw();
  void tick(Real t);

 protected:
   Real min, max, freq, phase;
   int corner, x, y;
   int avg_center;
   int not1, not2, not3;
};


#endif
