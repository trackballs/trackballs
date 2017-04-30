/* pipeConnector.cc
   Connects pipes

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
#include "animated.h"
#include "ball.h"
#include "game.h"
#include "pipeConnector.h"

using namespace std;

class set<PipeConnector*> *PipeConnector::connectors;
void PipeConnector::init() { connectors = new set<PipeConnector *>(); }
void PipeConnector::reset() { delete connectors; connectors = new set<PipeConnector *>(); }
PipeConnector::PipeConnector(Coord3d pos,Real r) :Animated() {
  assign(pos,position); radius = r;
  primaryColor[0]=primaryColor[1]=primaryColor[2]=0.6;
  connectors->insert(this);
}
void PipeConnector::draw() {if(primaryColor[3] >= 1.0) drawMe();}
void PipeConnector::draw2() {if(primaryColor[3] < 1.0) drawMe();}
void PipeConnector::drawMe() {
  int i;

  glPushAttrib(GL_ENABLE_BIT);
  GLfloat color[4];
  GLfloat specular[4];
  for(i=0;i<4;i++)
	color[i] = primaryColor[i];
  specular[0] = specular[1] = specular[2] = 0.5;
  specular[3] = 1.0;
  double shininess = 20.0;
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMaterialf(GL_FRONT,GL_SHININESS,shininess);	

  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glColor4f(1.0,1.0,1.0,0.5);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);
  gluSphere(qball, radius, 11, 11);
  glPopMatrix();
  glPopAttrib();
}
void PipeConnector::tick(Real t) {}
void PipeConnector::onRemove() {Animated::onRemove(); connectors->erase(this);}
