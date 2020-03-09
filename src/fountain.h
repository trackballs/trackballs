/* fountain.h
   Decoration that continously creates droplets of specific color.

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

#ifndef FOUNTAIN_H
#define FOUNTAIN_H

#include "animated.h"

class Fountain : public Animated {
 public:
  Fountain(Game& g, const Coord3d& position, double randomSpeed, double radius,
           double strength);

  virtual void generateBuffers(const GLuint*, const GLuint*, const GLuint*, bool) const;
  virtual void drawBuffers1(const GLuint*) const;
  virtual void drawBuffers2(const GLuint*) const;

  void tick(double);

  double randomSpeed, radius, strength;
  Coord3d velocity;

 private:
  double createPoint;  // when > 0.0, create a new point

  Coord3d positions[800], velocities[800];
  double creationTime[800];
  int nextPoint, drawFrom;
};

#endif
