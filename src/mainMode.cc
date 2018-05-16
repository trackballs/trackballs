/* mainMode.cc
   The main operation mode of the game.

   Copyright (C) 2000  Mathias Broxvall
                       Yannick Perret

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

#include "mainMode.h"

#include "enterHighScoreMode.h"
#include "font.h"
#include "game.h"
#include "gamer.h"
#include "map.h"
#include "player.h"
#include "settings.h"
#include "sign.h"
#include "sound.h"
#include "weather.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <string.h>
#include <cstdlib>

#define ENVIRONMENT_TEXTURE_SIZE 512

const int MainMode::statusBeforeGame = 0, MainMode::statusGameOver = 1,
          MainMode::statusInGame = 2;
const int MainMode::statusRestartPlayer = 3, MainMode::statusNextLevel = 4,
          MainMode::statusVictory = 5;
const int MainMode::statusLevelComplete = 6, MainMode::statusBonusLevelComplete = 7;
const int MainMode::statusPaused = 8;

static const int debug_shadowmap = 0;

MainMode *MainMode::mainMode = NULL;

MainMode *MainMode::init() {
  if (!mainMode) mainMode = new MainMode();
  return mainMode;
}
void MainMode::cleanup() {
  if (mainMode) delete mainMode;
}
MainMode::MainMode() {
  flash = 0;
  zAngle = 0.;
  wantedZAngle = 0.;
  xyAngle = 0.;
  wantedXYAngle = 0.;
  camFocus = Coord3d();
  camDelta = Coord3d();
  gameStatus = 0;
  statusCount = 0.;
  time = 0.;
  this->go_to_pause = 0;
  this->pause_time = 0.;
  memset(&this->cameraModelView[0], 0, 16 * sizeof(double));
  memset(&this->cameraProjection[0], 0, 16 * sizeof(double));
  mapname = NULL;
}
MainMode::~MainMode() {}

void MainMode::display() {
  if (!Game::current) { return; }

  Map *map = Game::current->map;
  Player *player1 = Game::current->player1;

  if (Game::current->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    activeView.fog_enabled = 1;
    assign(Game::current->fogColor, activeView.fog_color);
    activeView.fog_start = std::max(0.0, 14.0 - 7.0 * Game::current->fogThickness);
    activeView.fog_end = 26.0 - 4.0 * Game::current->fogThickness;
  } else
    activeView.fog_enabled = 0;

  perspectiveMatrix(40, (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1), 0.1, 200,
                    activeView.projection);

  /* Setup matrixes for the camera perspective */
  if (gameStatus == statusBeforeGame) {
    lookAtMatrix(map->startPosition[0] - 12 * sin(time),
                 map->startPosition[1] - 12 * cos(time), 10.0 + map->startPosition[2],
                 map->startPosition[0], map->startPosition[1], map->startPosition[2], 0.0, 0.0,
                 1.0, activeView.modelview);

  } else {
    double angle = xyAngle * M_PI / 2. + M_PI / 4.;
    Coord3d up(sin(angle) * zAngle, cos(angle) * zAngle, 1.0 - zAngle);
    up = up / length(up);
    lookAtMatrix(camFocus[0] - 10. * sin(angle) * cos(zAngle * M_PI / 2.),
                 camFocus[1] - 10. * cos(angle) * cos(zAngle * M_PI / 2.),
                 10.0 + camFocus[2] * 0.5 + (10.0 + camFocus[2]) * sin(zAngle * M_PI / 2.),
                 camFocus[0], camFocus[1], camFocus[2], up[0], up[1], up[2],
                 activeView.modelview);
  }
  markViewChanged();

  /* lighting must be set before we render the shadow map */
  /* Shadow map rendering returns active modelview/projection to orig state */
  setupLighting(Game::current->isNight);
  if (Game::current->isNight) {
    if (Settings::settings->doShadows) {
      renderShadowMap(camFocus, map, Game::current);
      renderDummyShadowCascade();
    } else {
      renderDummyShadowCascade();
      renderDummyShadowMap();
    }
  } else {
    activeView.day_mode = true;
    if (Settings::settings->doShadows) {
      renderShadowCascade(camFocus, map, Game::current);
      renderDummyShadowMap();
    } else {
      renderDummyShadowCascade();
      renderDummyShadowMap();
    }
  }

  /* record modelview/projection matrices for item visibility testing */
  assign(activeView.modelview, this->cameraModelView);
  assign(activeView.projection, this->cameraProjection);

  /* Primary scene rendering */
  glViewport(0, 0, screenWidth, screenHeight);

  if (Game::current->fogThickness)
    glClearColor(Game::current->fogColor[0], Game::current->fogColor[1],
                 Game::current->fogColor[2], Game::current->fogColor[3]);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

  switch (gameStatus) {
  case statusBeforeGame:
    if (map) map->draw(0, (int)map->startPosition[0], (int)map->startPosition[1]);
    Game::current->draw();
    if (map) map->draw(1, (int)map->startPosition[0], (int)map->startPosition[1]);
    {
      const char *lp[3], *rp[3];
      lp[0] = _("Track:");
      if (strlen(Game::current->map->mapname)) {
        rp[0] = gettext(Game::current->map->mapname);
      } else {
        rp[0] = _("Unknown track");
      }
      lp[1] = _("Author:");
      if (strlen(Game::current->map->author)) {
        rp[1] = Game::current->map->author;
      } else {
        rp[1] = _("Unknown author");
      }
      lp[2] = _("Press spacebar to begin");
      rp[2] = " ";
      multiMessage(3, lp, rp);
    }
    break;
  case statusGameOver:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    {
      char str[256];
      snprintf(str, sizeof(str), _("Press spacebar to continue"));
      message(_("Game over"), str);
    }
    break;
  case statusRestartPlayer:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Oops"), _("Press spacebar to continue"));
    break;
  case statusInGame:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    break;
  case statusPaused:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();

    Enter2DMode();
    // darken the display
    draw2DRectangle(0, 0, screenWidth, screenHeight, 0., 0., 1., 1., 0., 0., 0., pause_time);
    Font::drawCenterSimpleText(_("Paused"), screenWidth / 2, screenHeight / 2 - 16, 16, 1.0,
                               1.0, 1.0, 0.75);
    Leave2DMode();
    break;
  case statusBonusLevelComplete:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Bonus level complete"), _("Press spacebar to continue"));
    break;
  case statusLevelComplete:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    showBonus();
    break;
  case statusNextLevel:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Level complete"), _("Press spacebar to continue"));
    break;
  case statusVictory:
    map->draw(0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Congratulations"), _("You have completed the game!"));
    break;
  }

  Enter2DMode();
  if (player1->modTimeLeft[MOD_FROZEN]) {
    draw2DRectangle(0, 0, screenWidth, screenHeight, 0., 0., 1., 1., 0.5, 0.5, 1.0,
                    0.5 * std::min(1.0, (double)player1->modTimeLeft[MOD_FROZEN]));
  }
  /* Print joystick debugging information */
  if (debug_joystick && Settings::settings->hasJoystick()) {
    char str[256];
    snprintf(str, 255, "Joy: %d, %d -> %.1f, %.1f", Settings::settings->joystickRawX(),
             Settings::settings->joystickRawY(), Settings::settings->joystickX(),
             Settings::settings->joystickY());
    Font::drawCenterSimpleText(str, screenWidth / 2, screenHeight - 16, 8, 0.6, 0.6, 0.6, 0.6);
  }

  if (debug_shadowmap) {
    int N = activeView.day_mode ? 3 : 6;
    int size = activeView.day_mode ? activeView.cascadeTexsize : activeView.shadowMapTexsize;
    GLfloat *imgd = new GLfloat[size * size * N]();

    if (activeView.day_mode) {
      for (int i = 0; i < 3; i++) {
        glBindTexture(GL_TEXTURE_2D, activeView.cascadeTexture[i]);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &imgd[size * size * i]);
      }
    } else {
      glBindTexture(GL_TEXTURE_CUBE_MAP, activeView.shadowMapTexture);
      GLenum dirs[6] = {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
      for (int i = 0; i < 6; i++) {
        glGetTexImage(dirs[i], 0, GL_DEPTH_COMPONENT, GL_FLOAT, &imgd[size * size * i]);
      }
    }

    GLuint nloc[6][2] = {{2, 1}, {0, 1}, {1, 1}, {3, 1}, {1, 0}, {1, 2}};
    GLuint cloc[3][2] = {{0, 0}, {0, 1}, {0, 2}};
    GLuint(*locations)[2] = activeView.day_mode ? cloc : nloc;

    GLfloat maxf = 0;
    GLfloat minf = FLT_MAX;

    int step = size * size * (activeView.day_mode ? 1 : N);
    int ns = activeView.day_mode ? 3 : 1;

    GLfloat *showd = new GLfloat[4 * size * size * N];
    for (int s = 0; s < ns; s++) {
      for (int i = step * s; i < step * (s + 1); i++) {
        maxf = std::max(maxf, imgd[i]);
        minf = std::min(minf, imgd[i]);
      }
      if (maxf <= minf) {
        maxf = 1;
        minf = 0;
      }

      double sc = 1 / (maxf - minf);
      for (int i = step * s; i < step * (s + 1); i++) {
        showd[4 * i] = sc * (imgd[i] - minf);
        showd[4 * i + 1] = sc * (imgd[i] - minf);
        showd[4 * i + 2] = sc * (imgd[i] - minf);
        showd[4 * i + 3] = 1.;
      }
    }

    for (int i = 0; i < N; i++) {
      glActiveTexture(GL_TEXTURE0);

      GLuint cpy;
      glGenTextures(1, &cpy);
      /* must bind cpy or we overwrite random textures */
      glBindTexture(GL_TEXTURE_2D, cpy);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT,
                   &showd[4 * size * size * i]);

      int s = 128;
      draw2DRectangle(50 + (s + 10) * locations[i][0], 50 + (s + 10) * locations[i][1], s, s,
                      0., 1., 1., -1., 0.9, 0.8, 0.8, 1., cpy);

      glDeleteTextures(1, &cpy);
    }

    delete[] imgd;
    delete[] showd;
  }

  displayFrameRate();
  Leave2DMode();
}
void MainMode::doExpensiveComputations() { Game::current->doExpensiveComputations(); }

