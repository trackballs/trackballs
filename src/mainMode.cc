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
  gameStatus = statusBeforeGame;
  statusCount = 0.;
  time = 0.;
  game = NULL;
  this->go_to_pause = 0;
  this->pause_time = 0.;
  memset(&this->cameraModelView[0], 0, 16 * sizeof(double));
  memset(&this->cameraProjection[0], 0, 16 * sizeof(double));
}
MainMode::~MainMode() {}

void MainMode::display() {
  if (!game) { return; }

  Map *map = game->map;
  Player *player1 = game->player1;

  if (game->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    activeView.fog_enabled = 1;
    assign(game->fogColor, activeView.fog_color);
    activeView.fog_start = std::max(0.0, 14.0 - 7.0 * game->fogThickness);
    activeView.fog_end = 26.0 - 4.0 * game->fogThickness;
  } else
    activeView.fog_enabled = 0;

  perspectiveMatrix(40, (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1), 0.1, 200,
                    activeView.projection);

  /* Setup matrixes for the camera perspective */
  if (gameStatus == statusBeforeGame) {
    lookAtMatrix(map->startPosition[0] - 12 * std::sin(time),
                 map->startPosition[1] - 12 * std::cos(time), 10.0 + map->startPosition[2],
                 map->startPosition[0], map->startPosition[1], map->startPosition[2], 0.0, 0.0,
                 1.0, activeView.modelview);

  } else {
    double angle = xyAngle * M_PI / 2. + M_PI / 4.;
    Coord3d up(std::sin(angle) * zAngle, std::cos(angle) * zAngle, 1.0 - zAngle);
    up = up / length(up);
    lookAtMatrix(
        camFocus[0] - 10. * std::sin(angle) * std::cos(zAngle * M_PI / 2.),
        camFocus[1] - 10. * std::cos(angle) * std::cos(zAngle * M_PI / 2.),
        10.0 + camFocus[2] * 0.5 + (10.0 + camFocus[2]) * std::sin(zAngle * M_PI / 2.),
        camFocus[0], camFocus[1], camFocus[2], up[0], up[1], up[2], activeView.modelview);
  }
  markViewChanged();

  /* lighting must be set before we render the shadow map */
  /* Shadow map rendering returns active modelview/projection to orig state */
  setupLighting(game, game->isNight);
  if (game->isNight) {
    if (Settings::settings->doShadows) {
      renderShadowMap(camFocus, map, game);
      renderDummyShadowCascade();
    } else {
      renderDummyShadowCascade();
      renderDummyShadowMap();
    }
  } else {
    activeView.day_mode = true;
    if (Settings::settings->doShadows) {
      renderShadowCascade(camFocus, map, game);
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

  if (game->fogThickness)
    glClearColor(game->fogColor[0], game->fogColor[1], game->fogColor[2], game->fogColor[3]);
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
    if (map) map->draw(0, map->startPosition, game->gameTime);
    game->draw();
    if (map) map->draw(1, map->startPosition, game->gameTime);
    {
      const char *lp[3], *rp[3];
      lp[0] = _("Track:");
      if (strlen(game->map->mapname)) {
        rp[0] = gettext(game->map->mapname);
      } else {
        rp[0] = _("Unknown track");
      }
      lp[1] = _("Author:");
      if (strlen(game->map->author)) {
        rp[1] = game->map->author;
      } else {
        rp[1] = _("Unknown author");
      }
      lp[2] = _("Press spacebar to begin");
      rp[2] = " ";
      multiMessage(3, lp, rp);
    }
    break;
  case statusGameOver:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    {
      char str[256];
      snprintf(str, sizeof(str), _("Press spacebar to continue"));
      message(_("Game over"), str);
    }
    break;
  case statusRestartPlayer:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    message(_("Oops"), _("Press spacebar to continue"));
    break;
  case statusInGame:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    break;
  case statusPaused:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();

    Enter2DMode();
    // darken the display
    draw2DRectangle(0, 0, screenWidth, screenHeight, 0., 0., 1., 1.,
                    Color(0., 0., 0., pause_time));
    drawCenterSimpleText(_("Paused"), screenWidth / 2, screenHeight / 2 - 16, 16,
                         Color(1.0, 1.0, 1.0, 0.75));
    Leave2DMode();
    break;
  case statusBonusLevelComplete:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    message(_("Bonus level complete"), _("Press spacebar to continue"));
    break;
  case statusLevelComplete:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    showBonus();
    break;
  case statusNextLevel:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    message(_("Level complete"), _("Press spacebar to continue"));
    break;
  case statusVictory:
    map->draw(0, camFocus, game->gameTime);
    game->draw();
    map->draw(1, camFocus, game->gameTime);
    showInfo();
    message(_("Congratulations"), _("You have completed the game!"));
    break;
  }

  Enter2DMode();
  if (player1->modTimeLeft[MOD_FROZEN]) {
    draw2DRectangle(
        0, 0, screenWidth, screenHeight, 0., 0., 1., 1.,
        Color(SRGBColor(0.5, 0.5, 1.0,
                        0.5 * std::min(1.0, (double)player1->modTimeLeft[MOD_FROZEN]))));
  }
  /* Print joystick debugging information */
  if (debug_joystick && Settings::settings->hasJoystick()) {
    char str[256];
    snprintf(str, 255, "Joy: %d, %d -> %.1f, %.1f", Settings::settings->joystickRawX(),
             Settings::settings->joystickRawY(), Settings::settings->joystickX(),
             Settings::settings->joystickY());
    drawCenterSimpleText(str, screenWidth / 2, screenHeight - 16, 8,
                         Color(SRGBColor(0.6, 0.6, 0.6, 0.6)));
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
                      0., 1., 1., -1., Color(SRGBColor(0.9, 0.8, 0.8, 1.)), cpy);

      glDeleteTextures(1, &cpy);
    }

    delete[] imgd;
    delete[] showd;
  }

  displayFrameRate();
  Leave2DMode();
}
void MainMode::doExpensiveComputations() { game->doExpensiveComputations(); }

