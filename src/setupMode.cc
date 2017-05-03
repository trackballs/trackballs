/* setupMode.cc
   Allows the human to select player name, color, start level etc.

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
#include "setupMode.h"
#include "game.h"
#include "player.h"
#include "mainMode.h"
#include "settings.h"
#include "gamer.h"
#include "ctype.h"
#include "ball.h"
#include "map.h"
#include "hofMode.h"
#include "font.h"
#include "menusystem.h"
#include "sound.h"

#define SETUP_NAME 0
#define NUM_FIELDS 1

using namespace std;

#define Y_BASE (screenHeight / 2 - 30)
#define Y_DELTA 40
#define Y_IMG_DIFF 52

#define CODE_NAME 1
#define CODE_LEVEL_SET 2
#define CODE_START_LV 3
#define CODE_DIFFICULTY 4
#define CODE_START 5
#define CODE_COLOR 6

SetupMode *SetupMode::setupMode = NULL;
SDL_Surface *SetupMode::background;

GLfloat colors[5][3] = {
    {1.0, 0.2, 0.2}, {0.2, 1.0, 0.2}, {0.5, 0.5, 1.0}, {1.0, 1.0, 0.2}, {1.0, 1.0, 1.0}};

void SetupMode::init() {
  char str[256];
  int i;

  if (low_memory)
    background = NULL;
  else {
    snprintf(str, sizeof(str), "%s/images/setupBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf(_("Error: Failed to load %s\n"), str);
      exit(0);
    }
  }

  setupMode = new SetupMode();
}
SetupMode::SetupMode() : GameMode() {
  gamer = new Gamer();
  levelSet = level = 0;
  screenshot = 0;
}
void SetupMode::display() {
  glPushAttrib(GL_ENABLE_BIT);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Draw background */
  Enter2DMode();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  if (screenshot)
    glBindTexture(GL_TEXTURE_2D, screenshot);
  else
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

  char str[256];
  int opt;
  Settings *settings = Settings::settings;
  int w, h;
  int fontSize = 48;
  int titleFontSize = 64;
  int col0 = screenWidth / 2 - 400;
  int col1 = screenWidth / 2 + 100;
  int row0 = screenHeight / 2 - 100;
  int rowSep = fontSize + 5;

  int INFO_BASE = screenHeight - 70;
  int INFO_DELTA = 20;
  int INFO_RIGHT = 20;
  double INFO_WIDTH = 10.0, INFO_HEIGHT = 10.0, INFO_R = 1.0, INFO_G = 1.0, INFO_B = 1.0,
         INFO_A = 1.0;

  double DESC_WIDTH = 14.0, DESC_HEIGHT = 14.0, DESC_R = 1.0, DESC_G = 1.0, DESC_B = 1.0,
         DESC_A = 1.0;

  /* Special fix for lower screen resolutions */
  if (screenWidth == 640) {
    fontSize = 32;
    titleFontSize = 48;
    col0 = screenWidth / 2 - 300;
    col1 = screenWidth / 2 + 50;
    rowSep = fontSize + 5;

    INFO_WIDTH = 8;
    INFO_HEIGHT = 8;
    INFO_DELTA = 16;

    DESC_WIDTH = 8.0;
    DESC_HEIGHT = 8.0;
  }
  if (screenWidth == 800) {
    fontSize = 32;
    titleFontSize = 48;
    col0 = screenWidth / 2 - 350;
    col1 = screenWidth / 2 + 75;
    rowSep = fontSize + 5;

    DESC_WIDTH = 10.0;
    DESC_HEIGHT = 10.0;

    /*
    INFO_WIDTH=8;
    INFO_HEIGHT=8;
    INFO_DELTA=16;    */
  }

  int DESC_RIGHT = (int)(col0 - DESC_WIDTH);
  int DESC_BASE = (int)(row0 + rowSep * 4);
  int DESC_DELTA = (int)(DESC_HEIGHT * 2);

  /* Draw title */
  addText_Center(0, titleFontSize / 2, 64, _("Setup new game"), screenWidth / 2);

  /* Name */
  clearSelectionAreas();
  addText_Left(0, fontSize / 2, row0 + rowSep * 0, _("Player Name"), col0);
  addText_Left(CODE_NAME, fontSize / 2, row0 + rowSep * 0, gamer->name, col1);

  /* Level set */
  addText_Left(0, fontSize / 2, row0 + rowSep * 1, _("Level Set"), col0);
  if (settings->doSpecialLevel)
    addText_Left(0, fontSize / 2, row0 + rowSep * 1, _("N/A"), col1);
  else
    addText_Left(CODE_LEVEL_SET, fontSize / 2, row0 + rowSep * 1,
                 settings->levelSets[levelSet].name, col1);

  /* Start level */
  addText_Left(0, fontSize / 2, row0 + rowSep * 2, _("Level"), col0);
  if (settings->doSpecialLevel)
    addText_Left(0, fontSize / 2, row0 + rowSep * 2, settings->specialLevel, col1);
  else
    addText_Left(CODE_START_LV, fontSize / 2, row0 + rowSep * 2,
                 gamer->levels[levelSet][level].name, col1);

  /* Difficulty */
  addText_Left(0, fontSize / 2, row0 + rowSep * 3, _("Difficulty"), col0);
  addText_Left(
      CODE_DIFFICULTY, fontSize / 2, row0 + rowSep * 3,
      (settings->sandbox
           ? (settings->difficulty == 0
                  ? _("Sandbox (Easy)")
                  : (settings->difficulty == 1 ? _("Sandbox (Normal)") : _("Sandbox (Hard)")))
           : (settings->difficulty == 0
                  ? _("Easy")
                  : (settings->difficulty == 1 ? _("Normal") : _("Hard")))),
      col1);

  /* Start */
  addText_Center(CODE_START, fontSize / 2, screenHeight - 64, _("Start Game"),
                 screenWidth / 2);

  /* Info */
  snprintf(str, sizeof(str), _("Total games played: %d"), gamer->timesPlayed);
  Font::drawSimpleText(0, str, INFO_RIGHT, INFO_BASE + INFO_DELTA * 0, INFO_WIDTH, INFO_HEIGHT,
                       INFO_R, INFO_G, INFO_B, INFO_A);
  if (gamer->timesPlayed)
    snprintf(str, sizeof(str), _("Average score: %d"), gamer->totalScore / gamer->timesPlayed);
  else
    snprintf(str, sizeof(str), _("Average score: N/A"));
  Font::drawSimpleText(0, str, INFO_RIGHT, INFO_BASE + INFO_DELTA * 1, INFO_WIDTH, INFO_HEIGHT,
                       INFO_R, INFO_G, INFO_B, INFO_A);
  snprintf(str, sizeof(str), _("Levels completed: %d"), gamer->nLevelsCompleted);
  Font::drawSimpleText(0, str, INFO_RIGHT, INFO_BASE + INFO_DELTA * 2, INFO_WIDTH, INFO_HEIGHT,
                       INFO_R, INFO_G, INFO_B, INFO_A);

  /* Descriptive level text */
  int lineno;
  for (lineno = 0; lineno < 5; lineno++)
    Font::drawSimpleText(0, settings->levelSets[levelSet].description[lineno], DESC_RIGHT,
                         DESC_BASE + DESC_DELTA * lineno, DESC_WIDTH, DESC_HEIGHT, DESC_R,
                         DESC_G, DESC_B, DESC_A);

  /*                      */
  /* Draw the player ball */
  /*                      */

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glTranslatef(0.75, -0.75, 0.0);  // 0.7
  gluPerspective(40, (GLdouble)screenWidth / (GLdouble)max(screenHeight, 1), 1.0, 1e20);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(-6.5, -6.5, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  GLfloat lightDiffuse[] = {0.9, 0.9, 0.9, 1.0};
  GLfloat lightPosition[] = {-100.0, -50.0, 150.0, 0.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textures[gamer->textureNum]);

  if (settings->gfx_details == 5) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  glColor4f(1.0, 1.0, 1.0, 1.0);
  GLfloat diffuse[4];
  for (int i = 0; i < 3; i++) diffuse[i] = colors[gamer->color][i];
  diffuse[3] = 1.0;
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuse);

  GLfloat specular[4] = {1.0, 1.0, 1.0, 1.0};
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  glRotated(t * -50.0, 0.0, 1.0, 0.0);

  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
  gluSphere(qball, 0.8, max(10, 4 + 3 * settings->gfx_details),
            max(10, 4 + 3 * settings->gfx_details));
  glPopAttrib();

  drawMousePointer();
  displayFrameRate();

  glPopAttrib();
}
void SetupMode::key(int key) {
  int len = strlen(gamer->name);
  int selected = getSelectedArea();

  if (key == SDLK_SPACE && selected != CODE_NAME) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    mouseDown(1, x, y);
  }
  if (key == SDLK_RETURN) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    mouseDown(3, x, y);
  }

  if (key == SDLK_BACKSPACE) {
    if (len > 0) gamer->name[len - 1] = 0;
    gamer->update();
    name = -1;
    return;
  }
  if (len == 19) {
    playEffect(SFX_PLAYER_DIES);
    return;
  }
  if (key < 127 && isprint(key)) {
    if (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) key = toupper(key);
    gamer->name[len] = (char)key;
    gamer->name[len + 1] = 0;
    gamer->update();
    name = -1;
    level = 0;
  }
}
void SetupMode::idle(Real td) {
  int x, y;

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

  t += td;
}
void SetupMode::activated() {
  char str[256];

  gamer->reloadNames();

  if (!background) {
    snprintf(str, sizeof(str), "%s/images/setupBackground.jpg", SHARE_DIR);
    background = IMG_Load(str);
    if (!background) {
      printf(_("Error: Failed to load %s\n"), str);
      exit(0);
    }
  }

  /* Preloads the background texture. */
  GLfloat texcoord[4];
  texture = LoadTexture(background, texcoord);
  texMaxX = texcoord[0];
  texMinY = texcoord[1];
  texMinX = texcoord[2];
  texMaxY = texcoord[3];

  t = 0.0;
  level = 0;
  name = -1;

  levelSetChanged();
}
void SetupMode::deactivated() {
  if (texture != 0) {
    glDeleteTextures(1, &texture);
    texture = 0;
  }
  if (screenshot) {
    glDeleteTextures(1, &screenshot);
    screenshot = 0;
  }
  gamer->save();
  if (low_memory && background) {
    SDL_FreeSurface(background);
    background = NULL;
  }
}
void SetupMode::start() {
  if (Game::current) delete Game::current;
  if (Settings::settings->doSpecialLevel) {
    Game::current = new Game(Settings::settings->specialLevel, gamer);
    Game::current->currentLevelSet = -1;
  } else {
    Game::current = new Game(gamer->levels[levelSet][level].fileName, gamer);
    Game::current->currentLevelSet = levelSet;
    HallOfFameMode::hallOfFameMode->levelSet = levelSet;
  }
  GameMode::activate(MainMode::mainMode);
}

