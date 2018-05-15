/** \file bird.h
   Travels along their path and kill player if touched
*/
/*
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

#ifndef BIRD_H
#define BIRD_H

#include "animated.h"

#define BIRD_CONSTANT_HEIGHT 1

class Bird : public Animated {
 public:
  Bird(Real x, Real y, Real dx, Real dy, Real size, Real speed);

  virtual void generateBuffers(const GLuint*, const GLuint*, const GLuint*, bool) const;
  virtual void drawBuffers1(const GLuint*) const;
  virtual void drawBuffers2(const GLuint*) const;

  void tick(Real t);

 protected:
  Real x, y;
  Real dx, dy;
  Real size;
  Real speed;
  Real lng;
  float hide, animation;
  float rotation;
};

#endif
