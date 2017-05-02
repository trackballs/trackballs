/* settingsMode.cc
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

#include "general.h"
#include "gameMode.h"
#include "menuMode.h"
#include "glHelp.h"
#include "SDL2/SDL_image.h"
#include "settingsMode.h"
#include "game.h"
#include "settings.h"
#include "menuMode.h"
#include "sound.h"
#include "font.h"
#include "calibrateJoystickMode.h"
#include "menusystem.h"

using namespace std;

// SDL_Surface *SettingsMode::title;
SettingsMode *SettingsMode::settingsMode;

/* Not properly abstracted, part of global stuff in mmad.cc */
extern int screenResolutions[5][2], nScreenResolutions;
extern void changeScreenResolution();

void SettingsMode::init() {
  char str[256];
  int i;

  settingsMode = new SettingsMode();
}
SettingsMode::SettingsMode() {}
void SettingsMode::activated() {
  Settings *settings = Settings::settings;

  resolution = settings->resolution;
  colorDepth = settings->colorDepth;
  restoreResolution = settings->resolution;
  restoreColorDepth = settings->colorDepth;
  testingResolution = 0.0;
  subscreen = SUBSCREEN_GRAPHICS;
}
void SettingsMode::deactivated() {
  Settings *settings = Settings::settings;

  settings->save();
  regenerateSphereDisplaylists(); /* In case we have changed detail level */

  /* Switch back to the last verified resolution */
  if (settings->resolution != restoreResolution || settings->colorDepth != restoreColorDepth) {
    settings->resolution = restoreResolution;
    settings->colorDepth = restoreColorDepth;
    changeScreenResolution();
  }
}
void SettingsMode::display() {
  int w, h, i;
  int menucount;
  int titleFontSize = 64;
  Settings *settings = Settings::settings;

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);  // | GL_DEPTH_BUFFER_BIT);
  glColor4f(1.0, 1.0, 1.0, 1.0);

  glPushAttrib(GL_ENABLE_BIT);
  Enter2DMode();

  /* Draw title */
  addText_Center(0, titleFontSize / 2, 64, _("Game Settings"), screenWidth / 2);

  // drawSurface(title,(screenWidth-title->w)/2,64,title->w,title->h);

  char str[256];
  int opt;
  glColor3f(1.0, 1.0, 1.0);

  clearSelectionAreas();

  menucount = 0;

  if (testingResolution > 0.0) {
    if (resolution >= 0) {
      snprintf(str, sizeof(str), _("Is resolution %dx%dx%d ok?"), screenWidth, screenHeight,
               colorDepth);
    } else {
      snprintf(str, sizeof(str), _("Is resolution Auto-%d ok?"), colorDepth);
    }
    menuItem_Center(0, menucount++, str);
    menucount++;  // looks better with a small gap
    menuItem_Left(MENU_RESOLUTION_OK, menucount++, _("Ok, use this resolution"));
    menuItem_Left(MENU_RESOLUTION_BAD, menucount++, _("No, revert to old resolution"));
    menucount++;  // looks better with a small gap
    snprintf(str, sizeof(str), _("Timeout in %d seconds"), (int)(testingResolution + 0.5));
    menuItem_Left(0, menucount++, str);

    glEnable(GL_BLEND);
    drawMousePointer();
    displayFrameRate();
    Leave2DMode();
    glPopAttrib();
    return;
  }

  switch (subscreen) {
  case SUBSCREEN_GRAPHICS:
    // Choose subscreen
    menuItem_Left(MENU_SUBSCREEN, menucount++, _("Graphics"));

    // Resolution
    if (resolution >= 0) {
      snprintf(str, sizeof(str), "%dx%dx%d", screenResolutions[resolution][0],
               screenResolutions[resolution][1], colorDepth);
    } else {
      snprintf(str, sizeof(str), _("Auto-%d"), colorDepth);
    }
    menuItem_LeftRight(MENU_RESOLUTION, menucount++, _("  Resolution"), str);
    menuItem_Left(MENU_APPLY_RESOLUTION, menucount++, _("  Test this resolution"));

    // Windowed
    menuItem_LeftRight(MENU_WINDOWED, menucount++, _("  Fullscreen"),
                       (char *)(Settings::settings->is_windowed ? _("No") : _("Yes")));

    // gfx details
    switch (Settings::settings->gfx_details) {
    case 0:
      snprintf(str, sizeof(str), _("None"));
      break;
    case 1:
      snprintf(str, sizeof(str), _("Minimalistic"));
      break;
    case 2:
      snprintf(str, sizeof(str), _("Simple"));
      break;
    case 3:
      snprintf(str, sizeof(str), _("Standard"));
      break;
    case 4:
      snprintf(str, sizeof(str), _("Extra"));
      break;
    case 5:
      snprintf(str, sizeof(str), _("Everything"));
      break;
    }
    menuItem_LeftRight(MENU_GFX_DETAILS, menucount++, _("  Details"), str);
    if (Settings::settings->doReflections)
      menuItem_LeftRight(MENU_DO_REFLECTIONS, menucount++, _("  Reflections (beta)"),
                         _("Yes"));
    else
      menuItem_LeftRight(MENU_DO_REFLECTIONS, menucount++, _("  Reflections (beta)"), _("No"));

    // show FPS
    menuItem_LeftRight(MENU_SHOW_FPS, menucount++, _("  Show FPS"),
                       (char *)(Settings::settings->showFPS ? _("Yes") : _("No")));

    break;

  case SUBSCREEN_CONTROLS:
    /* Choose subscreen */
    menuItem_Left(MENU_SUBSCREEN, menucount++, _("Controls"));

    /* Use mouse */
    menuItem_LeftRight(MENU_USEMOUSE, menucount++, _("  Use mouse"),
                       (char *)(Settings::settings->ignoreMouse ? _("No") : _("Yes")));

    /* Mouse sensitivity */
    if (!Settings::settings->ignoreMouse) {
      snprintf(str, sizeof(str), "%1.2f", Settings::settings->mouseSensitivity);
      menuItem_LeftRight(MENU_SENSITIVITY, menucount++, _("  Sensitivity"), str);
    }

    /* Steering */
    if (Settings::settings->rotateSteering == 0)
      snprintf(str, sizeof(str), _("normal"));
    else if (Settings::settings->rotateSteering > 0)
      snprintf(str, sizeof(str), _("+%d degrees"), 45 * Settings::settings->rotateSteering);
    else if (Settings::settings->rotateSteering < 0)
      snprintf(str, sizeof(str), _("-%d degrees"), -45 * Settings::settings->rotateSteering);
    menuItem_LeftRight(MENU_STEERING, menucount++, _("  Steering"), str);

    /* Joystick */
    i = Settings::settings->joystickIndex;
    if (i)
      snprintf(str, 255, "%s", SDL_JoystickNameForIndex(i - 1));
    else if (SDL_NumJoysticks() == 0)
      snprintf(str, 255, _("no joystick found"));
    else
      snprintf(str, 255, "no joystick selected (%d found)", SDL_NumJoysticks());

    menuItem_Left(0, menucount++, _("  Joystick:"));
    if (strlen(str) < 15)
      menuItem_LeftRight(MENU_JOYSTICK, menucount - 1, "", str);  // overwrite last
    else
      menuItem_Center(MENU_JOYSTICK, menucount++, str);

    break;

  case SUBSCREEN_OTHER:
    // Choose subscreen
    menuItem_Left(MENU_SUBSCREEN, 0, _("Other"));

    /* Music and Sfx volumes */
    snprintf(str, sizeof(str), "%d%%", (int)(Settings::settings->musicVolume * 100.0));
    menuItem_LeftRight(MENU_MUSIC_VOLUME, 1, _("  Music volume"), str);
    snprintf(str, sizeof(str), "%d%%", (int)(Settings::settings->sfxVolume * 100.0));
    menuItem_LeftRight(MENU_SFX_VOLUME, 2, _("  Effects volume"), str);
    /* Language */
    menuItem_LeftRight(MENU_LANGUAGE, 3, _("  Language"),
                       Settings::languageNames[Settings::settings->language]);

    break;
  }
  menuItem_Left(MENU_RETURN, 10, _("Back"));

  glEnable(GL_BLEND);
  drawMousePointer();
  displayFrameRate();
  Leave2DMode();
  glPopAttrib();
}
void SettingsMode::key(int key) {
  if (key == SDLK_SPACE) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    mouseDown(1, x, y);
  }
  if (key == SDLK_RETURN) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    mouseDown(3, x, y);
  }
  if (key == SDLK_ESCAPE) GameMode::activate(MenuMode::menuMode);
}
void SettingsMode::idle(Real td) {
  int x, y;
  Settings *settings = Settings::settings;

  tickMouse(td);

  SDL_GetMouseState(&x, &y);
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  if (keystate[SDL_SCANCODE_LEFT]) {
    x -= (int)(150 / fps);
    SDL_WarpMouseInWindow(window, x, y);
  }
  if (keystate[SDL_SCANCODE_RIGHT]) {
    x += (int)(150 / fps);
    SDL_WarpMouseInWindow(window, x, y);
  }
  if (keystate[SDL_SCANCODE_UP]) {
    y -= (int)(150 / fps);
    SDL_WarpMouseInWindow(window, x, y);
  }
  if (keystate[SDL_SCANCODE_DOWN]) {
    y += (int)(150 / fps);
    SDL_WarpMouseInWindow(window, x, y);
  }

  /* Check against timeouts when testing a new resolution */
  if (testingResolution > 0.0) {
    testingResolution -= td;
    if (testingResolution <= 0.0) {
      testingResolution = 0.0;
      settings->resolution = restoreResolution;
      settings->colorDepth = restoreColorDepth;
      resolution = restoreResolution;
      colorDepth = restoreColorDepth;
      changeScreenResolution();
    }
  }
}

