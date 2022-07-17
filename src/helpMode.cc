/* HelpMode.cc
   Displayes the help screen

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

#include "helpMode.h"

#include "font.h"
#include "game.h"
#include "gamer.h"
#include "mainMode.h"
#include "map.h"
#include "menuMode.h"
#include "menusystem.h"
#include "settings.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_surface.h>

static HelpMode *helpMode = NULL;

#define CODE_BACK 1
#define CODE_MOREHELP 2

HelpMode *HelpMode::init() {
  if (!helpMode) helpMode = new HelpMode();
  return helpMode;
}
void HelpMode::cleanup() {
  if (helpMode) delete helpMode;
}
HelpMode::HelpMode() {
  timeLeft = 0.;
  isExiting = 0;
  page = 0;
  helpGame = NULL;
  if (low_memory)
    background = NULL;
  else { background = loadImage("helpBackground.jpg"); }
}
HelpMode::~HelpMode() {
  if (background) SDL_FreeSurface(background);
}
void HelpMode::activated() {
  if (!background) { background = loadImage("helpBackground.jpg"); }

  /* Loads the background images. */
  bgTexture = LoadTexture(background, bgCoord);

  helpGame = new Game("help", NULL);

  clearKeyboardFocus();

  isExiting = 0;
  timeLeft = 1.0;
  page = 0;
}
void HelpMode::deactivated() {
  glDeleteTextures(1, &bgTexture);

  if (low_memory && background) {
    SDL_FreeSurface(background);
    background = NULL;
  }
  delete helpGame;
}
void HelpMode::display() {
  // Draw the background using the preloaded texture
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  Enter2DMode();
  draw2DRectangle(0, 0, screenWidth, screenHeight, bgCoord[0], bgCoord[1], bgCoord[2],
                  bgCoord[3], Color(1., 1., 1., 1.), bgTexture);
  Leave2DMode();

  /* for shadow map */
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  /* Some standard GL settings needed */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  /* Debugging for problems with some cards */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* Setup how we handle textures based on gfx_details */
  if (Settings::settings->gfx_details == 5) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  if (helpGame->fogThickness)
    glClearColor(helpGame->fogColor[0], helpGame->fogColor[1], helpGame->fogColor[2],
                 helpGame->fogColor[3]);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);

  if (helpGame->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    activeView.fog_enabled = 1;
    assign(helpGame->fogColor, activeView.fog_color);
    activeView.fog_start = std::max(0.0, 14.0 - 7.0 * helpGame->fogThickness);
    activeView.fog_end = 26.0 - 4.0 * helpGame->fogThickness;
  } else
    activeView.fog_enabled = 0;
  MainMode::setupLighting(NULL, false);

  /* Render two views of the level map */
  perspectiveMatrix(25, (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1), 0.1, 200,
                    activeView.projection);

  /* Setup matrixes for the camera perspective */
  GLdouble step = 16.;
  Coord3d cameraFrom1(0. + step * page, 0., -1.);
  Coord3d cameraTo1(5. + step * page, 5., -8.);
  Coord3d cameraFrom2(0. + step * page, 0. + step, -1.);
  Coord3d cameraTo2(5. + step * page, 5. + step, -8.);
  lookAtMatrix(cameraFrom1[0], cameraFrom1[1], cameraFrom1[2], cameraTo1[0], cameraTo1[1],
               cameraTo1[2], 0.0, 0.0, 1.0, activeView.modelview);
  markViewChanged();
  if (Settings::settings->doShadows) {
    renderShadowCascade(cameraTo1, helpGame->map, helpGame);
  } else {
    renderDummyShadowCascade();
  }
  renderDummyShadowMap();

  glViewport(11 * screenWidth / 20, 11 * screenHeight / 20, 4 * screenWidth / 10,
             4 * screenHeight / 10);
  Coord3d focus_a(step / 2 + step * page, step / 2, 2);
  helpGame->map->draw(0, focus_a, helpGame->gameTime);
  helpGame->draw();
  helpGame->map->draw(1, focus_a, helpGame->gameTime);

  lookAtMatrix(cameraFrom2[0], cameraFrom2[1], cameraFrom2[2], cameraTo2[0], cameraTo2[1],
               cameraTo2[2], 0.0, 0.0, 1.0, activeView.modelview);

  markViewChanged();
  if (Settings::settings->doShadows) {
    renderShadowCascade(cameraTo1, helpGame->map, helpGame);
  } else {
    renderDummyShadowCascade();
  }
  renderDummyShadowMap();
  glViewport(11 * screenWidth / 20, 1 * screenHeight / 10, 4 * screenWidth / 10,
             4 * screenHeight / 10);
  Coord3d focus_b(step / 2 + step * page, 3 * step / 2, 2);
  helpGame->map->draw(0, focus_b, helpGame->gameTime);
  helpGame->draw();
  helpGame->map->draw(1, focus_b, helpGame->gameTime);

  glViewport(0, 0, screenWidth, screenHeight);
  Enter2DMode();

  const char *text[] = {_("Your marble:"),
                        _("Steer this with your mouse, numpad or arrows. "),
                        _("Left mouse or spacebar is jump."),
                        _("Shift gives top speed when using keyboard."),
                        NULL,
                        _("Mr Black:"),
                        _("Your enemy. Defeat with Modpill - Spike."),
                        NULL,
                        _("Modpill Speed:"),
                        _("Makes you go faster for short period of time."),
                        NULL,
                        _("Modpill Spikes:"),
                        _("Weapon against Mr Black and grip for icy terrain."),
                        NULL,

                        _("Modpill Freeze:"),
                        _("This will freeze you - avoid it."),
                        NULL,
                        _("Modpill Glass:"),
                        _("Protects you from acid, but makes you more fragile."),
                        NULL,
                        _("Modpill Nitro:"),
                        _("Move really fast. Active nitro with second button."),
                        NULL,
                        _("Switch:"),
                        _("Activates/deactivates some object/element in the level."),
                        NULL,

                        _("Moving spike:"),
                        _("Obstacle to avoid."),
                        NULL,
                        _("Flag:"),
                        _("Collect for bonus points."),
                        NULL,

                        _("Normal terrain:"),
                        _("Can be of any color and is safe to move on."),
                        NULL,
                        _("Acid terrain:"),
                        _("Will kill your marble unless you have a glass modpill."),
                        NULL,

                        _("Icy terrain:"),
                        _("Slippery terrain unless you have a spikes modpill."),
                        NULL,
                        _("Sandy terrain:"),
                        _("Soft to land on but will slow you down."),
                        NULL,

                        _("Goal:"),
                        _("Will take you to the next level."),
                        _("Get here before time runs out."),
                        NULL,

                        _("Force Field:"),
                        _("You cannot pass this unless you switch it off."),
                        _("Some are lethal and some are one way."),
                        NULL,

                        _("Cactus:"),
                        _("Spiky plant, lethal unless you have a spikes modpill."),
                        NULL,

                        _("Bird:"),
                        _("Kills balls it flies across unless they have spikes."),
                        NULL,

                        _("Teleport:"),
                        _("Will send your marble somewhere else."),
                        NULL,

                        _("Tunnel:"),
                        _("Takes you places, sometimes with great speed."),
                        NULL,
                        NULL};

  int s1 = -1, s2 = -1;
  for (int i = 0, j = 0; text[i]; i++, j++) {
    if (j == 2 * page) { s1 = i; }
    if (j == 2 * page + 1) { s2 = i; }
    while (text[i]) i++;
  }

  int fontSize = computeLineSize();
  if (s1 >= 0) {
    for (int i = s1; text[i]; i++) {
      drawSimpleText(text[i], 50, 1 * screenHeight / 10 + (i - s1) * 3 * fontSize + fontSize,
                     fontSize, Color(SRGBColor(0.9, 0.9, 0.25, 1.)), screenWidth / 2 - 50);
    }
  }

  if (s2 >= 0) {
    for (int i = s2; text[i]; i++) {
      drawSimpleText(text[i], 50, 6 * screenHeight / 10 + (i - s2) * 3 * fontSize + fontSize,
                     fontSize, Color(SRGBColor(0.9, 0.9, 0.25, 1.)), screenWidth / 2 - 50);
    }
  }

  clearSelectionAreas();
  int menusize = computeMenuSize();
  addText_Right(CODE_MOREHELP, menusize, screenHeight - 5 * menusize / 2, _("More Help"),
                screenWidth - 10 - menusize);
  addText_Left(CODE_BACK, menusize, screenHeight - 5 * menusize / 2, _("Back"), 10 + menusize);

  drawMousePointer();
  displayFrameRate();

  Leave2DMode();
}
void HelpMode::key(int key) {
  int shift = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
  if (key == SDLK_TAB) { moveKeyboardFocus(shift); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)
    mouseDown(shift ? 3 : 1, -1, -1);
  if (key == SDLK_ESCAPE) GameMode::activate(MenuMode::init());
}
void HelpMode::tick(Real td) {
  helpGame->tick(td);
  tickMouse(td);

  if (isExiting)
    timeLeft -= td;
  else
    timeLeft = std::min(1.0, timeLeft + td);
  if (timeLeft < 0.0) GameMode::activate(MenuMode::init());
}
void HelpMode::mouseDown(int button, int /*x*/, int /*y*/) {
  int selected = getSelectedArea();

  if (selected == CODE_MOREHELP) {
    int n = 10;
    page = (page + n + (button == 1 ? 1 : -1)) % n;
  } else if (selected == CODE_BACK) {
    isExiting = 1;
  }
}
