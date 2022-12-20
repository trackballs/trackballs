/* pipe.h
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

#ifndef PIPE_H
#define PIPE_H

#define PIPE_SOFT_ENTER 1
#define PIPE_SOFT_EXIT 2

#include "animated.h"

class Pipe : public Animated {
 public:
  Pipe(Game& g, const Coord3d& from, const Coord3d& to, Real radius);

  virtual void updateBuffers(const GLuint*, const GLuint*, const GLuint*, bool);
  virtual void drawBuffers1(const GLuint*) const;
  virtual void drawBuffers2(const GLuint*) const;

  virtual void tick(Real t);

  const Coord3d from, to;
  const Real radius;

  Real windForward, windBackward;

 private:
  Color bufferColor;
  void drawTrunk(const GLuint* vaolist) const;
};

#endif