void MainMode::key(int key) {
  Player *player1 = Game::current ? Game::current->player1 : NULL;

  switch (gameStatus) {
  case statusBeforeGame:
    if (key == ' ') startGame();
    break;
  case statusRestartPlayer:
    if (key == ' ') {
      restartPlayer();
      gameStatus = statusInGame;
    }
    break;
  case statusPaused:
    if (key != 'p' && key != 'P') go_to_pause = 0;
    // gameStatus = statusInGame;
    break;
  case statusInGame:
    if ((key == 'p') || (key == 'P')) {
      go_to_pause = 1;
      pause_time = 0.;
      gameStatus = statusPaused;
      break;
    }
    if (key == SDLK_PRINTSCREEN) {
      createSnapshot();
      break;
    }
    player1->key(key);
    break;
  case statusBonusLevelComplete:
  case statusNextLevel:
    if (key == ' ') {
      if (!Game::current->nextLevel[0]) {
        gameStatus = statusVictory;
        break;
      }
      xyAngle = wantedXYAngle = zAngle = wantedZAngle = 0.0;
      Game::current->fogThickness = Game::current->wantedFogThickness;
      Game::current->clearLevel();
      Game::current->loadLevel(Game::current->nextLevel);
      Game::current->player1->newLevel();
      restartPlayer();
      gameStatus = statusBeforeGame;
    }
    break;
  case statusGameOver: {
    if (key == ' ' || key == 'n') GameMode::activate(EnterHighScoreMode::init());
  } break;
  case statusVictory:
    if (key == ' ') {
      GameMode::activate(EnterHighScoreMode::init());
      Game::current->gamer->playerLose();
    }
    break;
  }
}

