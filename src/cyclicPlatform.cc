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

#include "general.h"
#include "cyclicPlatform.h"
#include "map.h"
#include "game.h"
#include "ball.h"

using namespace std;

Real timeLow=2.0,timeRise=3.0,timeHigh=2.0,timeFall=3.0;

CyclicPlatform::CyclicPlatform(int x1,int y1,int x2,int y2,Real low,Real high,Real offset,Real speed) {
  this->x1 = x1;
  this->x2 = x2;
  this->y1 = y1;
  this->y2 = y2;
  this->low = low;
  this->high = high;
  phase = offset;
  this->speed = speed;
}

void CyclicPlatform::tick(Real dt) {
  if(is_on)
	phase += dt / speed;
  Real t = fmod(phase, 10.0);
  /*
  Real t = Game::current->gameTime + offset;
  t -= ((int)(t/cycleTime)) * cycleTime;*/
  Map *map = Game::current->map;
  
  Real h,oldHeight=map->cell(x1,y1).heights[0];
  
  if(t < timeLow)
    h=low;
  else if(t < timeLow+timeRise)
    h=low+(high-low)*(t-timeLow)/timeRise;
  else if(t < timeLow+timeRise+timeHigh)
    h=high;
  else
    h=high+(low-high)*(t-timeLow-timeRise-timeHigh)/timeFall;

  if(!map) return;
  for(int x=x1;x<=x2;x++)
    for(int y=y1;y<=y2;y++) {
      Cell& c = map->cell(x,y);
      for(int j=0;j<5;j++) c.heights[j] = h;
	  c.displayListDirty=1;
	  map->cell(x+1,y).displayListDirty=1;
	  map->cell(x-1,y).displayListDirty=1;
	  map->cell(x,y+1).displayListDirty=1;
	  map->cell(x,y-1).displayListDirty=1;

    }

  if(h < oldHeight) {
    set<Ball*>::iterator iter = Ball::balls->begin();
    set<Ball*>::iterator end = Ball::balls->end();
    for(;iter!=end;iter++) {
      Ball *ball = *iter;
      if(ball->position[0] >= x1 && ball->position[0] < x2+1.0 &&
		 ball->position[1] >= y1 && ball->position[1] < y2+1.0 &&
		 ball->position[2]-ball->radius <= oldHeight+0.1)	//		 ball->position[2] - oldHeight < ball->radius + 0.02)
		ball->position[2] += h - oldHeight;
    }
  }
}
