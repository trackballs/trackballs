/* teleport.cc
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

#include "general.h"
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "teleport.h"
#include "ball.h"
#include "player.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"

using namespace std;

Teleport::Teleport(int x,int y,int dx,int dy,Real radius) {
  this->x  = x;
  this->y  = y;
  this->dx = dx;
  this->dy = dy;
  this->radius=radius;
  position[0] = x+0.5;
  position[1] = y+0.5;  
  position[2] = Game::current->map->getHeight(position[0],position[1]);
  primaryColor[0] = .5;
  primaryColor[1] = .7;
  primaryColor[2] = .6;
  primaryColor[3] = 1.0;
  secondaryColor[0] = 1.0;
  secondaryColor[1] = 0.9;
  secondaryColor[2] = 0.4;
  secondaryColor[3] = 1.0;
  is_on=1;
}

void Teleport::draw() {
  int i;

  double d1=Game::current->gameTime*1.0,d2=3.0,d3=0.5;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);

  glMaterialfv(GL_FRONT,GL_SPECULAR,specularColor);
  glMaterialf(GL_FRONT,GL_SHININESS,15.0);
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,primaryColor);
  glShadeModel(GL_SMOOTH);

  glColor3f(1.0,1.0,1.0);
  glBegin(GL_TRIANGLE_FAN); {
  	glNormal3f(0., 0., 1.);
   	glVertex3f(0., 0., 0.3);
  	for(i=0; i<=16; i++)
	  {
	  glNormal3f(cos(2.*3.14159*i/16.),
	  	     sin(2.*3.14159*i/16.),
		     .4);
	  glVertex3f(radius*cos(2.*3.14159*i/16.),
	  	     radius*sin(2.*3.14159*i/16.),
		     0.1);
	  }
  } glEnd();

  double rscale=radius/0.3;
  /* MB: Added a stargate like appearance to teleporters **/
  // north side
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3f(0.0,1.0,0.0);
  for(i=0;i<=10;i++) {
	double angle=2.0*M_PI*i/10.0;
	glVertex3f(sin(angle)*0.4*rscale,0.05*rscale,cos(angle)*0.4*rscale+0.5);
	glVertex3f(sin(angle)*0.5*rscale,0.05*rscale,cos(angle)*0.5*rscale+0.5);
  }
  glEnd();
  // south side
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3f(0.0,1.0,0.0);
  for(i=0;i<=10;i++) {
	double angle=2.0*M_PI*i/10.0;
	glVertex3f(sin(angle)*0.4*rscale,-0.05*rscale,cos(angle)*0.4*rscale+0.5);
	glVertex3f(sin(angle)*0.5*rscale,-0.05*rscale,cos(angle)*0.5*rscale+0.5);
  }
  glEnd();
  // outer side
  glBegin(GL_TRIANGLE_STRIP);
  for(i=0;i<=10;i++) {
	double angle=2.0*M_PI*i/10.0;
	glNormal3f(sin(angle),cos(angle),0.0);
	glVertex3f(sin(angle)*0.5*rscale,+0.05*rscale,cos(angle)*0.5*rscale+0.5);
	glVertex3f(sin(angle)*0.5*rscale,-0.05*rscale,cos(angle)*0.5*rscale+0.5);
  }
  glEnd();
  // inner side
  glBegin(GL_TRIANGLE_STRIP);
  for(i=0;i<=10;i++) {
	double angle=2.0*M_PI*i/10.0;
	glNormal3f(-sin(angle),-cos(angle),0.0);
	glVertex3f(sin(angle)*0.4*rscale,+0.05*rscale,cos(angle)*0.4*rscale+0.5);
	glVertex3f(sin(angle)*0.4*rscale,-0.05*rscale,cos(angle)*0.4*rscale+0.5);
  }
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

void Teleport::draw2() {
  if(!is_on) return;

  double rscale=radius/0.3;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);

  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glColor4f(secondaryColor[0],secondaryColor[1],secondaryColor[2],secondaryColor[3]*(0.4 + 0.3 * frandom()));
  glShadeModel(GL_SMOOTH);

  glBegin(GL_TRIANGLE_FAN);
  glNormal3f(0.0,-1.0,0.0);
  glVertex3f(0.0,0.0,0.5);
  for(int i=0;i<=10;i++) {
	double angle=2.0*M_PI*i/10.0;
	glVertex3f(sin(angle)*0.4*rscale,0.0,cos(angle)*0.4*rscale+0.5);
  }
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

void Teleport::tick(Real t) {
  position[2] = Game::current->map->getHeight(position[0],position[1]);    
  Player *p = Game::current->player1;
  Coord3d diff;

  sub(position,p->position,diff);
  if(length(diff) < p->radius + radius) {
  	p->position[0] = dx + .5;
	p->position[1] = dy + .5;
	p->position[2] = Game::current->map->getHeight(p->position[0],p->position[1]);
	// generate a sound for the teleportation
	// playEffect(SFX_TELEPORT);
	}
}