void MainMode::idle(Real td) {
  Player *player1 = Game::current ? Game::current->player1 : NULL;

  time += td;
  flash -= td;
  if (flash < 0.0) flash = 0.0;

  switch (gameStatus) {
  case statusBeforeGame:
    SDL_SetRelativeMouseMode(SDL_TRUE);
    break;
  case statusGameOver: /* rotate view? */
    SDL_SetRelativeMouseMode(SDL_TRUE);
    Game::current->tick(td);
    break;
  case statusLevelComplete: {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    int old_status = (int)statusCount;
    statusCount += 0.8 * td;
    Player *player = Game::current->player1;
    if ((int)statusCount != old_status) {
      if ((int)statusCount <= 3) playEffect(SFX_GOT_FLAG);
      switch ((int)statusCount) {
      case 1:
        player->score += player->lives * 100;
        break;
      case 2:
        player->score += (player->timeLeft / 60) * 250;
        break;
      case 3:
        player->score += Settings::settings->difficulty * 500;
        break;
      }
    }
    if (statusCount > 5.0) {
      playEffect(SFX_LV_COMPLETE);
      if (Game::current->nextLevel[0])
        gameStatus = statusNextLevel;
      else
        gameStatus = statusVictory;
    }
  }
  // note. No break here!
  case statusNextLevel:
  case statusInGame:
    SDL_SetRelativeMouseMode(SDL_TRUE);
    if (wantedZAngle > zAngle)
      zAngle += std::min(0.4 * td, wantedZAngle - zAngle);
    else if (wantedZAngle < zAngle)
      zAngle -= std::min(0.4 * td, zAngle - wantedZAngle);
    /* take shortest path under modulo */
    if (abs(xyAngle + 4.f - wantedXYAngle) < abs(xyAngle - wantedXYAngle)) { xyAngle += 4.f; }
    if (abs(xyAngle - 4.f - wantedXYAngle) < abs(xyAngle - wantedXYAngle)) { xyAngle -= 4.f; }
    if (wantedXYAngle > xyAngle)
      xyAngle += std::min(0.4 * td, wantedXYAngle - xyAngle);
    else if (wantedXYAngle < xyAngle)
      xyAngle -= std::min(0.4 * td, xyAngle - wantedXYAngle);

    Game::current->tick(td);
    for (double t = td; t >= 0.0; t -= 0.01)
      for (int i = 0; i < 3; i++) {
        camDelta[i] = camDelta[i] * 0.9 + 0.002 * (player1->position[i] - camFocus[i]);
        camFocus[i] += camDelta[i];
      }
    break;
  case statusPaused:
    SDL_SetRelativeMouseMode(SDL_FALSE);
    if (go_to_pause) {
      if (pause_time > 0.5) {
        pause_time = 0.5;
        break;
      }
      pause_time += td;
    } else {
      if (pause_time <= 0.) {
        pause_time = 0.;
        gameStatus = statusInGame;
        break;
      }
      pause_time -= td;
    }
    break;
  case statusRestartPlayer:
    SDL_SetRelativeMouseMode(SDL_TRUE);
    Game::current->tick(td);
    //    assign(map->startPosition,camFocus); zero(camDelta);
    break;
  }
}
void MainMode::activated() {
  zAngle = wantedZAngle = 0.0;
  xyAngle = wantedXYAngle = 0.0;
  camFocus = Coord3d();
  camDelta = Coord3d();
  gameStatus = statusBeforeGame;
  SDL_SetRelativeMouseMode(SDL_TRUE);

  camFocus[0] = Game::current->map->startPosition[0] - 5;
  camFocus[1] = Game::current->map->startPosition[1] - 5;
  time = 0.0;
  flash = 0.0;
}
void MainMode::deactivated() { SDL_SetRelativeMouseMode(SDL_FALSE); }
void MainMode::playerLose() {
  Game::current->gamer->playerLose();
  gameStatus = statusGameOver;
}
void MainMode::playerDie() { gameStatus = statusRestartPlayer; }

