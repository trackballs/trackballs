/* flag.cc
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
#include "flag.h"
#include "ball.h"
#include "player.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"

using namespace std;

Flag::Flag(int x,int y,int points,int visible,Real radius) {
  this->points=points;
  this->x=x;
  this->y=y;
  this->visible=visible;
  this->radius=radius;
  position[0] = x+0.5;
  position[1] = y+0.5;  
  position[2] = Game::current->map->getHeight(position[0],position[1]);
  primaryColor[0] = 0.5;
  primaryColor[1] = 0.5;
  primaryColor[2] = 1.0;
  secondaryColor[0] = 0.8;
  secondaryColor[1] = 0.8;
  secondaryColor[2] = 0.8;
}

void Flag::draw() {
  int i;

  if(!visible) return;

  double d1=Game::current->gameTime*1.0,d2=3.0,d3=0.5;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);
  //  glRotatef(life*rotation,0.0,0.0,1.0);
 
  GLfloat color[4];
  for(i=0;i<3;i++)
    color[i] = primaryColor[i];
  color[3] = 0.0;

  GLfloat specular[4];
  for(i=0;i<3;i++)
    specular[i] = specularColor[i];
  specular[3] = 0.0;

  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMaterialf(GL_FRONT,GL_SHININESS,10.0);
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glShadeModel(GL_SMOOTH);

  glColor3f(1.0,1.0,1.0);
  glBegin(GL_TRIANGLE_STRIP); {
	double x1 = 0.0;
	double x2 = 0.1*d3*sin(d1+d2*0.1);
	double x3 = 0.2*d3*sin(d1+d2*0.2);
	double x4 = 0.3*d3*sin(d1+d2*0.3);
	double x5 = 0.4*d3*sin(d1+d2*0.4);
	Coord3d up={0.0,0.0,1.0},b={0.0,0.0,0.0},normal;
	
	b[0]=x2 - x1;
	b[1]=0.1;
	crossProduct(up,b,normal);
	normalize(normal);
	glNormal3dv(normal);
	glVertex3f(x1,-0.0,0.7);
	glVertex3f(x1,-0.0,0.5);

	b[0]=x3 - x1;
	b[1]=0.2;
	crossProduct(up,b,normal);
	normalize(normal);
	glNormal3dv(normal);
	glVertex3f(x2,-0.1,0.7);
	glVertex3f(x2,-0.1,0.5);

 	b[0]=x4 - x2;
	b[1]=0.2;
	crossProduct(up,b,normal);
	normalize(normal);
	glNormal3dv(normal);
	glVertex3f(x3,-0.2,0.7);
	glVertex3f(x3,-0.2,0.5);

 	b[0]=x5 - x3;
	b[1]=0.2;
	crossProduct(up,b,normal);
	normalize(normal);
	glNormal3dv(normal);
	glVertex3f(x4,-0.3,0.7);
	glVertex3f(x4,-0.3,0.5);

 	b[0]=x5 - x4;
	b[1]=0.1;
	crossProduct(up,b,normal);
	normalize(normal);
	glNormal3dv(normal);
	glVertex3f(x5,-0.4,0.7);
	glVertex3f(x5,-0.4,0.5);
  } glEnd();


  for(i=0;i<3;i++)
    color[i] = secondaryColor[i];  
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glBegin(GL_LINES); {
	glNormal3f(-1.,-1.,0.);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,0.0,0.71);
  } glEnd();

  glPopMatrix();
  glPopAttrib();
}

void Flag::tick(Real t) {
  position[2] = Game::current->map->getHeight(position[0],position[1]);    
  Player *p = Game::current->player1;
  Coord3d diff;

  sub(position,p->position,diff);
  if(!visible) diff[2] = 0.0;
  if(length(diff) < p->radius + radius)
	onGet();
}

void Flag::onGet() {
  position[2] += 1.0;
  playEffect(SFX_GOT_FLAG);
  new ScoreSign(points,position);
  remove();
}
