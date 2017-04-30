/* heightModifier.cc
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

/*
  WARNING: no check for bounds of cells. NEVER try to modify heights of a
           bordering cell !!! (core dump!)
	   To be corrected.
*/
#include "general.h"
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "heightModifier.h"
#include "ball.h"
#include "player.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"

using namespace std;

HeightModifier::HeightModifier(int corner, int x, int y, Real min, Real max, Real freq, Real phase,
 			       int not1, int not2, int not3)  {
  this->x=x;
  this->y=y;
  this->min=min;
  this->max=max;
  this->freq=freq;
  this->phase=phase;
  if (corner >= 10)  {
  this->avg_center = 1;
  corner -= 10;
  } else
  this->avg_center = 0;
  if ((corner < 0)||(corner > 3))  {
	fprintf(stderr, "Warning: erroneous corner ID (%d) of HeightModifier (%d,%d). Using '0' instead.\n", corner, x, y);
        corner = 0;
  }
  this->corner=corner;
  this->not1 = not1;
  this->not2 = not2;
  this->not3 = not3;
  // not used
  position[0] = 0.;
  position[1] = 0.;  
  position[2] = 0.;
  primaryColor[0] = 1.;
  primaryColor[1] = 1.;
  primaryColor[2] = 1.;
  secondaryColor[0] = 1.;
  secondaryColor[1] = 1.;
  secondaryColor[2] = 1.;
}

void HeightModifier::draw() {
}

void HeightModifier::tick(Real t) {
  int i;
  double tt=Game::current->gameTime;
  Cell& c=Game::current->map->cell(x,y);
  int x1, y1, x2, y2, x3, y3;
  int cor1, cor2, cor3;

  float v = min + (max-min)*(1. + cos(phase+(tt*freq)*2.*3.14159)) / 2.;
  
  if (corner == 4) {
  	c.heights[4] = v;
  return;
  }
  
  switch(corner)
    {
    case 0:
      x1 = x-1; y1 = y;
      x2 = x; y2 = y-1;
      x3 = x-1; y3 = y-1;
      cor1 = 2;
      cor2 = 1;
      cor3 = 3;
      break;
    case 1:
      x1 = x; y1 = y+1;
      x2 = x-1; y2 = y;
      x3 = x-1; y3 = y+1;
      cor1 = 0;
      cor2 = 3;
      cor3 = 2;
      break;
    case 2:
      x1 = x+1; y1 = y;
      x2 = x; y2 = y-1;
      x3 = x+1; y3 = y-1;
      cor1 = 0;
      cor2 = 3;
      cor3 = 1;
      break;
    case 3:
      x1 = x+1; y1 = y;
      x2 = x; y2 = y+1;
      x3 = x+1; y3 = y+1;
      cor1 = 1;
      cor2 = 2;
      cor3 = 0;
      break;
    }

  Cell& c1 = Game::current->map->cell(x1,y1);
  Cell& c2 = Game::current->map->cell(x2,y2);
  Cell& c3 = Game::current->map->cell(x3,y3);


  c.heights[corner] = v;
  if ((cor1 != not1)&&(cor1 != not2)&&(cor1 != not3))
    c1.heights[cor1] = v;
  if ((cor2 != not1)&&(cor2 != not2)&&(cor2 != not3))
    c2.heights[cor2] = v;
  if ((cor3 != not1)&&(cor3 != not2)&&(cor3 != not3))
    c3.heights[cor3] = v;

  c.displayListDirty=1;
  c1.displayListDirty=1;
  c2.displayListDirty=1;
  c3.displayListDirty=1;

  if (avg_center)  {
    c.heights[4] =
       (c.heights[0] + c.heights[1] + c.heights[2] + c.heights[3]) / 4.;
  }
  
}


