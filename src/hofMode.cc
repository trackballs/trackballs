/** \file hofMode.cc
   Displayes the "Hall of Fame"
*/
/*
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

#include "hofMode.h"

#include "font.h"
#include "highScore.h"
#include "menuMode.h"
#include "menusystem.h"
#include "settings.h"

#include <set>

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_surface.h>

HallOfFameMode *HallOfFameMode::hallOfFameMode;
SDL_Surface *HallOfFameMode::background;

#define CODE_LEVELSET 1
#define CODE_RETURN 2

void HallOfFameMode::init() {
  if (low_memory)
    background = NULL;
  else {
    background = loadImage("displayHighscoreBackground.jpg");
  }
  hallOfFameMode = new HallOfFameMode();
}
HallOfFameMode::HallOfFameMode() {
  levelSet = 0;
  timeLeft = 0.;
  isExiting = 0;
}
void HallOfFameMode::activated() {
  if (!background) { background = loadImage("displayHighscoreBackground.jpg"); }

  clearKeyboardFocus();

  /* Loads the background images. */
  texture = LoadTexture(background, texCoord);
  isExiting = 0;
  timeLeft = 1.0;
}
void HallOfFameMode::deactivated() {
  glDeleteTextures(1, &texture);
  if (low_memory && background) {
    SDL_FreeSurface(background);
    background = NULL;
  }
}
void HallOfFameMode::display() {
  char str[256];

  // Draw the background using the preloaded texture
  Enter2DMode();
  draw2DRectangle(0, 0, screenWidth, screenHeight, texCoord[0], texCoord[1], texCoord[2],
                  texCoord[3], timeLeft, timeLeft, timeLeft, 1., texture);

  int fontSize = computeMenuSize();
  int border = computeScreenBorder();

  int y = fontSize * 10;
  int dy = fontSize * 2;

  Settings *settings = Settings::settings;
  clearSelectionAreas();
  addText_Center(0, computeHeaderSize(), computeHeaderSize(), _("High Scores"),
                 screenWidth / 2);

  addText_Left(0, fontSize, y - dy * 2, _("Level Set"), border);
  addText_Right(CODE_LEVELSET, fontSize, y - dy * 2, settings->levelSets[levelSet].name,
                screenWidth - border);

  HighScore *highscore = HighScore::highScore;

  for (int i = 0; i < 10; i++) {
    const char *name = highscore->dummy_player[levelSet][i]
                           ? _("Anonymous Coward")
                           : &highscore->names[levelSet][i][0];
    Font::drawSimpleText(name, border, y + dy * i - fontSize, fontSize, menuColor[0],
                         menuColor[1], menuColor[2], menuColor[3]);
    snprintf(str, sizeof(str), _("%d points"), highscore->points[levelSet][i]);
    Font::drawRightSimpleText(str, screenWidth - border, y + dy * i - fontSize, fontSize,
                              menuColor[0], menuColor[1], menuColor[2], menuColor[3]);
  }

  addText_Left(CODE_RETURN, computeMenuSize(), screenHeight - 5 * computeMenuSize() / 2,
               _("Back"), computeScreenBorder());
  drawMousePointer();
  displayFrameRate();

  Leave2DMode();
}
void HallOfFameMode::key(int key) {
  int shift = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
  if (key == SDLK_TAB) { moveKeyboardFocus(shift); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)
    mouseDown(shift ? 3 : 1, -1, -1);
  if (key == SDLK_ESCAPE) GameMode::activate(MenuMode::menuMode);
}
void HallOfFameMode::idle(Real td) {
  if (isExiting)
    timeLeft = fmax(0.0, timeLeft - td);
  else
    timeLeft = fmin(1.0, timeLeft + td);
  if (timeLeft <= 0.0) MenuMode::activate(MenuMode::menuMode);
  tickMouse(td);
}
void HallOfFameMode::mouseDown(int state, int /*mouseX*/, int /*mouseY*/) {
  int selection = getSelectedArea();
  if (selection == CODE_LEVELSET) {
    levelSet = mymod((levelSet + (state == 1 ? 1 : -1)), Settings::settings->nLevelSets);
  } else if (selection == CODE_RETURN) {
    isExiting = 1;
    timeLeft = -1.0;
  } else {
    isExiting = 1;
    timeLeft = 1.0;
  }
}
