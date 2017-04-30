/* switch.cc
   A switch which can be triggered by user to accomplish something

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
#include "gameHook.h"
#include "animated.h"
#include "game.h"
#include "map.h"
#include "switch.h"
#include "ball.h"
#include "player.h"
#include "sound.h"

using namespace std;

CSwitch::CSwitch(Real x,Real y,SCM on,SCM off) {
  position[0] = x;
  position[1] = y;
  position[2] = Game::current->map->getHeight(x,y);
  primaryColor[0] = 0.8;
  primaryColor[1] = 0.8;
  primaryColor[2] = 0.8;
  this->on = on; this->off = off;
  scm_gc_protect_object(on);
  scm_gc_protect_object(off);
}
CSwitch::~CSwitch() {
  scm_gc_unprotect_object(on);
  scm_gc_unprotect_object(off);
  this->Animated::~Animated();
}
void CSwitch::draw() {
  int i;

  glPushAttrib(GL_ENABLE_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);

  GLfloat color[4];
  for(i=0;i<3;i++)
    color[i] = primaryColor[i];
  color[3] = 0.0;

  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  gluSphere(qball, 0.2, 5, 5);
  glBegin(GL_LINES);
  glVertex3f(0.0,0.0,0.0);
  glVertex3f(0.0,is_on?0.2:-0.2,0.3);
  glEnd();
  glTranslatef(0.0,is_on?0.2:-0.2,0.3);
  gluSphere(qball, 0.05, 5, 5);
  glPopMatrix(); 
  glPopAttrib();
}
void CSwitch::tick(Real t) {
  Coord3d v;
  Player *player = Game::current->player1;
  sub(position,Game::current->player1->position,v);

  double dist = length(v);
  if(dist < player->radius + 0.3) {
	if(!is_touched) {
	  if(is_on) { is_on = 0; scm_apply_0(off,SCM_EOL); }
	  else { is_on = 1; scm_apply_0(on,SCM_EOL); }
	  playEffect(SFX_SWITCH);
	}
	is_touched = 1;
  } else is_touched = 0;
}
