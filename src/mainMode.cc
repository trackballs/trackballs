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

#include "general.h"
#include "gameMode.h"
#include "mainMode.h"
#include "glHelp.h"
#include "map.h"
#include "player.h"
#include "game.h"
#include "sound.h"
#include "flag.h"
#include "menuMode.h"
#include "enterHighScoreMode.h"
#include "sign.h"
#include "SDL/SDL_image.h"
#include "forcefield.h"
#include "scoreSign.h"
#include "settings.h"
#include "gamer.h"
#include "font.h"

using namespace std;

#define ENVIRONMENT_TEXTURE_SIZE 128

const int MainMode::statusBeforeGame = 0, MainMode::statusGameOver = 1,
          MainMode::statusInGame = 2;
const int MainMode::statusRestartPlayer = 3, MainMode::statusNextLevel = 4,
          MainMode::statusVictory = 5;
const int MainMode::statusLevelComplete = 6, MainMode::statusBonusLevelComplete = 7;
const int MainMode::statusPaused = 8;
// SDL_Surface *MainMode::panel,*MainMode::life,*MainMode::nolife;

MainMode *MainMode::mainMode = NULL;
char *MainMode::viewportData, *MainMode::environmentTextureData;

void MainMode::init() {
  char str[256];

  mainMode = new MainMode();

  /* Reflection maps */
  viewportData = NULL;
  environmentTextureData =
      (char *)malloc(sizeof(char) * 4 * ENVIRONMENT_TEXTURE_SIZE * ENVIRONMENT_TEXTURE_SIZE);
}
MainMode::MainMode() {
  this->go_to_pause = 0;
  this->pause_time = 0.;
}
MainMode::~MainMode() {}