void MainMode::key(int key) {
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
    game->player1->handleKey(key);
    break;
  case statusLevelComplete:
    break;
  case statusBonusLevelComplete:
  case statusNextLevel:
    if (key == ' ') {
      if (!game->nextLevel[0]) {
        gameStatus = statusVictory;
        break;
      }
      xyAngle = wantedXYAngle = zAngle = wantedZAngle = 0.0;
      game->fogThickness = game->wantedFogThickness;
      game->clearLevel();
      game->loadLevel(game->nextLevel, Gamer::gamer);
      game->player1->newLevel();
      restartPlayer();
      gameStatus = statusBeforeGame;
    }
    break;
  case statusGameOver: {
    if (key == ' ' || key == 'n') {
      EnterHighScoreMode::init();
      EnterHighScoreMode::enterHighScoreMode->lastGameScore = game->player1->score;
      EnterHighScoreMode::enterHighScoreMode->lastLevelSet = game->currentLevelSet;
      GameMode::activate(EnterHighScoreMode::init());
    }
  } break;
  case statusVictory:
    if (key == ' ') {
      Gamer::gamer->playerLose(game);
      EnterHighScoreMode::init();
      EnterHighScoreMode::enterHighScoreMode->lastGameScore = game->player1->score;
      EnterHighScoreMode::enterHighScoreMode->lastLevelSet = game->currentLevelSet;
      GameMode::activate(EnterHighScoreMode::init());
    }
    break;
  }
}

