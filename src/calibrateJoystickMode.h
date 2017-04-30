/* calibrateJoystickMode.h
   Used to calibrate the joystick

   Copyright (C) 2003  Mathias Broxvall

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

#ifndef CALIBRATEJOYSTICK_H
#define CALIBRATEJOYSTICK_H

class CalibrateJoystickMode : public GameMode {
 public:
  CalibrateJoystickMode();
  void display();
  void activated();
  void deactivated();
  void idle(Real);
  void mouseDown(int state,int x,int y);
  void key(int);

  static void init();
  static CalibrateJoystickMode *calibrateJoystickMode;
 private:
  void nextStage();

  int stage;
};

#endif
