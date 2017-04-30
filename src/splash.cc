/* splash.cc
   A splash effect for when the ball hits a liquid. Eyecandy only.

   Copyright (C) 2003  Mathias Broxvall

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
#include "splash.h"
#include "glHelp.h"
#include "settings.h"

using namespace std;

Splash::Splash(Coord3d center, Coord3d velocity, GLfloat color[4], double strength, double radius) {
  int i,j;

  timeLeft=3.0;
  nDroplets = (int) strength;
  if(nDroplets > 32) nDroplets=32;
  for(i=0;i<4;i++) primaryColor[i]=color[i];
  for(i=0;i<nDroplets;i++) {	
	positions[i][0] = center[0] + (frandom()-0.5)*2.0*radius;
	positions[i][1] = center[1] + (frandom()-0.5)*2.0*radius;
	positions[i][2] = center[2];
	velocities[i][0] = velocity[0] + (frandom()-0.5) * 1.0; 
	velocities[i][1] = velocity[1] + (frandom()-0.5) * 1.0;
	velocities[i][2] = velocity[2] + frandom() * 1.0;
  }
}

void Splash::draw() {
}
void Splash::draw2() {
  int i;

  if(Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE) return;

  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glColor4fv(primaryColor);

  if(screenWidth <= 800)
	glPointSize(2.0);
  else if(screenWidth == 1024)
	glPointSize(3.0);
  else if(screenWidth >= 1280)
	glPointSize(4.0);

  glBegin(GL_POINTS);
  for(i=0;i<nDroplets;i++)
	glVertex3f(positions[i][0],positions[i][1],positions[i][2]);
  glEnd();
  glPopAttrib();
}

void Splash::tick(Real t) {
  int i,j;

  timeLeft -= t;
  if(t < 0.0) remove();
  for(i=0;i<nDroplets;i++) {
	for(j=0;j<3;j++) positions[i][j] += velocities[i][j] * t;
	velocities[i][2] -= t * 2.0;
  }
}
