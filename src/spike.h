/* spike.h
   Dangerous spikes shooting from the ground

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

#ifndef SPIKE_H
#define SPIKE_H

#include "animated.h"
#include "glHelp.h"

class Spike : public Animated {
 public:
  Spike(Coord3d position, Real speed, Real phase);

  virtual int generateBuffers(GLuint*& idxbufs, GLuint*& databufs);
  virtual void drawBuffers1(GLuint* idxbufs, GLuint* databufs);
  virtual void drawBuffers2(GLuint* idxbufs, GLuint* databufs);

  void tick(Real t);

  Real speed, phase;

 private:
  int soundDone;
};

void generateSpikeVBO(GLfloat* data, ushort idxs[][3], int nfacets, Matrix3d rotmtx,
                      Coord3d position, GLfloat sidec[4], GLfloat tipc[4], GLfloat length);

#endif
