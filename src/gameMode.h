/* gameMode.h
   A gameMode describes the current state of the game, such as the
   load mission screen, at a mission, at base etc... When a gamemode
   is active it is used for the rendering of the display and it receives
   all input events. Gamemodes can activate other gamemode's and/or return
   to previous gamemodes. (Use with care - can be unintuitive to users)

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


#ifndef GAMEMODE_H
#define GAMEMODE_H

class GameMode {
 public:
  GameMode();
  virtual ~GameMode();

  virtual void display();
  virtual void key(int);
  virtual void keyUp(int);
  virtual void special(int,int,int);
  virtual void idle(Real dt);
  virtual void doExpensiveComputations();
  virtual void mouse(int state,int x,int y);
  virtual void mouseDown(int button,int x,int y);
  virtual void activated();
  virtual void deactivated();

  static void activate(GameMode*);
  static GameMode *current;
 protected:
  int keyUpReceived;
 private:
};

#endif
