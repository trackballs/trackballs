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

#include "general.h"
#include "gameMode.h"
#include "menuMode.h"
#include "glHelp.h"
#include "SDL2/SDL_image.h"
#include "highScore.h"
#include "hofMode.h"
#include "settings.h"
#include "font.h"
#include "menusystem.h"

using namespace std;

HallOfFameMode *HallOfFameMode::hallOfFameMode;
SDL_Surface *HallOfFameMode::background;

#define CODE_LEVELSET 1

void HallOfFameMode::init() {
  char str[256];
  if (low_memory)
    background = NULL;
  else {
    snprintf(str, sizeof(str), "%s/images/displayHighscoreBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf("Error: failed to load %s\n", str);
      exit(0);
    }
  }
  hallOfFameMode = new HallOfFameMode();
}
HallOfFameMode::HallOfFameMode() { levelSet = 0; }
void HallOfFameMode::activated() {
  char str[256];

  if (!background) {
    snprintf(str, sizeof(str), "%s/images/displayHighscoreBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf("Error: failed to load %s\n", str);
      exit(0);
    }
  }

  /* Loads the background images. */
  GLfloat texcoord[4];
  texture = LoadTexture(background, texcoord);
  texMaxX = texcoord[0];
  texMinY = texcoord[1];
  texMinX = texcoord[2];
  texMaxY = texcoord[3];
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
  int w, h;

  // Draw the background using the preloaded texture
  glColor3f(timeLeft, timeLeft, timeLeft);

  Enter2DMode();
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(0, 0);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(screenWidth, 0);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(0, screenHeight);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(screenWidth, screenHeight);
  glEnd();
  Leave2DMode();

  int y, dy, size, x, y2;
  if (screenWidth <= 640)
    x = 300;
  else if (screenWidth <= 800)
    x = 350;
  else if (screenWidth <= 1024)
    x = 400;
  else
    x = 450;
  if (screenHeight <= 480) {
    y = 160;
    y2 = 150;
    dy = 32;
    size = 16;
  } else if (screenHeight <= 600) {
    y = 200;
    y2 = 180;
    dy = 40;
    size = 20;
  } else if (screenHeight <= 768) {
    y = 280;
    y2 = 200;
    dy = 52;
    size = 24;
  } else {
    y = 300;
    y2 = 200;
    dy = 58;
    size = 24;
  }

  int fontSize = size * 2;
  int col0 = screenWidth / 2 - x;  // screenWidth/2 - 400;
  int col1 = screenWidth / 2 + x + fontSize / 2;
  Settings *settings = Settings::settings;
  clearSelectionAreas();
  addText_Left(0, fontSize / 2, y - dy * 2, _("Level Set"), col0);
  addText_Right(CODE_LEVELSET, fontSize / 2, y - dy * 2, settings->levelSets[levelSet].name,
                col1);

  // sprintf(str,_("Level Set: %s"),settings->levelSets[levelSet].name);
  // TTF_SizeText(menuFont,str,&w,&h);
  // draw2DString(menuFont,str,screenWidth/2 - w/2,y-dy,0,0,0);
  // Font::drawCenterSimpleText(0,str,screenWidth/2,(int)(y-dy-size*1.5),size*0.9,size,1.,1.,1.,1.);

  HighScore *highscore = HighScore::highScore;

  for (int i = 0; i < 10; i++) {
    // draw2DString(menuFont,&highscore->names[levelSet][i][0],screenWidth/2 -
    // x,y+dy*i,220,220,64);
    Font::drawSimpleText(0, &highscore->names[levelSet][i][0], screenWidth / 2 - x,
                         y + dy * i - size, size * 0.9, size, 220 / 256., 220 / 256.,
                         64 / 256., 1.0);
    snprintf(str, sizeof(str), _("%d points"), highscore->points[levelSet][i]);
    Font::drawRightSimpleText(0, str, screenWidth / 2 + x + size / 2, y + dy * i - size,
                              size * 0.9, size, 220 / 256., 220 / 256., 64 / 256., 1.0);
    // TTF_SizeText(menuFont,str,&w,&h);
    // draw2DString(menuFont,str,screenWidth/2 + x - w,y+dy*i,220,220,64);
  }

  drawMousePointer();
  displayFrameRate();
}
void HallOfFameMode::key(int key) {
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
}
void HallOfFameMode::idle(Real td) {
  int x, y;
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

  if (isExiting)
    timeLeft = max(0.0, timeLeft - td);
  else
    timeLeft = min(1.0, timeLeft + td);
  if (timeLeft <= 0.0) MenuMode::activate(MenuMode::menuMode);
  tickMouse(td);
}
void HallOfFameMode::mouseDown(int state, int mouseX, int mouseY) {
  int selection = getSelectedArea();
  if (selection == CODE_LEVELSET) {
    levelSet = mymod((levelSet + (state == 1 ? 1 : -1)), Settings::settings->nLevelSets);
  } else {
    isExiting = 1;
    timeLeft = 1.0;
  }
}