/* Pass along, nothing to see here... */
void MainMode::mouse(int state, int x, int y) {
  if (Game::current) Game::current->player1->mouse(state, x, y);
}
void MainMode::mouseDown(int button, int /*x*/, int /*y*/) {
  if (button == SDL_BUTTON_LMASK) key(' ');
}
/* Starts the current game */
void MainMode::startGame() {
  gameStatus = statusInGame;
  Game::current->player1->hasWon = 0;
  playEffect(SFX_START);
  Coord3d pos = Game::current->map->startPosition;
  Game::current->gamer->levelStarted();

  pos[2] += 2.0;
  Game::current->add(new Sign(_("Good luck!"), 6, 1.0, 60.0, pos));
  Game::current->player1->position[2] += 1.0;
  Game::current->player1->triggerHook(GameHookEvent_Spawn, NULL);
}

/* (Re)starts the current player */
void MainMode::restartPlayer() {
  Player *player1 = Game::current ? Game::current->player1 : NULL;
  player1->restart(Game::current->map->startPosition);
  camFocus[0] = Game::current->map->startPosition[0];
  camFocus[1] = Game::current->map->startPosition[1];
  Game::current->gamer->levelStarted();

  Coord3d pos = Game::current->map->startPosition;
  pos[2] += 2.0;
  Game::current->add(new Sign(_("Good luck!"), 7, 1.0, 60.0, pos));
  player1->triggerHook(GameHookEvent_Spawn, NULL);
}