void SettingsMode::mouseDown(int button, int x, int y) {
  int up;
  if (button == 1)
    up = 1;
  else
    up = 0;
  Settings *settings = Settings::settings;

  switch (getSelectedArea()) {
  case MENU_SUBSCREEN:
    subscreen = (SettingsMode::eSubScreen)mymod(subscreen + (up ? 1 : -1), NUM_SUBSCREENS);
    break;
  case MENU_RETURN:
    GameMode::activate(MenuMode::menuMode);
    break;

  case MENU_RESOLUTION:
    // Change screen resolution until we got a valid screen

    resolution = resolution + (up ? 1 : -1);
    if (resolution >= nScreenResolutions) {
      resolution = -1;
      if (colorDepth == 16)
        colorDepth = 32;
      else
        colorDepth = 16;
    } else if (resolution <= -2) {
      resolution = nScreenResolutions - 1;
      if (colorDepth == 16)
        colorDepth = 32;
      else
        colorDepth = 16;
    }
    break;
  case MENU_APPLY_RESOLUTION:
    settings->resolution = resolution;
    settings->colorDepth = colorDepth;
    changeScreenResolution();
    testingResolution = 10.0;
    break;
  case MENU_RESOLUTION_OK:
    testingResolution = 0.0;
    restoreResolution = settings->resolution;
    restoreColorDepth = settings->colorDepth;
    break;
  case MENU_RESOLUTION_BAD:
    testingResolution = 0.0;
    settings->resolution = restoreResolution;
    settings->colorDepth = restoreColorDepth;
    resolution = restoreResolution;
    colorDepth = restoreColorDepth;
    changeScreenResolution();
    break;

  case MENU_WINDOWED:
    Settings::settings->is_windowed = Settings::settings->is_windowed ? 0 : 1;
    changeScreenResolution();
    break;
  case MENU_GFX_DETAILS:
    // Change level of graphic details */
    settings->gfx_details = mymod((settings->gfx_details + (up ? 1 : -1)), 6);
    break;
  case MENU_DO_REFLECTIONS:
    Settings::settings->doReflections = Settings::settings->doReflections ? 0 : 1;
    break;
  case MENU_SHOW_FPS:
    Settings::settings->showFPS = Settings::settings->showFPS ? 0 : 1;
    break;

  case MENU_USEMOUSE:
    Settings::settings->ignoreMouse = Settings::settings->ignoreMouse ? 0 : 1;
    break;
  case MENU_SENSITIVITY:
    Settings::settings->mouseSensitivity += up ? 0.25 : -0.25;
    if (Settings::settings->mouseSensitivity > 5.001)
      Settings::settings->mouseSensitivity = 0.0;
    if (Settings::settings->mouseSensitivity < 0.000)
      Settings::settings->mouseSensitivity = 5.00;
    break;
  case MENU_STEERING:
    settings->rotateSteering = mymod(settings->rotateSteering + 3 + (up ? 1 : -1), 8) - 3;
    break;
  case MENU_JOYSTICK:
    settings->joystickIndex =
        mymod(settings->joystickIndex + (up ? 1 : -1), SDL_NumJoysticks() + 1);
    if (settings->joystickIndex > 0) {
      GameMode::activate(CalibrateJoystickMode::calibrateJoystickMode);
    }
    break;

  case MENU_MUSIC_VOLUME:
    settings->musicVolume =
        mymod(((int)(settings->musicVolume * 10.0)) + (up ? 1 : -1), 11) * 0.1;
    volumeChanged();
    break;
  case MENU_SFX_VOLUME:
    settings->sfxVolume = mymod(((int)(settings->sfxVolume * 10.0)) + (up ? 1 : -1), 11) * 0.1;
    volumeChanged();
    playEffect(SFX_GOT_FLAG);
    break;
  case MENU_LANGUAGE:
    settings->language =
        mymod(((int)(settings->language + (up ? 1 : -1))), Settings::nLanguages);
    settings->setLocale();
    /* TODO. Restart all modules to make language change take effect */
    break;
  }
}