void MainMode::tick(Real td) {
  time += td;
  flash -= td;
  if (flash < 0.0) flash = 0.0;

  switch (gameStatus) {
  case statusVictory:
  case statusBonusLevelComplete:
    /* do nothing */
    break;
  case statusBeforeGame:
    SDL_SetRelativeMouseMode(SDL_TRUE);
    break;
  case statusGameOver: /* rotate view? */
    SDL_SetRelativeMouseMode(SDL_TRUE);
    game->tick(td);
    break;
  case statusLevelComplete: {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    int old_status = (int)statusCount;
    statusCount += 0.8 * td;
    Player *player = game->player1;
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
      if (game->nextLevel[0])
        gameStatus = statusNextLevel;
      else
        gameStatus = statusVictory;
    }
  }
  // note. No break here!
  case statusNextLevel:
  case statusInGame:
    SDL_SetRelativeMouseMode(SDL_TRUE);
    game->handleUserInput(gameStatus == statusInGame);

    if (wantedZAngle > zAngle)
      zAngle += std::min(0.4 * td, wantedZAngle - zAngle);
    else if (wantedZAngle < zAngle)
      zAngle -= std::min(0.4 * td, zAngle - wantedZAngle);
    /* take shortest path under modulo */
    if (std::abs(xyAngle + 4.f - wantedXYAngle) < std::abs(xyAngle - wantedXYAngle)) {
      xyAngle += 4.f;
    }
    if (std::abs(xyAngle - 4.f - wantedXYAngle) < std::abs(xyAngle - wantedXYAngle)) {
      xyAngle -= 4.f;
    }
    if (wantedXYAngle > xyAngle)
      xyAngle += std::min(0.4 * td, wantedXYAngle - xyAngle);
    else if (wantedXYAngle < xyAngle)
      xyAngle -= std::min(0.4 * td, xyAngle - wantedXYAngle);

    game->tick(td);
    for (int i = 0; i < 3; i++) {
      camDelta[i] = camDelta[i] * std::pow(1.9e-6, td) +
                    0.25 * td * (game->player1->position[i] - camFocus[i]);
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
    /* continue the simulation in the background to let things settle */
    game->tick(td);
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

  camFocus[0] = game->map->startPosition[0] - 5;
  camFocus[1] = game->map->startPosition[1] - 5;
  time = 0.0;
  flash = 0.0;
}
void MainMode::deactivated() {
  delete game;
  game = NULL;
  SDL_SetRelativeMouseMode(SDL_FALSE);
}
void MainMode::playerLose() {
  Gamer::gamer->playerLose(game);
  gameStatus = statusGameOver;
}
void MainMode::playerDie() { gameStatus = statusRestartPlayer; }

/* Pass along, nothing to see here... */
void MainMode::mouse(int state, int x, int y) {
  if (game) game->player1->mouse(state, x, y);
}
void MainMode::mouseDown(int button, int /*x*/, int /*y*/) {
  if (button == SDL_BUTTON_LMASK) key(' ');
}
/* Starts the current game */
void MainMode::startGame() {
  gameStatus = statusInGame;
  game->player1->hasWon = 0;
  playEffect(SFX_START);
  Coord3d pos = game->map->startPosition;
  Gamer::gamer->levelStarted(game);

  pos[2] += 2.0;
  game->add(new Sign(*game, _("Good luck!"), 6, 1.0, 60.0, pos));
  game->player1->position[2] += 1.0;
  game->player1->triggerHook(GameHookEvent_Spawn, NULL);
}

/* (Re)starts the current player */
void MainMode::restartPlayer() {
  Player *player1 = game->player1;
  player1->restart(game->map->startPosition);
  camFocus[0] = game->map->startPosition[0];
  camFocus[1] = game->map->startPosition[1];
  Gamer::gamer->levelStarted(game);

  Coord3d pos = game->map->startPosition;
  pos[2] += 2.0;
  game->add(new Sign(*game, _("Good luck!"), 7, 1.0, 60.0, pos));
  player1->triggerHook(GameHookEvent_Spawn, NULL);
}

/* Shows various information on screen */
void MainMode::showInfo() {
  if (!game) return;
  Player *player = game->player1;

  /* Don't draw the panel if we have released the cursor etc. This is
     useful for screenshots. */
  if (SDL_GetModState() & KMOD_CAPS) return;

  Enter2DMode();

  GLfloat notexco[4][2] = {{0., 0.}, {0., 0.}, {0., 0.}, {0., 0.}};

  GLfloat pvertices[4][2] = {{0.f, 0.f}, {160.f, 0.f}, {0.f, 100.f}, {160.f, 100.f}};
  Color pcols[4] = {Color(SRGBColor(1.0, 1.0, 0.0, 1.0)), Color(SRGBColor(1.0, 0.5, 0.1, 1.0)),
                    Color(SRGBColor(1.0, 0.5, 0.1, 1.0)),
                    Color(SRGBColor(1.0, 0.1, 0.1, 1.0))};
  draw2DRectangle(0., 0., 160. + 1.5, 100. + 1.5, 0., 0., 1., 1., Color(0., 0., 0., 1.));
  draw2DQuad(pvertices, notexco, pcols);

  const char *fieldNames[3] = {_("Lives"), _("Score"), _("Time left")};
  for (int i = 0; i < 3; i++) {
    GLfloat s = 29.f;
    GLfloat mvertices[4][2] = {{8.f, 13.f + i * s},
                               {152.f, 13.f + i * s},
                               {8.f, 35.f + i * s},
                               {152.f, 35.f + i * s}};
    Color mcols[4] = {
        Color(SRGBColor(0.75, 0.5, 0.0, 1.0)), Color(SRGBColor(1.0, 0.25, 0.0, 1.0)),
        Color(SRGBColor(0.75, 0.25, 0.0, 1.0)), Color(SRGBColor(1.0, 0.0, 0.0, 1.0))};
    draw2DRectangle(8.f - 1.5f, 13.f + i * s - 1.5f, 144.f, 22.f, 0., 0., 1., 1.,
                    Color(0., 0., 0., 1.));
    draw2DQuad(mvertices, notexco, mcols);
    drawSimpleText(fieldNames[i], 12, 23 + i * s, 9, Color(SRGBColor(1.0, 1.0, 0.0, 1.0)));
  }

  // lives
  for (int i = 0; i < 4; i++) {
    const char *name = (i < player->lives) ? "life.png" : "nolife.png";
    draw2DRectangle(63 + i * 20, 7, 32, 32, 0., 0., 1., 1., Color(1., 1., 1., 1.),
                    textures[loadTexture(name)]);
  }

  // Score
  char str[256];
  snprintf(str, sizeof(str), "%d", player->score);
  drawRightSimpleText(str, 150, 52, 9, Color(0.0, 0.0, 0.0, 1.0));

  // Time left
  snprintf(str, sizeof(str), "%2.2d:%2.2d", player->timeLeft / 60, player->timeLeft % 60);
  if (player->timeLeft < 15 && player->timeLeft % 2)
    drawRightSimpleText(str, 150, 81, 9, Color(1.0, 1.0, 1.0, 1.0));
  else
    drawRightSimpleText(str, 150, 81, 9, Color(0.0, 0.0, 0.0, 1.0));

  // Health
  draw2DRectangle(8, 3, 144, 5, 0., 0., 1., 1., Color(SRGBColor(0.5, 0.1, 0.1, 1.0)));

  GLfloat bvertices[4][2] = {{8.f, 3.f},
                             {8.f + (144.f * (GLfloat)player->health), 3.f},
                             {8.f, 8.f},
                             {8.f + (144.f * (GLfloat)player->health), 8.f}};
  Color bcols[4] = {Color(SRGBColor(0.8, 0.8, 0.1, 1.0)), Color(SRGBColor(0.8, 0.1, 0.1, 1.0)),
                    Color(SRGBColor(0.8, 0.8, 0.1, 1.0)),
                    Color(SRGBColor(0.8, 0.1, 0.1, 1.0))};
  draw2DQuad(bvertices, notexco, bcols);

  // Oxygen
  draw2DRectangle(8, 3, (int)(144.0 * (1. - player->oxygen)), 5, 0., 0., 1., 1.,
                  Color(SRGBColor(0.2, 0.2, 0.7, 0.8)));

  Leave2DMode();
}
void MainMode::levelComplete() {
  game->player1->hasWon = 1;
  gameStatus = statusLevelComplete;
  statusCount = 0.0;
}
void MainMode::showBonus() {
  char left[16][256];
  char right[16][256];
  const char *left_pointers[16];
  const char *right_pointers[16];
  int difficulty = Settings::settings->difficulty;
  Player *player = game->player1;

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
  strncpy(game->nextLevel, game->returnLevel, sizeof(game->nextLevel));
}

void MainMode::renderEnvironmentTexture(GLuint texture, const Coord3d &focus) const {
  // Save gfx_details and change before drawing environment */
  int gfx_details = Settings::settings->gfx_details;
  Settings::settings->gfx_details = GFX_DETAILS_MINIMALISTIC;

  if (game->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    activeView.fog_enabled = 1;
    activeView.fog_start = std::max(0.0, 14.0 - 7.0 * game->fogThickness);
    activeView.fog_end = std::max(0.0, 14.0 - 7.0 * game->fogThickness);
    assign(game->fogColor, activeView.fog_color);
  } else
    activeView.fog_enabled = 0;
  perspectiveMatrix(140, (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1), 0.01,
                    200, activeView.projection);

  /* Setup openGL matrixes for the camera perspective */
  double angle = xyAngle * M_PI / 2. + M_PI / 4.;
  // TODO. Fixme. This computation is wrong when zAngle > 0.0 !!
  Coord3d up(std::sin(angle) * zAngle, std::cos(angle) * zAngle, 1.0 - zAngle);
  up = up / length(up);
  lookAtMatrix(focus[0], focus[1], focus[2] + 0.0,
               focus[0] - 10. * std::sin(angle) * std::cos(zAngle * M_PI / 2.),
               focus[1] - 10. * std::cos(angle) * std::cos(zAngle * M_PI / 2.),
               zAngle > 0.0
                   ? 10.0 + focus[2] * 0.5 + (10.0 + focus[2]) * std::sin(zAngle * M_PI / 2.)
                   : focus[2] + 2.0,
               up[0], up[1], up[2], activeView.modelview);

  setupLighting(game, game->isNight);

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

  if (game->fogThickness)
    glClearColor(game->fogColor[0], game->fogColor[1], game->fogColor[2], game->fogColor[3]);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, reflFBO);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  markViewChanged();

  game->map->draw(0, focus, game->gameTime);
  game->drawReflection(focus);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &reflFBO);

  // Restore graphics details
  Settings::settings->gfx_details = gfx_details;
}

void MainMode::setupLighting(const Game *game, bool isNight) {
  const Coord3d moonPosition(0., 0., 2.);
  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
  if (isNight && game && game->player1) {
    GLfloat lightDiffuse2[3] = {0.5, 0.5, 0.5};
    GLfloat lightPosition2[3] = {(GLfloat)(game->player1->position[0] + moonPosition[0]),
                                 (GLfloat)(game->player1->position[1] + moonPosition[1]),
                                 (GLfloat)(game->player1->position[2] + moonPosition[2])};
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
