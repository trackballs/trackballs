/* settings.h
   Contains all user specified settings for the game

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

#ifndef SETTINGS_H
#define SETTINGS_H

#define GFX_DETAILS_NONE       0
#define GFX_DETAILS_MINIMALISTIC 1
#define GFX_DETAILS_SIMPLE     2
#define GFX_DETAILS_NORMAL     3
#define GFX_DETAILS_EXTRA      4
#define GFX_DETAILS_EVERYTHING 5

class LevelSet {
 public:
  char name[256];
  char startLevel[256];
  char startLevelName[256];
  char description[5][80];
  char imagename[256];
};

class Settings {
 public:
  Settings();
  void loadLevelSets();
  void save();
  static void init();
  static Settings *settings;

  double sfxVolume,musicVolume;
  double mouseSensitivity;
  int is_windowed,resolution,colorDepth;
  int gfx_details; /* 0 - 5 */
  int showFPS;
  int difficulty;
  int sandbox;
  int rotateSteering;
  int ignoreMouse;
  int doReflections;

  void setLocale();
  int hasJoystick();
  void closeJoystick();
  int joystickRawX();
  double joystickX();
  int joystickRawY(); 
  double joystickY();
  int joystickButton(int n);

  char specialLevel[256];
  int doSpecialLevel;

  int nLevelSets;
  class LevelSet levelSets[256];
  SDL_Joystick *joystick;
  int joystickIndex,joy_left,joy_right,joy_up,joy_down,joy_center[2];
  int lastJoyX,lastJoyY;

  int language;
  static char *languageNames[], *languageCodes[];
  static int nLanguages;

 private:
  void loadLevelSet(char *name);
};

#endif
