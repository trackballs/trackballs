/* enterHighScoreMode.h
   Lets the user enter his/her name for the highscore list

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

#include "enterHighScoreMode.h"

#include "font.h"
#include "game.h"
#include "gamer.h"
#include "highScore.h"
#include "menuMode.h"
#include "menusystem.h"
#include "player.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_surface.h>
#include <string.h>

EnterHighScoreMode *EnterHighScoreMode::enterHighScoreMode;
SDL_Surface *EnterHighScoreMode::background;

void EnterHighScoreMode::init() {
  enterHighScoreMode = new EnterHighScoreMode();
  if (low_memory)
    background = NULL;
  else {
    background = loadImage("setupBackground.jpg");
  }
}
EnterHighScoreMode::EnterHighScoreMode() { memset(name, 0, sizeof(name)); }
void EnterHighScoreMode::display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Enter2DMode();
  draw2DRectangle(0, 0, screenWidth, screenHeight, texMinX, texMinY, texMaxX, texMaxY, 1., 1.,
                  1., 1., texture);

  int headerSize = computeHeaderSize();
  int fontSize = computeMenuSize();

  clearSelectionAreas();
  addText_Center(0, headerSize, screenHeight / 2 - 2 * headerSize, _("High Score"),
                 screenWidth / 2);

  char str[256];
  snprintf(str, sizeof(str), _("You got %d points"), Game::current->player1->score);
  addText_Center(0, fontSize, screenHeight / 2, str, screenWidth / 2);

  snprintf(str, sizeof(str), _("Enter your name: %s"), name);
  addText_Center(0, fontSize, screenHeight / 2 + 2 * fontSize, str, screenWidth / 2);

  displayFrameRate();
  Leave2DMode();
}
void EnterHighScoreMode::key(int key) {
  int len = strlen(name);
  if (key == SDLK_BACKSPACE) {
    if (len > 0) name[len - 1] = 0;
    return;
  }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
    HighScore::highScore->addHighScore(Game::current->player1->score, name);
    GameMode::activate(MenuMode::menuMode);
    return;
  }
  if (len == 19) /* TODO. Beep */
    return;

  if (key < 127 && isprint(key)) {
    if (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) key = toupper(key);
    name[len] = (char)key;
    name[len + 1] = 0;
  }
}
void EnterHighScoreMode::activated() {
  if (!background) { background = loadImage("enterHighscoreBackground.jpg"); }

  snprintf(name, sizeof(name), "%s", Game::current->gamer->name);
  if (!HighScore::highScore->isHighScore(Game::current->player1->score))
    GameMode::activate(MenuMode::menuMode);

  GLfloat texcoord[4];
  texture = LoadTexture(background, texcoord);
  texMinX = texcoord[0];
  texMinY = texcoord[1];
  texMaxX = texcoord[2];
  texMaxY = texcoord[3];
}
void EnterHighScoreMode::deactivated() {
  glDeleteTextures(1, &texture);
  if (low_memory && background) {
    SDL_FreeSurface(background);
    background = NULL;
  }
}
