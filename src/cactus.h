/* cactus.h
   Hurts when touched

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

#ifndef CACTUS_H
#define CACTUS_H

#include "animated.h"

class Cactus : public Animated {
 public:
  Cactus(Game& g, Real x, Real y, Real radius);

  virtual void updateBuffers(const GLuint*, const GLuint*, const GLuint*, bool);
  virtual void drawBuffers1(const GLuint*) const;
  virtual void drawBuffers2(const GLuint*) const;

  void tick(Real t);

 protected:
  Real base_radius;
  Real killed_time;
  bool killed;
};

#endif
