/* debris.cc
   Use to create new .cc project files

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

#include "debris.h"

Debris::Debris(Ball* parent, const Coord3d& pos, const Coord3d& vel, double dur)
    : Ball(Role_Ball) {
  position = pos;
  velocity = vel;
  crashTolerance = 10000;
  if (parent) {
    primaryColor = parent->primaryColor;
    for (int i = 0; i < NUM_MODS; i++) modTimeLeft[i] = parent->modTimeLeft[i];
    modTimeLeft[MOD_DIZZY] = 0.0;
    modTimeLeft[MOD_NITRO] = 0.0;
    texture = parent->texture;
  }
  if (parent)
    initialSize = parent->realRadius / 4.0;
  else
    initialSize = 0.05;
  endingSize = 0.0;
  duration = dur;
  elapsedTime = 0.0;
  floatingUp = false;
  calcRadius();
}
void Debris::calcRadius() {
  double a, b;
  b = elapsedTime / duration;
  a = 1.0 - b;
  realRadius = initialSize * a + endingSize * b;
  radius = realRadius;
}
void Debris::tick(Real t) {
  elapsedTime += t;
  calcRadius();
  if (elapsedTime > duration)
    remove();  // delete this;
  else
    Ball::tick(t);
}
void Debris::die(int /*how*/) { remove(); }
