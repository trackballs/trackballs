/* settingsMode.h
   Allows the user to change game settings

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

#ifndef SETTINGSMODE_H
#define SETTINGSMODE_H

#include "gameMode.h"

class SettingsMode : public GameMode {
 public:
  SettingsMode();
  void display() const;
  void activated();
  void deactivated();
  void key(int);
  void tick(Real);
  void mouseDown(int state, int x, int y);

  static SettingsMode *init();
  static void cleanup();

 private:
  enum eSubScreen {
    SUBSCREEN_VIDEO = 0,
    SUBSCREEN_GRAPHICS,
    SUBSCREEN_CONTROLS,
    SUBSCREEN_OTHER,
    NUM_SUBSCREENS
  };

  enum eSettingsMenu {
    MENU_SUBSCREEN = 1,
    MENU_RETURN,

    MENU_RESOLUTION,
    MENU_APPLY_RESOLUTION,
    MENU_WINDOWED,
    MENU_VSYNC,
    MENU_SHOW_FPS,

    MENU_GFX_DETAILS,
    MENU_DO_REFLECTIONS,
    MENU_DO_SHADOWS,
    MENU_SHADOW_TEXSIZE,

    MENU_USEMOUSE,
    MENU_SENSITIVITY,
    MENU_STEERING,
    MENU_JOYSTICK,

    MENU_MUSIC_VOLUME,
    MENU_SFX_VOLUME,
    MENU_LANGUAGE,
    MENU_SANDBOX_AVAILABLE,
    MENU_TIME_COMPRESSION,
    MENU_STORE_REPLAY,

    MENU_RESOLUTION_OK,
    MENU_RESOLUTION_BAD,

  };

  int resolution, colorDepth; /* The resolution/depth displayed in the window */
  int restoreResolution,
      restoreColorDepth; /* What the settings->res/depth will be set to when exiting */
  int selected;

  double testingResolution; /* > 0 when we are testing a new resolution */

  enum eSubScreen subscreen;
};

#endif
