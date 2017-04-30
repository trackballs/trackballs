/* cyclicPlatform.h
   A cyclicPlatform modifies the map so that a (flat) rectangle of it
   goes up and down cyclicaly with a specified frequency 

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

#ifndef CYCLICPLATFORM_H
#define CYCLICPLATFORM_H

#ifndef GAMEHOOK_H
#include "gameHook.h"
#endif

class CyclicPlatform : public GameHook {
 public:
  /* Area affected x1,y1 -> x2,y2
     Total cycle time timeLow+timeRise+timeHigh+timeFall
     offset is the cycle offset from timeZero
  */
  CyclicPlatform(int x1,int y1,int x2,int y2,Real low,Real high,Real offset,Real speed);
  void tick(Real);

  Real speed;
 private:
  int x1,y1,x2,y2;
  Real phase;
  Real low,high; //,timeLow,timeRise,timeHigh,timeFall,cycleTime;
};

#endif
