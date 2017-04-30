/* template.cc
   Use to create new .cc project files

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
#include "goal.h"
#include "sound.h"
#include "mainMode.h"
#include "player.h"
#include "game.h"
#include "map.h"

using namespace std;

Goal::Goal(int x,int y,int rotate,char *nextLevel) :Flag(x,y,1000,1,0.2) {
  strcpy(this->nextLevel,nextLevel);
  this->rotate = rotate;
  primaryColor[0] = 0.9;
  primaryColor[1] = 0.8;
  primaryColor[2] = 0.3;
  specularColor[0] = 0.9*2.;
  specularColor[1] = 0.8*2.;
  specularColor[2] = 0.3*2.;
}
void Goal::onGet() {
  // TODO. Make sure player is entering the goal from the right direction
  if(!Game::current->player1->hasWon) {	
    strcpy(Game::current->nextLevel,nextLevel);
	if(Game::current->map->isBonus) MainMode::mainMode->bonusLevelComplete();
	else MainMode::mainMode->levelComplete();
  }
}
void Goal::draw() {
  int i;

  if(!visible) return;
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);
  if(rotate)
    glRotatef(90.0,0.0,0.0,1.0);
  glTranslatef(-.5,0.0,0.0);
  GLfloat color[4];
  for(i=0;i<3;i++)
    color[i] = primaryColor[i];
  color[3] = 0.0;

  GLfloat specular[4];
  for(i=0;i<3;i++)
    specular[i] = specularColor[i];
  specular[3] = 0.0;

  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMaterialf(GL_FRONT,GL_SHININESS,50.0);
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glShadeModel(GL_SMOOTH);
  glColor3f(1.0,1.0,1.0);
  glBegin(GL_TRIANGLES);
  /* Right side, facing down */
  glNormal3f(0.0,-1.0,0.0);
  glVertex3f(1.0,-.1,0.0); glVertex3f(0.9,-.1,0.0); glVertex3f(0.9,-.1,0.6);
  glVertex3f(0.9,-.1,0.6); glVertex3f(1.0,-.1,0.6); glVertex3f(1.0,-.1,0.0);

  /* Left side, facing down */
  glVertex3f(0.0,-.1,0.0); glVertex3f(0.1,-.1,0.0); glVertex3f(0.1,-.1,0.6);
  glVertex3f(0.1,-.1,0.6); glVertex3f(0.0,-.1,0.6); glVertex3f(0.0,-.1,0.0);

  /* Right side, facing up */
  glNormal3f(0.0,1.0,0.0); 
  glVertex3f(1.0,.1,0.0); glVertex3f(0.9,.1,0.0); glVertex3f(0.9,.1,0.6);
  glVertex3f(0.9,.1,0.6); glVertex3f(1.0,.1,0.6); glVertex3f(1.0,.1,0.0);

  /* Left side, facing up */
  glVertex3f(0.0,.1,0.0); glVertex3f(0.1,.1,0.0); glVertex3f(0.1,.1,0.6);
  glVertex3f(0.1,.1,0.6); glVertex3f(0.0,.1,0.6); glVertex3f(0.0,.1,0.0);

  /* Right, right */
  glNormal3f(0.0,1.0,0.0);
  glVertex3f(1.0,-.1,0.0); glVertex3f(1.0,+.1,0.0); glVertex3f(1.0,+.1,0.6); 
  glVertex3f(1.0,+.1,0.6); glVertex3f(1.0,-.1,0.6); glVertex3f(1.0,-.1,0.0);

  /* Left, right */
  glNormal3f(0.0,1.0,0.0);
  glVertex3f(0.1,-.1,0.0); glVertex3f(0.1,+.1,0.0); glVertex3f(0.1,+.1,0.6); 
  glVertex3f(0.1,+.1,0.6); glVertex3f(0.1,-.1,0.6); glVertex3f(0.1,-.1,0.0);

  /* Right, left */
  glNormal3f(0.0,-1.0,0.0);
  glVertex3f(0.9,-.1,0.0); glVertex3f(0.9,+.1,0.0); glVertex3f(0.9,+.1,0.6); 
  glVertex3f(0.9,+.1,0.6); glVertex3f(0.9,-.1,0.6); glVertex3f(0.9,-.1,0.0);

  /* Left, left */
  glNormal3f(0.0,1.0,0.0);
  glVertex3f(0.0,-.1,0.0); glVertex3f(0.0,+.1,0.0); glVertex3f(0.0,+.1,0.6); 
  glVertex3f(0.0,+.1,0.6); glVertex3f(0.0,-.1,0.6); glVertex3f(0.0,-.1,0.0);  
  glEnd();

  /* Arc, facing down */
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3d(0.0,1.0,0.0);
  for(i=0;i<=6;i++) {
    glVertex3d(.5+.5*cos(i*M_PI/6.0),+.1,.6+.5*sin(i*M_PI/6.0));
    glVertex3d(.5+.4*cos(i*M_PI/6.0),+.1,.6+.4*sin(i*M_PI/6.0));
  }
  glEnd();

  /* Arc, facing up */
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3d(0.0,-1.0,0.0);
  for(i=0;i<=6;i++) {
    glVertex3d(.5+.5*cos(i*M_PI/6.0),-.1,.6+.5*sin(i*M_PI/6.0));
    glVertex3d(.5+.4*cos(i*M_PI/6.0),-.1,.6+.4*sin(i*M_PI/6.0));
  }
  glEnd();

  /* Arc, facing out */
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3d(0.0,-1.0,0.0);
  for(i=0;i<=6;i++) {
    glVertex3d(.5+.5*cos(i*M_PI/6.0),+.1,.6+.5*sin(i*M_PI/6.0));
    glVertex3d(.5+.5*cos(i*M_PI/6.0),-.1,.6+.5*sin(i*M_PI/6.0));
  }
  glEnd();

  /* Arc, facing in */
  glBegin(GL_TRIANGLE_STRIP);
  glNormal3d(0.0,-1.0,0.0);
  for(i=0;i<=6;i++) {
    glVertex3d(.5+.4*cos(i*M_PI/6.0),+.1,.6+.4*sin(i*M_PI/6.0));
    glVertex3d(.5+.4*cos(i*M_PI/6.0),-.1,.6+.4*sin(i*M_PI/6.0));
  }
  glEnd();

  glPopMatrix();
  glPopAttrib();
}
