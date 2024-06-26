/* setupMode.h
   Allows the human to select player name, color, start level etc.

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

#ifndef SETUPMODE_H
#define SETUPMODE_H

#include "gameMode.h"
#include "glHelp.h"

class SetupMode : public GameMode {
 public:
  SetupMode();
  virtual ~SetupMode();
  void display() const;
  void activated();
  void deactivated();
  void key(int);
  void tick(Real td);
  void mouseDown(int state, int x, int y);

  static SetupMode *init();
  static void cleanup();

 private:
  void start();
  void levelSetChanged();

  class Gamer *gamer;
  int levelSet, level, name;
  SDL_Surface *background;
  double t;

  GLuint texture;
  GLfloat texCoord[4];

  GLuint screenshot;
  GLfloat scrshtCoord[4];
  GLint scrshtW, scrshtH;
};

#endif
