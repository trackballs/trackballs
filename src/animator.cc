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

#include "animator.h"

#include "gameHook.h"
#include "general.h"
#include "guile.h"

Animator::Animator(Real length, Real position, Real direction, Real v0, Real v1, int repeat,
                   SCM fun)
    : GameHook(Role_GameHook),
      direction(direction),
      position(std::fmod(position, 1.0)),
      length(length),
      value0(v0),
      value1(v1),
      repeat(repeat),
      fun(fun) {
  if (fun) scm_gc_protect_object(fun);
}
Animator::~Animator() {
  if (fun) scm_gc_unprotect_object(fun);
}
Real Animator::getValue() const {
  return value0 * (1.0 - position / length) + position / length * value1;
}
void Animator::tick(Real td) {
  GameHook::tick(td);

  if (!direction) return;
  position += td * direction;
  if (position <= 0 && repeat & ANIMATOR_0_STOP) {
    direction = 0.0;
    position = 0.0;
  }
  if (position <= 0 && repeat & ANIMATOR_0_BOUNCE) {
    direction = -direction;
    position = -position;
  }
  if (position <= 0 && repeat & ANIMATOR_0_WRAP) { position = length + position; }
  if (position >= length && repeat & ANIMATOR_1_STOP) {
    direction = 0.0;
    position = length;
  }
  if (position >= length && repeat & ANIMATOR_1_BOUNCE) {
    direction = -direction;
    position = length * 2.0 - position;
  }
  if (position >= length && repeat & ANIMATOR_1_WRAP) { position = position - length; }

  scm_catch_apply_1(fun, scm_from_double(getValue()));

  if (position <= 0 && repeat & ANIMATOR_0_REMOVE) {
    remove();
    return;
  }
  if (position >= length && repeat & ANIMATOR_1_REMOVE) {
    remove();
    return;
  }
}