void SetupMode::mouseDown(int button, int x, int y) {
  int selected = getSelectedArea();

  x += 32;
  y += 32;
  if (y >= (screenHeight * 2) / 3 && x > (screenWidth * 3) / 4)
    selected = CODE_COLOR;  // Ugly fix for selecting the 3D object

  Settings *settings = Settings::settings;
  if (selected) {
    int left = (button == 1);
    switch (selected) {
    case CODE_START:
      start();
      break;
    case CODE_COLOR:
      if (left)
        gamer->color = (gamer->color + 1) % 5;
      else {
        gamer->color = 4;
        gamer->textureNum = (gamer->textureNum + 1) % numTextures;
      }
      break;
    case CODE_DIFFICULTY:
      settings->difficulty =
          mymod((settings->difficulty + 3 * settings->sandbox + (left ? 1 : -1)), 6);
      settings->sandbox = settings->difficulty / 3;
      settings->difficulty = settings->difficulty % 3;
      break;
    case CODE_START_LV:
      level = mymod((level + (left ? 1 : -1)), gamer->nKnownLevels[levelSet]);
      break;
    case CODE_NAME:
      if (gamer->nNames) {
        name = (name + (left ? 1 : -1)) % gamer->nNames;
        strncpy(gamer->name, gamer->names[name], 20);
        gamer->update();
        level = 0;
      }
      break;
    case CODE_LEVEL_SET:
      levelSet = mymod((levelSet + (left ? 1 : -1)), settings->nLevelSets);
      level = 0;
      levelSetChanged();
      break;
    }
  }
}

void SetupMode::levelSetChanged() {
  Settings *settings = Settings::settings;

  if (screenshot != 0) {
    glDeleteTextures(1, &screenshot);
    screenshot = 0;
  }

  if (settings->levelSets[levelSet].imagename[0] != 0 && settings->gfx_details >= 2) {
    SDL_Surface *surf = IMG_Load(settings->levelSets[levelSet].imagename);
    if (surf) {
      GLfloat texcoord[4];
      screenshot = LoadTexture(surf, texcoord);
      SDL_FreeSurface(surf);
      screenshotMax[0] = texcoord[2];
      screenshotMax[1] = texcoord[3];
    }
  }
}