/* Shows various information on screen */
void MainMode::showInfo() {
  if (!Game::current) return;
  Player *player = Game::current->player1;

  /* Don't draw the panel if we have released the cursor etc. This is
     useful for screenshots. */
  if (SDL_GetModState() & KMOD_CAPS) return;

  Enter2DMode();

  GLfloat notexco[4][2] = {{0., 0.}, {0., 0.}, {0., 0.}, {0., 0.}};

  GLfloat pvertices[4][2] = {{0.f, 0.f}, {160.f, 0.f}, {0.f, 100.f}, {160.f, 100.f}};
  GLfloat pcols[4][4] = {
      {1.0, 1.0, 0.0, 1.0}, {1.0, 0.5, 0.1, 1.0}, {1.0, 0.5, 0.1, 1.0}, {1.0, 0.1, 0.1, 1.0}};
  draw2DRectangle(0., 0., 160. + 1.5, 100. + 1.5, 0., 0., 1., 1., 0., 0., 0., 1.);
  draw2DQuad(pvertices, notexco, pcols);

  const char *fieldNames[3] = {_("Lives"), _("Score"), _("Time left")};
  for (int i = 0; i < 3; i++) {
    GLfloat s = 29.f;
    GLfloat mvertices[4][2] = {{8.f, 13.f + i * s},
                               {152.f, 13.f + i * s},
                               {8.f, 35.f + i * s},
                               {152.f, 35.f + i * s}};
    GLfloat mcols[4][4] = {{0.75, 0.5, 0.0, 1.0},
                           {1.0, 0.25, 0.0, 1.0},
                           {0.75, 0.25, 0.0, 1.0},
                           {1.0, 0.0, 0.0, 1.0}};
    draw2DRectangle(8.f - 1.5f, 13.f + i * s - 1.5f, 144.f, 22.f, 0., 0., 1., 1., 0., 0., 0.,
                    1.);
    draw2DQuad(mvertices, notexco, mcols);
    Font::drawSimpleText(fieldNames[i], 12, 23 + i * s, 9, 1.0, 1.0, 0.0, 1.0);
  }

  // lives
  for (int i = 0; i < 4; i++) {
    const char *name = (i < player->lives) ? "life.png" : "nolife.png";
    draw2DRectangle(63 + i * 20, 7, 32, 32, 0., 0., 1., 1., 1., 1., 1., 1.,
                    textures[loadTexture(name)]);
  }

  // Score
  char str[256];
  snprintf(str, sizeof(str), "%d", player->score);
  Font::drawRightSimpleText(str, 150, 52, 9, 0.0, 0.0, 0.0, 1.0);

  // Time left
  snprintf(str, sizeof(str), "%2.2d:%2.2d", player->timeLeft / 60, player->timeLeft % 60);
  if (player->timeLeft < 15 && player->timeLeft % 2)
    Font::drawRightSimpleText(str, 150, 81, 9, 1.0, 1.0, 1.0, 1.0);
  else
    Font::drawRightSimpleText(str, 150, 81, 9, 0.0, 0.0, 0.0, 1.0);

  // Health
  draw2DRectangle(8, 3, 144, 5, 0., 0., 1., 1., 0.5, 0.1, 0.1, 1.0);

  GLfloat bvertices[4][2] = {{8.f, 3.f},
                             {8.f + (144.f * (GLfloat)player->health), 3.f},
                             {8.f, 8.f},
                             {8.f + (144.f * (GLfloat)player->health), 8.f}};
  GLfloat bcols[4][4] = {
      {0.8, 0.8, 0.1, 1.0}, {0.8, 0.1, 0.1, 1.0}, {0.8, 0.8, 0.1, 1.0}, {0.8, 0.1, 0.1, 1.0}};
  draw2DQuad(bvertices, notexco, bcols);

  // Oxygen
  draw2DRectangle(8, 3, (int)(144.0 * (1. - player->oxygen)), 5, 0., 0., 1., 1., 0.2, 0.2, 0.7,
                  0.8);

  Leave2DMode();
}
void MainMode::levelComplete() {
  Game::current->player1->hasWon = 1;
  gameStatus = statusLevelComplete;
  statusCount = 0.0;
}
void MainMode::showBonus() {
  char left[16][256];
  char right[16][256];
  const char *left_pointers[16];
  const char *right_pointers[16];
  int difficulty = Settings::settings->difficulty;
  Player *player = Game::current->player1;

  for (int i = 0; i < 16; i++) {
    left_pointers[i] = &left[i][0];
    right_pointers[i] = &right[i][0];
  }

  snprintf(left[0], 255, _("Level bonuses"));
  snprintf(right[0], 255, " ");
  snprintf(left[1], 255, _("Lives left:"));
  snprintf(right[1], 255, _("%d x 100 = %d points"), player->lives, player->lives * 100);
  snprintf(left[2], 255, _("Time left:"));
  snprintf(right[2], 255, _("%d minutes = %d points"), player->timeLeft / 60,
           (player->timeLeft / 60) * 250);
  snprintf(left[3], 255, _("Difficulty:"));
  snprintf(right[3], 255, "%s = %d points",
           difficulty == 0 ? _("easy") : (difficulty == 1 ? _("normal") : _("hard")),
           difficulty * 500);

  multiMessage(std::min((int)statusCount + 1, 4), left_pointers, right_pointers);
}
void MainMode::bonusLevelComplete() {
  gameStatus = statusBonusLevelComplete;
  strncpy(Game::current->nextLevel, Game::current->returnLevel,
          sizeof(Game::current->nextLevel));
}

