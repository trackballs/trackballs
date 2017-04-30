/* diamond.cc
   Gives points when captured

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
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "ball.h"
#include "player.h"
#include "sound.h"
#include "diamond.h"
#include "sign.h"

using namespace std;

Diamond::Diamond(Coord3d pos) {  
  assign(pos,position);

  specularColor[0] = specularColor[1] = specularColor[2] = 1.0;
  primaryColor[0]=0.7;
  primaryColor[1]=0.7;
  primaryColor[2]=0.9;
  primaryColor[3]=0.7;
  fade=1.0;
}

void Diamond::draw() {
}
void Diamond::draw2() {
  int i;

  if(fade <= 0.0) return;
  glPushAttrib(GL_ENABLE_BIT);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);
  
  GLfloat color[4];
  for(i=0;i<4;i++)
	color[i] = primaryColor[i];
  color[3] *= fade;

  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glMaterialfv(GL_FRONT,GL_SPECULAR,specularColor);
  glMaterialf(GL_FRONT,GL_SHININESS,50.0);	
  glEnable(GL_BLEND);
  glShadeModel(GL_FLAT);
  glDisable(GL_CULL_FACE);

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0,0.0,-.4);
  for(i=0;i<=6;i++) {
	double v=i*2.0*M_PI/6.0 + Game::current->gameTime;
	glVertex3f(sin(v)*0.25,cos(v)*0.25,0.0);
	glNormal3f(sin(v),cos(v),0.0);
  }	
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0,0.0,+.4);
  for(i=6;i>=0;i--) {
	double v=i*2.0*M_PI/6.0 + Game::current->gameTime;
	glVertex3f(sin(v)*0.25,cos(v)*0.25,0.0);
	glNormal3f(sin(v),cos(v),0.0);
  }	
  glEnd();

  glShadeModel(GL_SMOOTH);
  glPopMatrix();
  glPopAttrib();
}
void Diamond::tick(Real t) {
  Coord3d v0;
  if(fade <= 0.0) return;
  sub(Game::current->player1->position,position,v0); 
  if(length(v0) < 0.3 + Game::current->player1->radius) {
	if(!taken)
	  onGet();
	taken=1;
  } else {
	taken=0;
	fade = min(1.0,fade+0.5 * t);
  }
}
void Diamond::onGet() {
  assign(position,Game::current->map->startPosition);
  playEffect(SFX_GOT_FLAG);
  fade=-14.0;

  Coord3d signPos;
  assign(position,signPos);
  signPos[2] += 1.0;
  new Sign("Save point",6.0,1.0,60.0,signPos);
}
