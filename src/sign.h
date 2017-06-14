/* sign.h
   Information given on the playfield (such as scores etc.)

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

#ifndef SIGN_H
#define SIGN_H

#include "animated.h"

class Sign : public Animated {
 public:
  Sign(const char* text, Real life, Real scale, Real rotation, Coord3d pos);
  virtual ~Sign();
  void mkTexture(const char*);

  virtual int generateBuffers(GLuint*& idxbufs, GLuint*& databufs);
  virtual void drawBuffers1(GLuint* idxbufs, GLuint* databufs);
  virtual void drawBuffers2(GLuint* idxbufs, GLuint* databufs);

  void tick(Real);

 protected:
  int width, height;
  Real life, scale, rotation, tot_rot;

  GLuint textimg;
  GLfloat texcoord[4];
};

#endif