void MainMode::display() {
  char str[256];
  int birdsEye = 0;

  Map *map = Game::current ? Game::current->map : NULL;
  Player *player1 = Game::current ? Game::current->player1 : NULL;

  glViewport(0, 0, screenWidth, screenHeight);
  glPushAttrib(GL_ENABLE_BIT);

  if (Game::current->fogThickness)
    glClearColor(Game::current->fogColor[0], Game::current->fogColor[1],
                 Game::current->fogColor[2], Game::current->fogColor[3]);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (Game::current->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, max(0.0, 14.0 - 7.0 * Game::current->fogThickness));
    glFogf(GL_FOG_END, 26.0 - 4.0 * Game::current->fogThickness);
    glFogfv(GL_FOG_COLOR, Game::current->fogColor);
  } else
    glDisable(GL_FOG);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40, 640.0 / 480.0, 0.1, 200);  // 40,640.0/480.0,1.0,1e20);

  /* Setup openGL matrixes for the camera perspective */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (gameStatus == statusBeforeGame) {
    gluLookAt(map->startPosition[0] - 12 * sin(time), map->startPosition[1] - 12 * cos(time),
              10.0 + map->startPosition[2], map->startPosition[0], map->startPosition[1],
              map->startPosition[2], 0.0, 0.0, 1.0);
    birdsEye = 1;
  } else {
    birdsEye = 0;
    if (zAngle > 0.25 || xyAngle > 0.3 || xyAngle < -.3) birdsEye = 1;

    double angle = xyAngle * M_PI / 2. + M_PI / 4.;
    Coord3d up;
    up[0] = sin(angle) * zAngle;
    up[1] = cos(angle) * zAngle;
    up[2] = 1.0 - zAngle;
    normalize(up);
    gluLookAt(camFocus[0] - 10. * sin(angle) * cos(zAngle * M_PI / 2.),
              camFocus[1] - 10. * cos(angle) * cos(zAngle * M_PI / 2.),
              10.0 + camFocus[2] * 0.5 + (10.0 + camFocus[2]) * sin(zAngle * M_PI / 2.),
              camFocus[0], camFocus[1], camFocus[2], up[0], up[1], up[2]);
  }

  /* Some standard GL settings needed */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glShadeModel(GL_SMOOTH);

  /* Debugging for problems with some cards */
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (Game::current && Game::current->isNight) {
    GLfloat lightDiffuse2[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat lightPosition2[4] = {252.0, 252.0, 10.0, 1.0};
    lightPosition2[0] = Game::current->player1->position[0];
    lightPosition2[1] = Game::current->player1->position[1];
    lightPosition2[2] = Game::current->player1->position[2] + 2.0;
    GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse2);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightDiffuse2);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition2);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.25);
    glEnable(GL_LIGHT0);
  } else {
    GLfloat sunLight[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat sunPosition[] = {-100.0, -50.0, 150.0, 0.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunLight);
    glLightfv(GL_LIGHT0, GL_POSITION, sunPosition);
    glEnable(GL_LIGHT0);
  }
  GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  glEnable(GL_LIGHTING);

  /* Setup how we handle textures based on gfx_details */
  if (Settings::settings->gfx_details == 5) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glHint(GL_FOG_HINT, GL_NICEST);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glHint(GL_FOG_HINT, GL_FASTEST);
  }

  switch (gameStatus) {
  case statusBeforeGame:
    if (map) map->draw(1, 0, (int)map->startPosition[0], (int)map->startPosition[1]);
    Game::current->draw();
    if (map) map->draw(1, 1, (int)map->startPosition[0], (int)map->startPosition[1]);
    {
      const char *lp[3], *rp[3];
      lp[0] = _("Track:");
      rp[0] = Game::current->map->mapname;
      lp[1] = _("Author:");
      rp[1] = Game::current->map->author;
      lp[2] = _("Press spacebar to begin");
      rp[2] = " ";
      multiMessage(3, lp, rp);
    }
    break;
  case statusGameOver:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    {
      char str[256];
      /*	  int buyCost;
      buyCost=(Game::current->player1->score/200)*100+500;
      if(Game::current->player1->score >= buyCost)
            snprintf(str,sizeof(str),"Buy continue (%d points)?",buyCost);
            else*/
      snprintf(str, sizeof(str), _("Press spacebar to continue"));
      message(_("Game over"), str);
    }
    break;
  case statusRestartPlayer:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Oops"), _("Press spacebar to continue"));
    break;
  case statusInGame:
    // printf("before map draw 0\n"); SDL_GL_SwapBuffers(); SDL_GL_SwapBuffers(); getc(stdin);
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    // printf("before game draw\n"); SDL_GL_SwapBuffers(); SDL_GL_SwapBuffers(); getc(stdin);
    Game::current->draw();
    // printf("before map draw 1\n"); SDL_GL_SwapBuffers(); SDL_GL_SwapBuffers(); getc(stdin);
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    // printf("before showinfo\n"); SDL_GL_SwapBuffers(); SDL_GL_SwapBuffers(); getc(stdin);
    showInfo();
    // printf("after everything\n"); SDL_GL_SwapBuffers(); SDL_GL_SwapBuffers(); getc(stdin);
    break;
  case statusPaused:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    // darken the display
    Enter2DMode();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glColor4f(0., 0., 0., pause_time);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(0, 0);
    glVertex2i(screenWidth, 0);
    glVertex2i(0, screenHeight);
    glVertex2i(screenWidth, screenHeight);
    glEnd();
    glDisable(GL_BLEND);
    Leave2DMode();
    // print 'paused'
    glColor4f(1., 1., 1., 1.);
    Font::drawCenterSimpleText(0, _("Paused"), screenWidth / 2, screenHeight / 2 - 16, 16.,
                               16., 1.0, 1.0, 1.0, 0.75);
    break;
  case statusBonusLevelComplete:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Bonus level complete"), _("Press spacebar to continue"));
    break;
  case statusLevelComplete:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    showBonus();
    break;
  case statusNextLevel:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Level complete"), _("Press spacebar to continue"));
    break;
  case statusVictory:
    map->draw(birdsEye, 0, (int)camFocus[0], (int)camFocus[1]);
    Game::current->draw();
    map->draw(birdsEye, 1, (int)camFocus[0], (int)camFocus[1]);
    showInfo();
    message(_("Congratulations"), _("You have completed the game!"));
    break;
  }

  if (player1->modTimeLeft[MOD_FROZEN]) {
    Enter2DMode();
    glColor4f(0.5, 0.5, 1.0, 0.5 * min(1.0, (double)player1->modTimeLeft[MOD_FROZEN]));
    ;
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(0, 0);
    glVertex2i(screenWidth, 0);
    glVertex2i(0, screenHeight);
    glVertex2i(screenWidth, screenHeight);
    glEnd();
    Leave2DMode();
  }

  displayFrameRate();
  /* Print joystick debugging information */
  if (debug_joystick && Settings::settings->hasJoystick()) {
    Enter2DMode();
    snprintf(str, 255, "Joy: %d, %d -> %.1f, %.1f", Settings::settings->joystickRawX(),
             Settings::settings->joystickRawY(), Settings::settings->joystickX(),
             Settings::settings->joystickY());
    Font::drawCenterSimpleText(0, str, screenWidth / 2, screenHeight - 16, 8., 8., 0.6, 0.6,
                               0.6, 0.6);
    Leave2DMode();
  }

  /* For debugging, this draws the players environment map on the upper right corner of the
   * screen */
  /*
  Enter2DMode();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,Game::current->player1->environmentTexture);
  glColor4f(1., 1., 1., 1.0);
  glEnable(GL_BLEND);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(1.0,1.0); glVertex2i(screenWidth-0-10, 0);
  glTexCoord2f(0.0,1.0); glVertex2i(screenWidth-ENVIRONMENT_TEXTURE_SIZE-10, 0);
  glTexCoord2f(1.0,0.0); glVertex2i(screenWidth-0-10, ENVIRONMENT_TEXTURE_SIZE);
  glTexCoord2f(0.0,0.0); glVertex2i(screenWidth-ENVIRONMENT_TEXTURE_SIZE-10,
  ENVIRONMENT_TEXTURE_SIZE);
  glEnd();
  Leave2DMode();
  */

  glPopAttrib();
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
    if (key == SDLK_PRINT) {
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
    /*	  int buyCost;
    buyCost=(Game::current->player1->score/200)*100+500;
    if(Game::current->player1->score >= buyCost && key == 'y') {
          restartPlayer();
          player1->lives++;
          player1->timeLeft += 30;
          gameStatus=statusInGame;
          new ScoreSign(-buyCost,player1->position);
          } else  */
    if (key == ' ' || key == 'n') GameMode::activate(EnterHighScoreMode::enterHighScoreMode);
  } break;
  case statusVictory:
    if (key == ' ') {
      GameMode::activate(EnterHighScoreMode::enterHighScoreMode);
      Game::current->gamer->playerLoose();
    }
    break;
  }
}
void MainMode::special(int key, int x, int y) {}
void MainMode::idle(Real td) {
  Player *player1 = Game::current ? Game::current->player1 : NULL;
  Map *map = Game::current ? Game::current->map : NULL;
  double t;
  int i;

  time += td;
  flash -= td;
  if (flash < 0.0) flash = 0.0;

  switch (gameStatus) {
  case statusBeforeGame:
    break;
  case statusGameOver: /* rotate view? */
    Game::current->tick(td);
    break;
  case statusLevelComplete: {
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
    if (wantedZAngle > zAngle)
      zAngle += min(0.4 * td, wantedZAngle - zAngle);
    else if (wantedZAngle < zAngle)
      zAngle -= min(0.4 * td, zAngle - wantedZAngle);
    if (wantedXYAngle > xyAngle)
      xyAngle += min(0.4 * td, wantedXYAngle - xyAngle);
    else if (wantedXYAngle < xyAngle)
      xyAngle -= min(0.4 * td, xyAngle - wantedXYAngle);

    Game::current->tick(td);
    for (t = td; t >= 0.0; t -= 0.01)
      for (i = 0; i < 3; i++) {
        camDelta[i] = camDelta[i] * 0.9 + 0.002 * (player1->position[i] - camFocus[i]);
        camFocus[i] += camDelta[i];
      }
    break;
  case statusPaused:
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
    Game::current->tick(td);
    //    assign(map->startPosition,camFocus); zero(camDelta);
    break;
  }
}
void MainMode::activated() {
  zAngle = wantedZAngle = 0.0;
  xyAngle = wantedXYAngle = 0.0;
  zero(camFocus);
  zero(camDelta);
  gameStatus = statusBeforeGame;
  SDL_WarpMouse(screenWidth / 2, screenHeight / 2);

  camFocus[0] = Game::current->map->startPosition[0] - 5;
  camFocus[1] = Game::current->map->startPosition[1] - 5;
  time = 0.0;
  flash = 0.0;

  /* Fix for an apparanet bug in my SDL + Xorg combination... */
  SDL_WM_ToggleFullScreen(screen);
  SDL_WM_ToggleFullScreen(screen);
}
void MainMode::deactivated() {
  free(viewportData);
  viewportData = NULL;
}
void MainMode::playerLoose() {
  Game::current->gamer->playerLoose();
  gameStatus = statusGameOver;
}
void MainMode::playerDie() { gameStatus = statusRestartPlayer; }

/* Pass along, nothing to see here... */
void MainMode::mouse(int state, int x, int y) {
  if (Game::current) Game::current->player1->mouse(state, x, y);
}
void MainMode::mouseDown(int button, int x, int y) {
  if (button == SDL_BUTTON_LMASK) key(' ');
}
/* Starts the current game */
void MainMode::startGame() {
  gameStatus = statusInGame;
  Game::current->player1->hasWon = 0;
  playEffect(SFX_START);
  Coord3d pos;
  assign(Game::current->map->startPosition, pos);
  pos[2] += 2.0;
  new Sign(_("Good luck!"), 6, 1.0, 60.0, pos);
  Game::current->player1->position[2] += 1.0;
}

/* (Re)starts the current player */
void MainMode::restartPlayer() {
  Player *player1 = Game::current ? Game::current->player1 : NULL;
  player1->restart(Game::current->map->startPosition);
  camFocus[0] = Game::current->map->startPosition[0];
  camFocus[1] = Game::current->map->startPosition[1];
  Game::current->gamer->levelStarted();

  Coord3d pos;
  assign(Game::current->map->startPosition, pos);
  pos[2] += 2.0;
  new Sign(_("Good luck!"), 7, 1.0, 60.0, pos);
}

/* Shows various information on screen */
void MainMode::showInfo() {
  if (!Game::current) return;
  char str[256];
  Player *player = Game::current->player1;
  int i, w, h;

  /* Don't draw the panel if we have released the cursor etc. This is
     usefull for screenshots. */
  if (SDL_GetModState() & KMOD_CAPS) return;

  glColor3f(1.0, 1.0, 1.0);
  Enter2DMode();

  // the panel
  bindTexture("left_panel.png");
  drawTextured2DRectangle(0, 0, 256, 128);

  // lives
  for (i = 0; i < 4; i++) {
    if (i < player->lives)
      bindTexture("life.png");
    else
      bindTexture("nolife.png");
    drawTextured2DRectangle(63 + i * 20, 7, 32, 32);
  }

  // Score
  snprintf(str, sizeof(str), "%d", player->score);
  Font::drawRightSimpleText(0, str, 152, 52, 8., 8., 0.0, 0.0, 0.0, 1.0);

  // Time left
  snprintf(str, sizeof(str), "%2.2d:%2.2d", player->timeLeft / 60, player->timeLeft % 60);
  if (player->timeLeft < 15 && player->timeLeft % 2)
    Font::drawRightSimpleText(0, str, 152, 81, 8., 8., 1.0, 1.0, 1.0, 1.0);
  else
    Font::drawRightSimpleText(0, str, 152, 81, 8., 8., 0.0, 0.0, 0.0, 1.0);

  // Health
  glDisable(GL_TEXTURE_2D);
  glColor4f(0.5, 0.1, 0.1, 1.0);
  glBegin(GL_POLYGON);
  glVertex2i(8, 3);
  glVertex2i(8 + 144, 3);
  glVertex2i(8 + 144, 8);
  glVertex2i(8, 8);
  glEnd();
  glBegin(GL_POLYGON);
  glColor4f(0.8, 0.8, 0.1, 1.0);
  glVertex2i(8, 3);
  glColor4f(0.8, 0.1, 0.1, 1.0);
  glVertex2i(8 + (int)(144.0 * player->health), 3);
  glVertex2i(8 + (int)(144.0 * player->health), 8);
  glColor4f(0.8, 0.8, 0.1, 1.0);
  glVertex2i(8, 8);
  glEnd();

  // Oxygen
  glEnable(GL_BLEND);
  glColor4f(0.2, 0.2, 0.7, 0.8);
  glBegin(GL_POLYGON);
  glVertex2i(8, 3);
  glVertex2i(8 + (int)(144.0 * (1. - player->oxygen)), 3);
  glVertex2i(8 + (int)(144.0 * (1. - player->oxygen)), 8);
  glVertex2i(8, 8);
  glEnd(),

      Leave2DMode();
  return;
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
  int i, difficulty = Settings::settings->difficulty;
  Player *player = Game::current->player1;

  for (i = 0; i < 16; i++) {
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

  multiMessage(min((int)statusCount + 1, 4), left_pointers, right_pointers);
}
void MainMode::bonusLevelComplete() {
  gameStatus = statusBonusLevelComplete;
  strcpy(Game::current->nextLevel, Game::current->returnLevel);
}

void MainMode::renderEnvironmentTexture(GLuint texture, Coord3d focus) {
  // Save gfx_details and change before drawing environment */
  int gfx_details = Settings::settings->gfx_details;
  Settings::settings->gfx_details = GFX_DETAILS_MINIMALISTIC;

  static int currentViewportSize = 512;
  static double currentTime = 0.0;
  if (currentTime > 0.05)
    currentViewportSize = max(128, currentViewportSize / 2);
  else if (currentTime < 0.05 / 4.0)
    currentViewportSize = min(512, currentViewportSize * 2);

  int viewportSize = currentViewportSize;
  if (screenHeight < 512) viewportSize = screenHeight;
  // viewportSize=256;

  double t0 = ((double)SDL_GetTicks()) / 1000.0;

  glViewport(0, 0, viewportSize, viewportSize);
  if (Game::current->fogThickness)
    glClearColor(Game::current->fogColor[0], Game::current->fogColor[1],
                 Game::current->fogColor[2], Game::current->fogColor[3]);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushAttrib(GL_ENABLE_BIT);
  if (Game::current->fogThickness && Settings::settings->gfx_details != GFX_DETAILS_NONE) {
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, max(0.0, 14.0 - 7.0 * Game::current->fogThickness));
    glFogf(GL_FOG_END, 26.0 - 4.0 * Game::current->fogThickness);
    glFogfv(GL_FOG_COLOR, Game::current->fogColor);
  } else
    glDisable(GL_FOG);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(140, 1.0, 0.01, 200);

  /* Setup openGL matrixes for the camera perspective */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  int birdsEye = 1;
  double angle = xyAngle * M_PI / 2. + M_PI / 4.;
  // TODO. Fixme. This computation is wrong when zAngle > 0.0 !!
  Coord3d up;
  up[0] = sin(angle) * zAngle;
  up[1] = cos(angle) * zAngle;
  up[2] = 1.0 - zAngle;
  normalize(up);
  gluLookAt(focus[0], focus[1], focus[2] + 0.0,
            focus[0] - 10. * sin(angle) * cos(zAngle * M_PI / 2.),
            focus[1] - 10. * cos(angle) * cos(zAngle * M_PI / 2.),
            zAngle > 0.0 ? 10.0 + focus[2] * 0.5 + (10.0 + focus[2]) * sin(zAngle * M_PI / 2.)
                         : focus[2] + 2.0,
            up[0], up[1], up[2]);

  /* Some standard GL settings needed */
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

  if (Game::current && Game::current->isNight) {
    GLfloat lightDiffuse2[] = {0.9, 0.9, 0.9, 1.0};
    GLfloat lightPosition2[4] = {252.0, 252.0, 10.0, 1.0};
    lightPosition2[0] = Game::current->player1->position[0];
    lightPosition2[1] = Game::current->player1->position[1];
    lightPosition2[2] = Game::current->player1->position[2] + 2.0;
    GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse2);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightDiffuse2);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition2);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.25);
  }

  glShadeModel(GL_SMOOTH);

  /* Setup how we handle textures based on gfx_details */
  if (Settings::settings->gfx_details == 5) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glHint(GL_FOG_HINT, GL_NICEST);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glHint(GL_FOG_HINT, GL_FASTEST);
  }

  Map *map = Game::current ? Game::current->map : NULL;
  Player *player1 = Game::current ? Game::current->player1 : NULL;

  map->draw(birdsEye, 0, (int)focus[0] + 10, (int)focus[1] + 10);
  Game::current->drawReflection(focus);
  // map->draw(birdsEye,1, (int) focus[0]+10,(int) focus[1]+10);

  glPopAttrib();

  /* Copy rendered image into viewportData */
  if (!viewportData) {
    viewportData = (char *)malloc(sizeof(char) * screenWidth * screenHeight * 3);
  }
  // approx 1-2 ms */
  glReadPixels(0, 0, viewportSize, viewportSize, GL_RGB, GL_UNSIGNED_BYTE,
               (GLvoid *)viewportData);

  // Converts viewport data to fisheye environment perspective
  // approx 10 ms */
  double t0_5 = ((double)SDL_GetTicks()) / 1000.0;
  convertToFisheye(environmentTextureData, viewportData, viewportSize);

  /* Takes approx 1-2 ms */
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA,  // GL_RGBA
               ENVIRONMENT_TEXTURE_SIZE, ENVIRONMENT_TEXTURE_SIZE, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, environmentTextureData);
  double t1 = ((double)SDL_GetTicks()) / 1000.0;
  currentTime = t1 - t0;
  // printf("SIZE: %d, TD: %f + %f = %f\n",viewportSize,t0_5-t0,t1-t0_5,t1-t0);

  // Restore graphics details
  Settings::settings->gfx_details = gfx_details;
}

