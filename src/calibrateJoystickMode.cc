/* calibrateJoystickMode.cc
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

#include "general.h"
#include "gameMode.h"
#include "calibrateJoystickMode.h"
#include "settingsMode.h"
#include "font.h"
#include "settings.h"
#include "menuMode.h"
#include "settings.h"

using namespace std;

CalibrateJoystickMode *CalibrateJoystickMode::calibrateJoystickMode;

CalibrateJoystickMode::CalibrateJoystickMode() {
}

void CalibrateJoystickMode::init() {
  calibrateJoystickMode = new CalibrateJoystickMode();
}

void CalibrateJoystickMode::activated() {
  stage=0;
}

void CalibrateJoystickMode::deactivated() {
}

void CalibrateJoystickMode::idle(Real t) {
  if(!Settings::settings->hasJoystick()) return;
  int pressed=SDL_JoystickGetButton(Settings::settings->joystick,0) || 
	SDL_JoystickGetButton(Settings::settings->joystick,1) || 
	SDL_JoystickGetButton(Settings::settings->joystick,2) || 
	SDL_JoystickGetButton(Settings::settings->joystick,3);
  static int wasPressed=0;
  if(pressed && !wasPressed) nextStage();
  wasPressed=pressed;
}

void CalibrateJoystickMode::nextStage() {
  Settings *settings=Settings::settings;
  int joyX=Settings::settings->joystickRawX();
  int joyY=Settings::settings->joystickRawY();
  /* TODO. Repair joystick */
  switch(stage) {
  case 0: settings->joy_center[0]=joyX; settings->joy_center[1]=joyY; break;
  case 1: settings->joy_left=joyX; break;
  case 2: settings->joy_right=joyX; break;
  case 3: settings->joy_up=joyY; break;
  case 4: settings->joy_down=joyY; break;
  }

  stage++;
  if(stage > 4) {
	/* TODO - change settings */
	GameMode::activate(SettingsMode::settingsMode);
  }
}

void CalibrateJoystickMode::mouseDown(int state,int x,int y) {
  //nextStage();
}

void CalibrateJoystickMode::display() {
  char str[512];
  glPushAttrib(GL_ENABLE_BIT);
  Enter2DMode();
  glColor4f(1.0,1.0,1.0,1.0);
  glClearColor(0.0,0.0,0.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT); // | GL_DEPTH_BUFFER_BIT);
 
  if(!Settings::settings->joystickIndex) { printf(_("Error, called CalibrateJoystickMode without valid joystick!\n")); exit(0); }

  Font::drawCenterSimpleText(0,_("Calibrating Joystick"),screenWidth/2,screenHeight/2-230, 20., 20., 1.0, 1.0, 1.0, 1.0);
  snprintf(str,sizeof(str),"%s",SDL_JoystickName(Settings::settings->joystickIndex-1));
  Font::drawCenterSimpleText(0,str,screenWidth/2,screenHeight/2-190, 16., 16., 1.0, 1.0, 1.0, 1.0);

  switch(stage) {
  case 0:
	  Font::drawCenterSimpleText(0,_("Center joystick"),screenWidth/2,screenHeight/2-120, 20., 20., 1.0, 1.0, 1.0, 1.0);
	  break;
  case 1:
	  Font::drawCenterSimpleText(0,_("Move joystick left"),screenWidth/2,screenHeight/2-120, 20., 20., 1.0, 1.0, 1.0, 1.0);
	  break;
  case 2:
	  Font::drawCenterSimpleText(0,_("Move joystick right"),screenWidth/2,screenHeight/2-120, 20., 20., 1.0, 1.0, 1.0, 1.0);
	  break;
  case 3:
	  Font::drawCenterSimpleText(0,_("Move joystick up"),screenWidth/2,screenHeight/2-120, 20., 20., 1.0, 1.0, 1.0, 1.0);
	  break;
  case 4:
	  Font::drawCenterSimpleText(0,_("Move joystick down"),screenWidth/2,screenHeight/2-120, 20., 20., 1.0, 1.0, 1.0, 1.0);
	  break;
  }
  Font::drawCenterSimpleText(0,_("Press any button to continue"),screenWidth/2,screenHeight/2+190, 16., 16., 1.0, 1.0, 1.0, 1.0);
  Font::drawCenterSimpleText(0,_("Press escape to cancel"),screenWidth/2,screenHeight/2+220, 16., 16., 1.0, 1.0, 1.0, 1.0);

  snprintf(str,sizeof(str),_("now: %d %d"),Settings::settings->joystickRawX(),Settings::settings->joystickRawY());
  Font::drawCenterSimpleText(0,str,screenWidth/2,screenHeight/2+300, 12., 12., 1.0, 1.0, 1.0, 0.5);

  snprintf(str,sizeof(str),_("center: %d %d left: %d right: %d up: %d down: %d"),Settings::settings->joy_center[0],
		  Settings::settings->joy_center[1],Settings::settings->joy_left,Settings::settings->joy_right,
		  Settings::settings->joy_up,Settings::settings->joy_down);
  Font::drawCenterSimpleText(0,str,screenWidth/2,screenHeight/2+320, 12., 12., 1.0, 1.0, 1.0, 0.5);

  glColor4f(0.1,0.1,0.4,0.5);  
  int CX=screenWidth/2, CY=screenHeight/2;
  glBegin(GL_POLYGON);
  glVertex2i(CX-100,CY-100);
  glVertex2i(CX+100,CY-100);
  glVertex2i(CX+100,CY+100);
  glVertex2i(CX-100,CY+100);
  glEnd();
  glColor4f(1.0,1.0,1.0,1.0);

  double jx=Settings::settings->joystickX();
  double jy=Settings::settings->joystickY();
  int x=(int) (screenWidth/2 + 50.0 * jx);
  int y=(int) (screenHeight/2 + 50.0 * jy);
  
  drawMouse(x-32,y-32,64,64,0.0);
  //drawSurface(mousePointer,x-32,y-32,64,64);
  Leave2DMode();
}

void CalibrateJoystickMode::key(int key) {
  if(key == SDLK_ESCAPE) GameMode::activate(SettingsMode::settingsMode);
  if(key == ' ') nextStage();
}

