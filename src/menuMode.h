/* menuMode.h
   The inital menu with game options and highscores

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

#ifndef MENUMODE_H
#define MENUMODE_H

class MenuMode : public GameMode {
 public:
  MenuMode();
  void display();
  void key(int);
  void idle(Real);
  void activated();
  void deactivated();
  void mouseDown(int state,int x,int y);

  static void init();
  static MenuMode *menuMode;
 private:
  void doSelection();
  double offset;
  double slideTime;

  /** Moves old slide2 as new slide1 and loads a new random slide as
      slide2 */
  void loadSlide();
  void drawSlide(int slide,double time);

  /** The texture objects used for slide1/2 */
  GLuint slides[2];

  /** The texture coordinates limiting the slides */
  GLfloat slideMin[2][2], slideMax[2][2];

  /** What kind of movement is done on each slide */
  int slideMode[2];
};


#endif
