/* enterHighScoreMode.h
   Lets the user enter his/her name for the highscore list

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

#ifndef ENTERHIGHSCOREMODE_H
#define ENTERHIGHSCOREMODE_H

class EnterHighScoreMode : public GameMode {
 public:
  EnterHighScoreMode();
  void display();
  void key(int);
  void activated();
  void deactivated();

  static void init();
  static EnterHighScoreMode *enterHighScoreMode;
 private:
  char name[20];  
  static SDL_Surface *background;
  GLuint texture;
  GLfloat texMinX, texMinY;
  GLfloat texMaxX, texMaxY;

};


#endif
