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

#include "general.h"
#include "gameMode.h"
#include "enterHighScoreMode.h"
#include "glHelp.h"
#include "game.h"
#include "player.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"
#include "menuMode.h"
#include "ctype.h"
#include "highScore.h"
#include "gamer.h"

using namespace std;

EnterHighScoreMode *EnterHighScoreMode::enterHighScoreMode;
SDL_Surface *EnterHighScoreMode::background;

void EnterHighScoreMode::init() {
  char str[256];
  enterHighScoreMode = new EnterHighScoreMode();
  if (low_memory)
    background = NULL;
  else {
    snprintf(str, sizeof(str), "%s/images/enterHighscoreBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf("Error: failed to load %s\n", str);
      exit(0);
    }
  }
}
EnterHighScoreMode::EnterHighScoreMode() {
  //  snprintf(name,20,getenv("USER"));
}
void EnterHighScoreMode::display() {
  int w, h;
  char str[256];
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0, 1.0, 1.0);

  Enter2DMode();
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(0, 0);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(screenWidth, 0);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(0, screenHeight);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(screenWidth, screenHeight);
  glEnd();
  Leave2DMode();

  snprintf(str, sizeof(str), "You got %d points", Game::current->player1->score);
  TTF_SizeText(menuFont, str, &w, &h);
  h += 5;
  draw2DString(menuFont, str, (screenWidth - w) / 2, (screenHeight - 2 * h) / 2 + 0 * h, 220,
               220, 64);
  snprintf(str, sizeof(str), "Enter your name: %s", name);
  TTF_SizeText(menuFont, str, &w, &h);
  h += 5;
  draw2DString(menuFont, str, (screenWidth - w) / 2, (screenHeight - 2 * h) / 2 + 1 * h, 220,
               220, 64);

  /*
  int x=350;
  if(screenWidth <= 640) x=300;
  sprintf(str," %s",name);
  draw2DString(menuFont,str,screenWidth/2-x,screenHeight/2, 220, 220, 64);
  sprintf(str,"%d points",Game::current->player1->score);
  TTF_SizeText(menuFont,str,&w,&h); h += 5;
  draw2DString(menuFont,str,screenWidth/2+x-w,screenHeight/2, 220, 220, 64);
  */
  displayFrameRate();
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

  if (isprint(key)) {
    if (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) key = toupper(key);
    name[len] = (char)key;
    name[len + 1] = 0;
  }
}
void EnterHighScoreMode::activated() {
  char str[256];
  if (!background) {
    snprintf(str, sizeof(str), "%s/images/enterHighscoreBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf("Error: failed to load %s\n", str);
      exit(0);
    }
  }

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