/* Creates a 128x128 fisheye texture from viewport data */
void MainMode::convertToFisheye(char *data, char *viewport, int viewportSize) {
  /* TODO. Let the environment map sample multiple viewport pixels.
         Encode an alpha value directly in the translationmap and let the alpha value increase
         as the angle away from the observer increases.
  */
  int x, y, i, j, x2, y2;
  static short *translationMap = NULL;
  static short translationViewportSize = 0;
  if (!translationMap)
    translationMap = (short *)malloc(sizeof(short) * ENVIRONMENT_TEXTURE_SIZE *
                                     ENVIRONMENT_TEXTURE_SIZE * 2);
  if (translationViewportSize != viewportSize) {
    translationViewportSize = viewportSize;
    /* Create mapping between fisheye texture and viewport */
    int x, y, i, x2, y2;
    for (x = 0; x < ENVIRONMENT_TEXTURE_SIZE; x++) {
      double nx = (x - ENVIRONMENT_TEXTURE_SIZE / 2) * 2.0 / ENVIRONMENT_TEXTURE_SIZE;
      for (y = 0; y < ENVIRONMENT_TEXTURE_SIZE; y++) {
        double ny = (y - ENVIRONMENT_TEXTURE_SIZE / 2) * 2.0 / ENVIRONMENT_TEXTURE_SIZE;
        double nz = 1.0 - nx * nx - ny * ny;
        if (nz > 0.0) {
          nz = sqrt(nz);
          double dotprod = 2.0 * nz;
          double vx = -2.0 * nx;
          double vy = -2.0 * ny;
          double vz = 1 - 2.0 * nz;
          double h = 0.05 / vz;
          double fx2 = -h * vx;
          double fy2 = +h * vy;
          if (dotprod < 1.0) {
            fy2 = -fy2;
            fx2 = -fx2;
          }
          if (fx2 > 0.5) {
            fx2 = 0.5;
            fy2 = (fy2 / fx2) * 0.5;
          } else if (fx2 < -0.5) {
            fx2 = -0.5;
            fy2 = (fy2 / -fx2) * 0.5;
          }
          if (fy2 > 0.5) {
            fy2 = 0.5;
            fx2 = (fx2 / fy2) * 0.5;
          } else if (fy2 < -0.5) {
            fy2 = -0.5;
            fx2 = (fx2 / -fy2) * 0.5;
          }
          x2 = (int)(viewportSize * (fx2 * 0.99 + 0.5));
          y2 = (int)(viewportSize * (fy2 * 0.99 + 0.5));

          translationMap[(x + y * ENVIRONMENT_TEXTURE_SIZE) * 2] = x2;
          translationMap[(x + y * ENVIRONMENT_TEXTURE_SIZE) * 2 + 1] = y2;
        } else {
          translationMap[(x + y * ENVIRONMENT_TEXTURE_SIZE) * 2 + 0] = -1;
          translationMap[(x + y * ENVIRONMENT_TEXTURE_SIZE) * 2 + 1] = 0;
        }
      }
    }
  }

  int size = ENVIRONMENT_TEXTURE_SIZE * ENVIRONMENT_TEXTURE_SIZE;
  for (i = 0; i < size; i++) {
    x2 = translationMap[i * 2 + 0];
    y2 = translationMap[i * 2 + 1];
    if (x2 == -1)
      data[i * 4 + 3] = 0;
    else {
      char *viewportPntr = &viewport[(x2 + y2 * viewportSize) * 3];
      for (j = 0; j < 3; j++) data[i * 4 + j] = viewportPntr[j];
      data[i * 4 + 3] = 255;
    }
  }
}
