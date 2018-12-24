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

#include "settingsMode.h"

#include "calibrateJoystickMode.h"
#include "glHelp.h"
#include "menuMode.h"
#include "menusystem.h"
#include "settings.h"
#include "sound.h"

#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

static SettingsMode *settingsMode = NULL;

/* Not properly abstracted, part of global stuff in mmad.cc */
extern int screenResolutions[5][2], nScreenResolutions;
extern void requestScreenUpdate();

SettingsMode *SettingsMode::init() {
  if (!settingsMode) settingsMode = new SettingsMode();
  return settingsMode;
}
void SettingsMode::cleanup() {
  if (settingsMode) delete settingsMode;
}
SettingsMode::SettingsMode() {
  resolution = 0;
  colorDepth = 0;
  restoreResolution = 0;
  restoreColorDepth = 0;
  testingResolution = 0.;
  selected = 0;
  subscreen = SUBSCREEN_VIDEO;
}
void SettingsMode::activated() {
  Settings *settings = Settings::settings;

  clearKeyboardFocus();

  resolution = settings->resolution;
  colorDepth = settings->colorDepth;
  restoreResolution = settings->resolution;
  restoreColorDepth = settings->colorDepth;
  testingResolution = 0.0;
  subscreen = SUBSCREEN_VIDEO;
}
void SettingsMode::deactivated() {
  Settings *settings = Settings::settings;

  settings->save();

  /* Switch back to the last verified resolution */
  if (settings->resolution != restoreResolution || settings->colorDepth != restoreColorDepth) {
    settings->resolution = restoreResolution;
    settings->colorDepth = restoreColorDepth;
    requestScreenUpdate();
  }
}
void SettingsMode::display() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);  // | GL_DEPTH_BUFFER_BIT);

  Enter2DMode();

  /* Draw title */
  int titleFontSize = computeHeaderSize();
  addText_Center(0, titleFontSize, 64, _("Game Settings"), screenWidth / 2);

  char str[256];

  clearSelectionAreas();

  int menucount = 0;
  Settings *settings = Settings::settings;

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

    drawMousePointer();
    displayFrameRate();
    Leave2DMode();
    return;
  }

  switch (subscreen) {
  case SUBSCREEN_VIDEO:
    // Choose subscreen
    menuItem_Left(MENU_SUBSCREEN, menucount++, _("Video"));

    // Resolution
    if (resolution >= 0) {
      snprintf(str, sizeof(str), "%dx%dx%d", screenResolutions[resolution][0],
               screenResolutions[resolution][1], colorDepth);
    } else {
      snprintf(str, sizeof(str), _("Auto-%d"), colorDepth);
    }
    menuItem_LeftRight(MENU_RESOLUTION, menucount++, 1, _("Resolution"), str);
    menuItem_Left(MENU_APPLY_RESOLUTION, menucount++, _("Test this resolution"));

    // Windowed
    menuItem_LeftRight(MENU_WINDOWED, menucount++, 1, _("Fullscreen"),
                       (settings->is_windowed ? _("No") : _("Yes")));

    menuItem_LeftRight(MENU_VSYNC, menucount++, 1, _("VSync"),
                       (settings->vsynced ? _("Yes") : _("No")));

    // show FPS
    switch (settings->showFPS) {
    case 0:
      snprintf(str, sizeof(str), _("No"));
      break;
    case 1:
      snprintf(str, sizeof(str), _("Frame rate"));
      break;
    case 2:
      snprintf(str, sizeof(str), _("Frame time"));
      break;
    }
    menuItem_LeftRight(MENU_SHOW_FPS, menucount++, 1, _("Show FPS"), str);

    break;

  case SUBSCREEN_GRAPHICS:
    menuItem_Left(MENU_SUBSCREEN, menucount++, _("Graphics"));

    // gfx details
    switch (settings->gfx_details) {
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
    menuItem_LeftRight(MENU_GFX_DETAILS, menucount++, 1, _("Details"), str);
    menuItem_LeftRight(MENU_DO_REFLECTIONS, menucount++, 1, _("Reflections"),
                       (settings->doReflections ? _("Yes") : _("No")));
    menuItem_LeftRight(MENU_DO_SHADOWS, menucount++, 1, _("Shadows"),
                       (settings->doShadows ? _("Yes") : _("No")));
    if (settings->doShadows) {
      snprintf(str, sizeof(str), "%d", 1 << settings->shadowTexsize);
      menuItem_LeftRight(MENU_SHADOW_TEXSIZE, menucount++, 1, _("Shadow Resolution"), str);
    }

    break;

  case SUBSCREEN_CONTROLS:
    /* Choose subscreen */
    menuItem_Left(MENU_SUBSCREEN, menucount++, _("Controls"));

    /* Use mouse */
    menuItem_LeftRight(MENU_USEMOUSE, menucount++, 1, _("Use mouse"),
                       (settings->ignoreMouse ? _("No") : _("Yes")));

    /* Mouse sensitivity */
    if (!settings->ignoreMouse) {
      snprintf(str, sizeof(str), "%1.2f", settings->mouseSensitivity);
      menuItem_LeftRight(MENU_SENSITIVITY, menucount++, 1, _("Sensitivity"), str);
    }

    /* Steering */
    if (settings->rotateSteering == 0)
      snprintf(str, sizeof(str), _("normal"));
    else if (settings->rotateSteering > 0)
      snprintf(str, sizeof(str), _("+%d degrees"), 45 * settings->rotateSteering);
    else if (settings->rotateSteering < 0)
      snprintf(str, sizeof(str), _("-%d degrees"), -45 * settings->rotateSteering);
    menuItem_LeftRight(MENU_STEERING, menucount++, 1, _("Steering"), str);

    /* Joystick */
    if (settings->joystickIndex)
      snprintf(str, 255, "%s", SDL_JoystickNameForIndex(settings->joystickIndex - 1));
    else if (SDL_NumJoysticks() == 0)
      snprintf(str, 255, _("no joystick found"));
    else
      snprintf(str, 255, "no joystick selected (%d found)", SDL_NumJoysticks());

    menuItem_Left(0, menucount++, _("Joystick:"));
    if (strlen(str) < 15)
      menuItem_LeftRight(MENU_JOYSTICK, menucount - 1, 0, "", str);  // overwrite last
    else
      menuItem_Center(MENU_JOYSTICK, menucount++, str);

    break;

  case SUBSCREEN_OTHER:
    // Choose subscreen
    menuItem_Left(MENU_SUBSCREEN, menucount++, _("Other"));

    /* Language */
    menuItem_LeftRight(MENU_LANGUAGE, menucount++, 1, _("Language"),
                       Settings::languageNames[settings->language]);
    /* Music and Sfx volumes */
    snprintf(str, sizeof(str), "%d%%", (int)(settings->musicVolume * 100.0));
    menuItem_LeftRight(MENU_MUSIC_VOLUME, menucount++, 1, _("Music volume"), str);
    snprintf(str, sizeof(str), "%d%%", (int)(settings->sfxVolume * 100.0));
    menuItem_LeftRight(MENU_SFX_VOLUME, menucount++, 1, _("Effects volume"), str);
    menucount++;
    /* Sandbox mode present */
    menuItem_LeftRight(MENU_SANDBOX_AVAILABLE, menucount++, 1, _("Sandbox mode available"),
                       (settings->sandboxAvailable ? _("Yes") : _("No")));
    /* Time compression */
    snprintf(str, sizeof(str), "%.3f", timeDilationFactor);
    menuItem_LeftRight(MENU_TIME_COMPRESSION, menucount++, 1, _("Time Compression"), str);

    menuItem_LeftRight(MENU_STORE_REPLAY, menucount++, 1, _("Store best level replay"),
                       (settings->storeReplay ? _("Yes") : _("No")));

    break;
  case NUM_SUBSCREENS:
    // never should happen
    break;
  }
  addText_Left(MENU_RETURN, computeMenuSize(), screenHeight - 5 * computeMenuSize() / 2,
               _("Back"), computeScreenBorder());

  drawMousePointer();
  displayFrameRate();
  Leave2DMode();
}
void SettingsMode::key(int key) {
  int shift = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
  if (key == SDLK_TAB) { moveKeyboardFocus(shift); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)
    mouseDown(shift ? 3 : 1, -1, -1);
  if (key == SDLK_ESCAPE) GameMode::activate(MenuMode::init());
}
void SettingsMode::tick(Real td) {
  tickMouse(td);

  /* Check against timeouts when testing a new resolution */
  if (testingResolution > 0.0) {
    testingResolution -= td;
    if (testingResolution <= 0.0) {
      testingResolution = 0.0;
      Settings::settings->resolution = restoreResolution;
      Settings::settings->colorDepth = restoreColorDepth;
      resolution = restoreResolution;
      colorDepth = restoreColorDepth;
      requestScreenUpdate();
    }
  }
}