void MainMode::renderEnvironmentTexture(GLuint texture, const Coord3d &focus) const {
  // Save gfx_details and change before drawing environment */
  int gfx_details = Settings::settings->gfx_details;
  Settings::settings->gfx_details = GFX_DETAILS_MINIMALISTIC;

  if (Game::current->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    activeView.fog_enabled = 1;
    activeView.fog_start = std::max(0.0, 14.0 - 7.0 * Game::current->fogThickness);
    activeView.fog_end = std::max(0.0, 14.0 - 7.0 * Game::current->fogThickness);
    assign(Game::current->fogColor, activeView.fog_color);
  } else
    activeView.fog_enabled = 0;
  perspectiveMatrix(140, (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1), 0.01,
                    200, activeView.projection);

  /* Setup openGL matrixes for the camera perspective */
  double angle = xyAngle * M_PI / 2. + M_PI / 4.;
  // TODO. Fixme. This computation is wrong when zAngle > 0.0 !!
  Coord3d up(sin(angle) * zAngle, cos(angle) * zAngle, 1.0 - zAngle);
  up = up / length(up);
  lookAtMatrix(focus[0], focus[1], focus[2] + 0.0,
               focus[0] - 10. * sin(angle) * cos(zAngle * M_PI / 2.),
               focus[1] - 10. * cos(angle) * cos(zAngle * M_PI / 2.),
               zAngle > 0.0
                   ? 10.0 + focus[2] * 0.5 + (10.0 + focus[2]) * sin(zAngle * M_PI / 2.)
                   : focus[2] + 2.0,
               up[0], up[1], up[2], activeView.modelview);

  setupLighting(Game::current->isNight);

  /* Setup how we handle textures based on gfx_details */
  if (Settings::settings->gfx_details == 5) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ENVIRONMENT_TEXTURE_SIZE, ENVIRONMENT_TEXTURE_SIZE,
               0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  GLuint reflFBO;
  glGenFramebuffers(1, &reflFBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, reflFBO);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
  GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
  if (GL_FRAMEBUFFER_COMPLETE != result) {
    warning("Framebuffer is not complete. w/err %x", result);
  }

  /* actually render the scene */
  glViewport(0, 0, ENVIRONMENT_TEXTURE_SIZE, ENVIRONMENT_TEXTURE_SIZE);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  if (Game::current->fogThickness)
    glClearColor(Game::current->fogColor[0], Game::current->fogColor[1],
                 Game::current->fogColor[2], Game::current->fogColor[3]);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, reflFBO);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  markViewChanged();
  Map *map = Game::current ? Game::current->map : NULL;

  map->draw(0, (int)focus[0] + 10, (int)focus[1] + 10);
  Game::current->drawReflection(focus);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &reflFBO);

  // Restore graphics details
  Settings::settings->gfx_details = gfx_details;
}

void MainMode::setupLighting(bool isNight) {
  const Coord3d moonPosition(0., 0., 2.);
  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
  if (isNight && Game::current && Game::current->player1) {
    GLfloat lightDiffuse2[3] = {0.9, 0.9, 0.9};
    GLfloat lightPosition2[3] = {
        (GLfloat)(Game::current->player1->position[0] + moonPosition[0]),
        (GLfloat)(Game::current->player1->position[1] + moonPosition[1]),
        (GLfloat)(Game::current->player1->position[2] + moonPosition[2])};
    assign(lightDiffuse2, activeView.light_diffuse);
    assign(lightDiffuse2, activeView.light_specular);
    assign(lightPosition2, activeView.light_position);
    assign(black, activeView.light_ambient);
    activeView.day_mode = false;
  } else {
    GLfloat sunLight[3] = {0.8, 0.8, 0.8};
    GLfloat ambient[3] = {0.2, 0.2, 0.2};
    assign(sunLight, activeView.light_diffuse);
    assign(sunLight, activeView.light_specular);
    assign(ambient, activeView.light_ambient);
    activeView.day_mode = true;
  }
}
