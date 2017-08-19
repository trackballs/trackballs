/* helpMode.h
   Displayes the help file

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

#ifndef HELPMODE_H
#define HELPMODE_H

#include "gameMode.h"
#include "glHelp.h"

class HelpMode : public GameMode {
 public:
  HelpMode();
  void display();
  void activated();
  void deactivated();
  void key(int);
  void idle(Real);
  void mouseDown(int button, int x, int y);

  static void init();
  static HelpMode *helpMode;

 private:
  double timeLeft;
  int isExiting, page;
  static SDL_Surface *background;

  Game *helpGame;

  GLuint bgTexture;
  GLfloat bgCoord[4];
};

#endif
