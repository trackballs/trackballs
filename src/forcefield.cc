/* forcefield.cc
   Implements the forcefield class, preventing passage of ball in one or both directions.

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
#include "forcefield.h"

using namespace std;

class set<ForceField*> *ForceField::forcefields;

void ForceField::init() {forcefields = new set<ForceField*>();}
void ForceField::reset() {
  delete forcefields;
  forcefields = new set<ForceField *>();
}


ForceField::ForceField(Coord3d pos,Coord3d dir,Real h,int a) :Animated() {
  assign(pos,position);
  assign(dir,direction);
  height = h;
  allow = a;
  primaryColor[0] = 0.3;
  primaryColor[1] = 1.0;
  primaryColor[2] = 0.3;
  secondaryColor[0] = 1.0;
  secondaryColor[1] = 1.0;
  secondaryColor[2] = 1.0;
  forcefields->insert(this);
  bounceFactor = 2.5;
}
void ForceField::onRemove() {Animated::onRemove(); forcefields->erase(this);}

void ForceField::draw() {}
void ForceField::draw2() {
  int i;

  if(!is_on) return;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glShadeModel(GL_SMOOTH);
  glColor3f(1.0,1.0,1.0);
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);

  glColor4f(primaryColor[0],primaryColor[1],primaryColor[2],0.4 + 0.2 * frandom());
			//0.2 * semiRand(47,(int) (Game::current->gameTime*50.0)));
  glBegin(GL_POLYGON);
  glNormal3d(direction[1],direction[0],direction[2]);
  glVertex3d(position[0],position[1],position[2]);
  glVertex3d(position[0]+direction[0],position[1]+direction[1],position[2]+direction[2]);
  glVertex3d(position[0]+direction[0],position[1]+direction[1],position[2]+direction[2]+height);
  glVertex3d(position[0],position[1],position[2]+height);
  glEnd();

  glColor4f(secondaryColor[0],secondaryColor[1],secondaryColor[2],0.6);

  //  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);

  glBegin(GL_LINE_LOOP);
  glNormal3d(direction[1],direction[0],direction[2]);
  glVertex3d(position[0],position[1],position[2]);
  glVertex3d(position[0]+direction[0],position[1]+direction[1],position[2]+direction[2]);
  glVertex3d(position[0]+direction[0],position[1]+direction[1],position[2]+direction[2]+height);
  glVertex3d(position[0],position[1],position[2]+height);
  glEnd();

  /*
  Coord3d a,b,c;
  assign(position,a);
  assign(position,b);
  b[2] += height;
  add(position,direction,c);
  drawTriangle(a,b,c);
  add(position,direction,a);
  a[2] += height;
  drawTriangle(a,b,c);
  */

  glPopMatrix();
  glPopAttrib();
}

void ForceField::tick(Real t) {
}
