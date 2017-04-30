/* forcefield.h
   Implements the forcefield class, preventing passage of ball in one or both directions.

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

#ifndef FORCEFIELD_H
#define FORCEFIELD_H

#define FF_KILL1    1
#define FF_BOUNCE1  2
#define FF_KILL2    4
#define FF_BOUNCE2  8

class ForceField : public Animated {
 public:
  ForceField(Coord3d position, Coord3d direction, Real height, int allow);
  virtual void draw();
  virtual void draw2();
  virtual void tick(Real);
  virtual void onRemove();

  static void init();
  static void reset();

  Coord3d direction;
  Real height,bounceFactor;
  int allow;

  static class std::set<ForceField *> *forcefields;  
};

#endif