void SettingsMode::mouseDown(int button, int /*x*/, int /*y*/) {
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
    GameMode::activate(MenuMode::init());
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
    requestScreenUpdate();
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
    requestScreenUpdate();
    break;

  case MENU_WINDOWED:
    settings->is_windowed = settings->is_windowed ? 0 : 1;
    requestScreenUpdate();
    break;
  case MENU_GFX_DETAILS:
    // Change level of graphic details */
    settings->gfx_details = mymod((settings->gfx_details + (up ? 1 : -1)), 6);
    break;
  case MENU_DO_REFLECTIONS:
    settings->doReflections = settings->doReflections ? 0 : 1;
    break;
  case MENU_DO_SHADOWS:
    settings->doShadows = settings->doShadows ? 0 : 1;
    break;
  case MENU_SHADOW_TEXSIZE:
    settings->shadowTexsize += (up ? 1 : -1);
    if (settings->shadowTexsize > 12) settings->shadowTexsize = 6;
    if (settings->shadowTexsize < 6) settings->shadowTexsize = 12;
    break;
  case MENU_SHOW_FPS:
    settings->showFPS = (settings->showFPS + 1) % 3;
    break;
  case MENU_VSYNC:
    settings->vsynced = settings->vsynced ? 0 : 1;
    SDL_GL_SetSwapInterval(settings->vsynced ? 1 : 0);
    break;

  case MENU_USEMOUSE:
    settings->ignoreMouse = settings->ignoreMouse ? 0 : 1;
    break;
  case MENU_SENSITIVITY:
    settings->mouseSensitivity += up ? 0.25 : -0.25;
    if (settings->mouseSensitivity > 10.001) settings->mouseSensitivity = 0.0;
    if (settings->mouseSensitivity < 0.000) settings->mouseSensitivity = 10.00;
    break;
  case MENU_STEERING:
    settings->rotateSteering = mymod(settings->rotateSteering + 3 + (up ? 1 : -1), 8) - 3;
    break;
  case MENU_JOYSTICK:
    settings->joystickIndex =
        mymod(settings->joystickIndex + (up ? 1 : -1), SDL_NumJoysticks() + 1);
    if (settings->joystickIndex > 0) { GameMode::activate(CalibrateJoystickMode::init()); }
    break;
  case MENU_STORE_REPLAY:
    settings->storeReplay = !settings->storeReplay;
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
    break;
  case MENU_SANDBOX_AVAILABLE:
    settings->sandboxAvailable = !settings->sandboxAvailable;
    break;
  case MENU_TIME_COMPRESSION:
    settings->timeCompression += (up ? 1 : -1);
    if (settings->timeCompression > 12) settings->timeCompression = -12;
    if (settings->timeCompression < -12) settings->timeCompression = 12;
    timeDilationFactor = std::pow(2.0, settings->timeCompression / 6.0);
    break;
  }
}
