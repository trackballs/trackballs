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

#include "calibrateJoystickMode.h"

#include "font.h"
#include "glHelp.h"
#include "menusystem.h"
#include "settings.h"
#include "settingsMode.h"

#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_keycode.h>

static CalibrateJoystickMode *calibrateJoystickMode = NULL;

CalibrateJoystickMode::CalibrateJoystickMode() { stage = 0; }

CalibrateJoystickMode *CalibrateJoystickMode::init() {
  if (!calibrateJoystickMode) calibrateJoystickMode = new CalibrateJoystickMode();
  return calibrateJoystickMode;
}
void CalibrateJoystickMode::cleanup() {
  if (calibrateJoystickMode) delete calibrateJoystickMode;
}

void CalibrateJoystickMode::activated() {
  stage = 0;
  clearKeyboardFocus();
}

void CalibrateJoystickMode::deactivated() {}

void CalibrateJoystickMode::tick(Real /*t*/) {
  if (!Settings::settings->hasJoystick()) return;
  int pressed = SDL_JoystickGetButton(Settings::settings->joystick, 0) ||
                SDL_JoystickGetButton(Settings::settings->joystick, 1) ||
                SDL_JoystickGetButton(Settings::settings->joystick, 2) ||
                SDL_JoystickGetButton(Settings::settings->joystick, 3);
  static int wasPressed = 0;
  if (pressed && !wasPressed) nextStage();
  wasPressed = pressed;
}

void CalibrateJoystickMode::nextStage() {
  Settings *settings = Settings::settings;
  int joyX = Settings::settings->joystickRawX();
  int joyY = Settings::settings->joystickRawY();
  /* TODO. Repair joystick */
  switch (stage) {
  case 0:
    settings->joy_center[0] = joyX;
    settings->joy_center[1] = joyY;
    break;
  case 1:
    settings->joy_left = joyX;
    break;
  case 2:
    settings->joy_right = joyX;
    break;
  case 3:
    settings->joy_up = joyY;
    break;
  case 4:
    settings->joy_down = joyY;
    break;
  }

  stage++;
  if (stage > 4) {
    /* TODO - change settings */
    GameMode::activate(SettingsMode::init());
  }
}

void CalibrateJoystickMode::mouseDown(int /*state*/, int /*x*/, int /*y*/) {}

void CalibrateJoystickMode::display() {
  char str[512];
  Enter2DMode();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);  // | GL_DEPTH_BUFFER_BIT);

  if (!Settings::settings->joystickIndex) {
    error("called CalibrateJoystickMode without valid joystick!");
  }

  clearSelectionAreas();

  int headerSize = computeHeaderSize();
  int menusize = computeMenuSize();
  int linesize = computeLineSize();
  addText_Center(0, headerSize, screenHeight / 2 - 100 - headerSize * 1 - menusize * 4,
                 _("Calibrating Joystick"), screenWidth / 2);

  snprintf(str, sizeof(str), "%s",
           SDL_JoystickNameForIndex(Settings::settings->joystickIndex - 1));
  addText_Center(0, menusize, screenHeight / 2 - 100 - menusize * 3, str, screenWidth / 2);

  char *stages[5] = {_("Center joystick"), _("Move joystick left"), _("Move joystick right"),
                     _("Move joystick up"), _("Move joystick down")};

  addText_Center(0, menusize, screenHeight / 2 - 100 - menusize * 1, stages[stage],
                 screenWidth / 2);

  addText_Center(0, menusize, screenHeight / 2 + 100 + menusize * 1,
                 _("Press any button to continue"), screenWidth / 2);
  addText_Center(0, menusize, screenHeight / 2 + 100 + menusize * 3,
                 _("Press escape to cancel"), screenWidth / 2);

  snprintf(str, sizeof(str), _("now: %d %d"), Settings::settings->joystickRawX(),
           Settings::settings->joystickRawY());
  addText_Center(0, linesize, screenHeight / 2 + 100 + menusize * 4 + linesize * 1, str,
                 screenWidth / 2);

  snprintf(str, sizeof(str), _("center: %d %d left: %d right: %d up: %d down: %d"),
           Settings::settings->joy_center[0], Settings::settings->joy_center[1],
           Settings::settings->joy_left, Settings::settings->joy_right,
           Settings::settings->joy_up, Settings::settings->joy_down);
  addText_Center(0, linesize, screenHeight / 2 + 100 + menusize * 4 + linesize * 3, str,
                 screenWidth / 2);

  draw2DRectangle(screenWidth / 2 - 85, screenHeight / 2 - 85, 170, 170, 0., 0., 1., 1.,
                  Color(SRGBColor(0.3, 0.3, 0.9, 1.0)));

  double jx = Settings::settings->joystickX();
  double jy = Settings::settings->joystickY();
  int x = (int)(screenWidth / 2 + 50.0 * jx);
  int y = (int)(screenHeight / 2 + 50.0 * jy);

  drawMouse(x, y, 64, 64);
  Leave2DMode();
}

void CalibrateJoystickMode::key(int key) {
  if (key == SDLK_ESCAPE) GameMode::activate(SettingsMode::init());
  if (key == ' ') nextStage();
}
