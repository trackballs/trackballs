/* pipe.cc
   One-way pipes dragging balls along them

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
#include "pipe.h"
#include "ball.h"
#include "game.h"

using namespace std;

class set<Pipe*> *Pipe::pipes;

void Pipe::init() { pipes = new set<Pipe *>(); }
void Pipe::reset() { delete pipes; pipes = new set<Pipe *>(); }
Pipe::Pipe(Coord3d f,Coord3d t,Real r) :Animated() {
  /* Note that the position attribute of Pipes are not used, use rather the to/from values */

  assign(f,from);
  assign(t,to);
  radius = r;
  pipes->insert(this);
  primaryColor[0]=primaryColor[1]=primaryColor[2]=0.6;
  
  windForward=windBackward=0.0;
  windForward=0.0;
  computeBoundingBox();
}
void Pipe::draw() { if(primaryColor[3] >= 1.0) drawTrunk();  }
void Pipe::draw2() { if(primaryColor[3] < 1.0) drawTrunk(); }
void Pipe::drawTrunk() {
  int i;

  up[0]=up[1]=0.0; up[2]=1.0;
  Coord3d dir; sub(to,from,dir); normalize(dir);
  right[0] = dir[1]; right[1] = -dir[0]; right[2] = 0.0;
  normalize(right);
  crossProduct(dir,right,up);
  if(up[2] < 0.0) { up[0] *= -1.; up[1] *= -1.; up[2] *= -1.; }

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
  glBegin(GL_TRIANGLE_STRIP);

  for(i=0;i<=10;i++) {
	double v=((double) i)*M_PI*2.0/10.0;

	glNormal3f(sin(v)*up[0]+cos(v)*right[0],
			   sin(v)*up[1]+cos(v)*right[1],
			   sin(v)*up[2]+cos(v)*right[2]);
	glVertex3f(from[0]+radius*sin(v)*up[0]+radius*cos(v)*right[0],
			   from[1]+radius*sin(v)*up[1]+radius*cos(v)*right[1],
			   from[2]+radius*sin(v)*up[2]+radius*cos(v)*right[2]);
	glVertex3f(to[0]+radius*sin(v)*up[0]+radius*cos(v)*right[0],
			   to[1]+radius*sin(v)*up[1]+radius*cos(v)*right[1],
			   to[2]+radius*sin(v)*up[2]+radius*cos(v)*right[2]);
  }
  glEnd();
  glPopAttrib();
}
void Pipe::tick(Real t) {
  /* TODO. Ugly fix, a better fix is to make from/to etc. *private* variables and add this fn.
	 inside the accessor methods */
  computeBoundingBox();
}
void Pipe::computeBoundingBox() {
  for(int i=0;i<3;i++) {
	boundingBox[0][i]=fmin(from[i]-radius,to[i]-radius)-position[i];
	boundingBox[1][i]=fmax(from[i]+radius,to[i]+radius)-position[i];
  }
}
void Pipe::onRemove() {Animated::onRemove(); pipes->erase(this);}
