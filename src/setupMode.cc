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

#include "setupMode.h"

#include "font.h"
#include "game.h"
#include "gamer.h"
#include "hofMode.h"
#include "mainMode.h"
#include "menuMode.h"
#include "menusystem.h"
#include "player.h"
#include "settings.h"
#include "sound.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mouse.h>

#define SETUP_NAME 0
#define NUM_FIELDS 1

#define Y_BASE (screenHeight / 2 - 30)
#define Y_DELTA 40
#define Y_IMG_DIFF 52

#define CODE_NAME 1
#define CODE_LEVEL_SET 2
#define CODE_START_LV 3
#define CODE_DIFFICULTY 4
#define CODE_START 5
#define CODE_COLOR 6

Color colors[5] = {Color(1.0, 0.2, 0.2, 1.0), Color(0.2, 1.0, 0.2, 1.0),
                   Color(0.5, 0.5, 1.0, 1.0), Color(1.0, 1.0, 0.2, 1.0),
                   Color(1.0, 1.0, 1.0, 1.0)};

static SetupMode *setupMode = NULL;
SetupMode *SetupMode::init() {
  if (!setupMode) setupMode = new SetupMode();
  return setupMode;
}
void SetupMode::cleanup() {
  if (setupMode) delete setupMode;
}

SetupMode::SetupMode() : GameMode() {
  gamer = new Gamer();
  levelSet = level = 0;
  screenshot = 0;
  t = 0.;
  name = 0;
  if (low_memory)
    background = NULL;
  else {
    background = loadImage("setupBackground.jpg");
  }
}
SetupMode::~SetupMode() {
  if (background) SDL_FreeSurface(background);
}
void SetupMode::display() {
  Settings *settings = Settings::settings;
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Draw background */
  GLfloat coord[4];
  GLuint active;
  if (screenshot && !Settings::settings->doSpecialLevel) {
    active = screenshot;
    for (int i = 0; i < 4; i++) coord[i] = scrshtCoord[i];
  } else {
    active = texture;
    for (int i = 0; i < 4; i++) coord[i] = texCoord[i];
  }
  /* Avoid distortion. Note 1024x512 textures are possible. */
  if (screenWidth * scrshtH * coord[3] > screenHeight * scrshtW * coord[2]) {
    double delta = coord[3] - coord[2] * screenHeight * scrshtW /
                                  (double)std::max(screenWidth * scrshtH, 1);
    coord[1] += delta / 2;
    coord[3] -= delta;
  } else {
    double delta = coord[2] - coord[3] * screenWidth * scrshtH /
                                  (double)std::max(screenHeight * scrshtW, 1);
    coord[0] += delta / 2;
    coord[2] -= delta;
  }
  Enter2DMode();
  draw2DRectangle(0, 0, screenWidth, screenHeight, coord[0], coord[1], coord[2], coord[3], 1.,
                  1., 1., 1., active);

  Leave2DMode();

  /*                      */
  /* Draw the player ball */
  /*                      */

  warnForGLerrors("start of setup ball");

  Matrix4d persp_trans = {{1.f, 0.f, 0.f, 0.f},
                          {0.f, 1.f, 0.f, 0.f},
                          {0.f, 0.f, 1.f, 0.f},
                          {0.75f, -0.75f, 0.f, 1.f}};
  Matrix4d persp_tmp;
  perspectiveMatrix(40, (GLdouble)screenWidth / (GLdouble)fmax(screenHeight, 1), 0.1, 200,
                    persp_tmp);
  matrixMult(persp_tmp, persp_trans, activeView.projection);

  lookAtMatrix(-6.5, -6.5, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, activeView.modelview);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  MainMode::setupLighting(false);
  activeView.fog_enabled = 0;
  markViewChanged();

  if (settings->gfx_details == 5) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  // Create sphere
  int ntries = 0;
  int nverts = 0;
  int detail = 7;
  countObjectSpherePoints(&ntries, &nverts, detail);
  GLfloat *data = new GLfloat[nverts * 8];
  ushort *idxs = new ushort[ntries * 3];
  GLfloat pos[3] = {0.f, 0.f, 0.f};
  Color color = colors[gamer->color];
  Matrix4d frommtx;
  identityMatrix(frommtx);
  rotateY(t * -0.3, frommtx);
  Matrix3d rotation;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) rotation[i][j] = frommtx[i][j];

  placeObjectSphere(data, idxs, 0, pos, rotation, 1.0, detail, color);

  // Transfer
  setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(Color(0.3, 0.3, 0.3, 0.3), 20., Lighting_NoShadows);
  glBindTexture(GL_TEXTURE_2D, textures[gamer->textureNum]);

  warnForGLerrors("trans");

  GLuint databuf, idxbuf, vao;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, nverts * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ntries * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);

  glBindVertexArray(0);
  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);
  glDeleteVertexArrays(1, &vao);

  warnForGLerrors("end of setup ball");
  Enter2DMode();

  char str[256];

  int lineSize = computeLineSize();
  int fontSize = computeMenuSize();
  int titleFontSize = computeHeaderSize();
  int col0 = computeScreenBorder();
  int col1 = screenWidth / 2;
  int col1MaxExtent = screenWidth;
  int rowSep = 9 * fontSize / 5;
  int row0 = screenHeight / 2 - rowSep * 4;
  int lineSep = 2 * lineSize;

  /* Draw title */
  addText_Center(0, titleFontSize, 32 + titleFontSize / 2, _("Setup new game"),
                 screenWidth / 2);

  /* Name */
  clearSelectionAreas();
  addText_Left(0, fontSize, row0 + rowSep * 0, _("Player Name"), col0);
  addText_Left(CODE_NAME, fontSize, row0 + rowSep * 0, gamer->name, col1, col1MaxExtent);

  /* Level set */
  addText_Left(0, fontSize, row0 + rowSep * 1, _("Level Set"), col0);
  if (settings->doSpecialLevel)
    addText_Left(0, fontSize, row0 + rowSep * 1, _("N/A"), col1, col1MaxExtent);
  else
    addText_Left(CODE_LEVEL_SET, fontSize, row0 + rowSep * 1,
                 gettext(settings->levelSets[levelSet].name), col1, col1MaxExtent);

  /* Start level */
  addText_Left(0, fontSize, row0 + rowSep * 2, _("Level"), col0);
  if (settings->doSpecialLevel)
    addText_Left(0, fontSize, row0 + rowSep * 2, settings->specialLevel, col1, col1MaxExtent);
  else if (strlen(gamer->levels[levelSet][level].name))
    addText_Left(CODE_START_LV, fontSize, row0 + rowSep * 2,
                 gettext(gamer->levels[levelSet][level].name), col1, col1MaxExtent);
  else
    addText_Left(CODE_START_LV, fontSize, row0 + rowSep * 2, _("Unknown track"), col1,
                 col1MaxExtent);

  /* Difficulty */
  addText_Left(0, fontSize, row0 + rowSep * 3, _("Difficulty"), col0);
  const char *difficulty_str =
      (settings->sandbox
           ? (settings->difficulty == 0
                  ? _("Sandbox (Easy)")
                  : (settings->difficulty == 1 ? _("Sandbox (Normal)") : _("Sandbox (Hard)")))
           : (settings->difficulty == 0
                  ? _("Easy")
                  : (settings->difficulty == 1 ? _("Normal") : _("Hard"))));
  addText_Left(CODE_DIFFICULTY, fontSize, row0 + rowSep * 3, difficulty_str, col1,
               col1MaxExtent);

  /* Info */
  snprintf(str, sizeof(str), _("Total games played: %d"), gamer->timesPlayed);
  Font::drawSimpleText(str, col0, screenHeight - fontSize * 2 - lineSep * 4, lineSize, 1.0,
                       1.0, 1.0, 1.0);
  if (gamer->timesPlayed)
    snprintf(str, sizeof(str), _("Average score: %d"), gamer->totalScore / gamer->timesPlayed);
  else
    snprintf(str, sizeof(str), _("Average score: N/A"));
  Font::drawSimpleText(str, col0, screenHeight - fontSize * 2 - lineSep * 3, lineSize, 1.0,
                       1.0, 1.0, 1.0);
  snprintf(str, sizeof(str), _("Levels completed: %d"), gamer->nLevelsCompleted);
  Font::drawSimpleText(str, col0, screenHeight - fontSize * 2 - lineSep * 2, lineSize, 1.0,
                       1.0, 1.0, 1.0);

  /* Descriptive level text */
  if (!Settings::settings->doSpecialLevel) {
    for (int lineno = 0; lineno < 5; lineno++) {
      if (strlen(settings->levelSets[levelSet].description[lineno])) {
        Font::drawSimpleText(gettext(settings->levelSets[levelSet].description[lineno]),
                             col0 + lineSize, row0 + rowSep * 4 + lineSep * lineno, lineSize,
                             1.0, 1.0, 1.0, 1.0, screenWidth - col0 - lineSize);
      }
    }
  }

  /* Start */
  addText_Center(CODE_START, fontSize, screenHeight - 5 * fontSize / 2, _("Start Game"),
                 screenWidth / 2);

  displayFrameRate();
  drawMousePointer();

  Leave2DMode();
}
void SetupMode::key(int key) {
  int len = strlen(gamer->name);
  int selected = getSelectedArea();

  int shift = (SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0;
  if (key == SDLK_TAB) { moveKeyboardFocus(shift); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER ||
      (key == SDLK_SPACE && selected != CODE_NAME))
    mouseDown(shift ? 3 : 1, -1, -1);
  if (key == SDLK_ESCAPE) GameMode::activate(MenuMode::init());

  if (selected == CODE_NAME) {
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
}
void SetupMode::tick(Real td) {
  tickMouse(td);

  t += td;
}
void SetupMode::activated() {
  gamer->reloadNames();

  if (!background) { background = loadImage("setupBackground.jpg"); }

  /* Preloads the background texture. */
  texture = LoadTexture(background, texCoord);

  clearKeyboardFocus();

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
    if (HallOfFameMode::hallOfFameMode) HallOfFameMode::hallOfFameMode->levelSet = levelSet;
  }
  GameMode::activate(MainMode::init());
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
      if (settings->sandboxAvailable) {
        settings->difficulty =
            mymod((settings->difficulty + 3 * settings->sandbox + (left ? 1 : -1)), 6);
        settings->sandbox = settings->difficulty / 3;
        settings->difficulty = settings->difficulty % 3;
      } else {
        settings->sandbox = 0;
        settings->difficulty = mymod((settings->difficulty + (left ? 1 : -1)), 3);
      }
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
      scrshtW = powerOfTwo(surf->w);
      scrshtH = powerOfTwo(surf->h);
      screenshot = LoadTexture(surf, scrshtCoord);
      SDL_FreeSurface(surf);
    }
  }
}
