/* splash.h
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

#ifndef SPLASH_H
#define SPLASH_H

#include "animated.h"

class Splash : public Animated {
 public:
  Splash(Game& g, Coord3d center, Coord3d velocity, Color color, double strength,
         double radius);
  void tick(Real t);

  virtual void updateBuffers(const GLuint*, const GLuint*, const GLuint*, bool);
  virtual void drawBuffers1(const GLuint*) const;
  virtual void drawBuffers2(const GLuint*) const;

 private:
  int nDroplets;
  double timeLeft;
  double positions[32][4];
  double velocities[32][4];
};

#endif
