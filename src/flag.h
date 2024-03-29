/* flag.h
   Gives points when captured

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

#ifndef FLAG_H
#define FLAG_H

#include "animated.h"

#include <map>
#include <vector>

class Flag : public Animated {
 public:
  Flag(Game& g, Real x, Real y, int points, int visible, Real radius, int role = Role_Flag);

  // all of these are noops; use `FlagRenderer` instead
  virtual void updateBuffers(const GLuint*, const GLuint*, const GLuint*, bool) {}
  virtual void drawBuffers1(const GLuint*) const {}
  virtual void drawBuffers2(const GLuint*) const {}

  void tick(Real t);
  virtual void onGet();

  int visible;

 protected:
  Real radius;
};

struct FlagDrawState {
  Color poleColor;
  Color flagColor;
};

struct FlagBuffer {
  GLuint vao;
  GLuint vertexBuffer;
  Real lastTime;  // needs exact same units as gameTime
  bool active;
};

class FlagRenderer {
 public:
  FlagRenderer();
  ~FlagRenderer();
  void draw(std::vector<GameHook*> flags);

 private:
  GLuint indexBuffer;
  std::map<struct FlagDrawState, struct FlagBuffer> buffers;
};

#endif
