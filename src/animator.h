/* animator.h
   Used by guile functions to easily get a soft transition of real values

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

#ifndef ANIMATOR_H
#define ANIMATOR_H

#define ANIMATOR_0_REMOVE  1
#define ANIMATOR_0_STOP    2
#define ANIMATOR_0_BOUNCE  4
#define ANIMATOR_0_WRAP    8
#define ANIMATOR_1_REMOVE  16
#define ANIMATOR_1_STOP    32
#define ANIMATOR_1_BOUNCE  64
#define ANIMATOR_1_WRAP    128


class Animator : public GameHook {
 public:
  Animator(Real length,Real position,Real direction,Real v0, Real v1, int repeat,SCM fun);
  ~Animator();
  void tick(Real);
  Real getValue();

  Real direction;
  Real position, length;
  Real value0, value1;
  int repeat;

  SCM fun;
};

#endif
