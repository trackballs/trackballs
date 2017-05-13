/** \file game.cc
   Collects information about the current game
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
#include "game.h"
#include "map.h"
#include "player.h"
#include "animated.h"
#include "guile.h"
#include "gameHook.h"
#include "gamer.h"
#include "forcefield.h"
#include "settings.h"
#include "pipe.h"
#include "weather.h"
#include "sound.h"

#define MARGIN 10

using namespace std;

Game *Game::current = NULL;

extern GLfloat colors[5][3];

double Game::defaultScores[SCORE_MAX][2];

Game::Game(char *name, Gamer *g) {
  Ball::reset();
  ForceField::reset();
  Pipe::reset();

  current = this;
  objects = new set<class Animated *>();
  hooks = new set<class GameHook *>();
  map = NULL;
  player1 = NULL;
  gameTime = 0.0;
  nextLevel[0] = 0;
  setDefaults();
  gamer = g;

  localPlayers = 1;

  weather = new Weather();

  /* Load the bootup script */
  char scmname[256];
  snprintf(scmname, sizeof(scmname), "%s/levels/boot.scm", SHARE_DIR);
  fprintf(stderr, "Loading script %s ...", scmname);
  scm_c_primitive_load(scmname);
  fprintf(stderr, " done\n");

  player1 = new Player(gamer);
  loadLevel(name);
  player1->restart(Game::current->map->startPosition);
  player1->timeLeft = startTime;
  player1->lives = 4 - Settings::settings->difficulty;
}

Game::~Game() {
  clearLevel();

  delete player1;
  delete map;
  delete objects;
  delete hooks;
  delete weather;
}

void Game::loadLevel(char *name) {
  char mapname[256];
  char scmname[256];

  if (player1) {
    // level scripts might have modified our appearance. Reset them
    for (int i = 0; i < 3; i++) player1->primaryColor[i] = colors[gamer->color][i];
    player1->texture = textures[gamer->textureNum];
  }

  setDefaults();

  /* Find path to map, default is load any maps and/or scripts in
   * the home directory first (eg. ~/.trackballs/levels/{name}.map)
   * It is possible to mix using a map file in the home dir and a
   * script in the share dir or viceversa */
  snprintf(mapname, sizeof(mapname) - 1, "%s/.trackballs/levels/%s.map", getenv("HOME"), name);
  snprintf(scmname, sizeof(scmname) - 1, "%s/.trackballs/levels/%s.scm", getenv("HOME"), name);
  if (!fileExists(mapname))
    snprintf(mapname, sizeof(mapname), "%s/levels/%s.map", SHARE_DIR, name);
  if (!fileExists(scmname))
    snprintf(scmname, sizeof(scmname), "%s/levels/%s.scm", SHARE_DIR, name);
  snprintf(levelName, sizeof(levelName), "%s", name);

  if (map) delete map;
  map = new Map(mapname);
  fprintf(stderr, "Loading script %s ...", scmname);
  scm_c_primitive_load(scmname);
  fprintf(stderr, " done\n");

  if (player1) player1->timeLeft = startTime;

  fogThickness = wantedFogThickness;
}

void Game::setDefaults() {
  int i;

  isNight = 0;
  fogThickness = 0.0;
  wantedFogThickness = 0.0;
  startTime = 0;
  fogColor[0] = fogColor[1] = fogColor[2] = fogColor[3] = 1.0;
  useGrid = 1;
  jumpFactor = 1.0;
  oxygenFactor = 1.0;

  for (i = 0; i < SCORE_MAX; i++) {
    defaultScores[i][0] = 0.0;
    defaultScores[i][1] = 0.0;
  }
  defaultScores[SCORE_PLAYER][0] = -100;
  defaultScores[SCORE_PLAYER][1] = 60 - 5 * Settings::settings->difficulty;
  defaultScores[SCORE_BLACK][0] = 100;
  defaultScores[SCORE_BLACK][1] = 0;
  defaultScores[SCORE_BABY][0] = 50;
  defaultScores[SCORE_BABY][1] = 5 - 2 * Settings::settings->difficulty;
  defaultScores[SCORE_BIRD][0] = 100;
  defaultScores[SCORE_BIRD][1] = 0;
  defaultScores[SCORE_CACTUS][0] = 100;
  defaultScores[SCORE_CACTUS][1] = 0;
  defaultScores[SCORE_FLAG][0] = 100;
  defaultScores[SCORE_FLAG][1] = 0;

  /* Since the player object lives on from level to level we need to
     reset it's default values here too */
  if (player1) {
    player1->scoreOnDeath = defaultScores[SCORE_PLAYER][0];
    player1->timeOnDeath = defaultScores[SCORE_PLAYER][1];
  }
}

void Game::clearLevel() {
  Ball::reset();
  ForceField::reset();

  weather->clear();
  clearMusicPreferences();
  if (hooks) {
    set<GameHook *> *old_hooks = new set<GameHook *>(*hooks);
    set<GameHook *>::iterator ih = old_hooks->begin();
    set<GameHook *>::iterator endh = old_hooks->end();

    for (; ih != endh; ih++)
      if (*ih != player1) (*ih)->remove();
    delete old_hooks;
    GameHook::deleteDeadObjects();
    hooks->clear();
    objects->clear();

    hooks->insert(player1);
    objects->insert(player1);
  }
}

void Game::tick(Real t) {
  gameTime += t;
  if (fogThickness < wantedFogThickness)
    fogThickness += min(0.3 * t, wantedFogThickness - fogThickness);
  if (fogThickness > wantedFogThickness)
    fogThickness -= min(0.3 * t, fogThickness - wantedFogThickness);

  set<GameHook *> *old_hooks = new set<GameHook *>(*hooks);
  set<GameHook *>::iterator ih = old_hooks->begin();
  set<GameHook *>::iterator endh = old_hooks->end();

  for (; ih != endh; ih++) (*ih)->tick(t);
  delete old_hooks;

  weather->tick(t);
}
void Game::doExpensiveComputations() {
  set<GameHook *> *old_hooks = new set<GameHook *>(*hooks);
  set<GameHook *>::iterator ih = old_hooks->begin();
  set<GameHook *>::iterator endh = old_hooks->end();

  for (; ih != endh; ih++) (*ih)->doExpensiveComputations();
  delete old_hooks;
}

/* Draw all the objects in the world in two stages (nontransparent objects first, transparent
 * objects second) */
void Game::draw() {
  GLint viewport[4];
  GLdouble model_matrix[16], proj_matrix[16];

  /* Some GL defaults the objects can rely on being setup */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);

  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, proj_matrix);

  set<Animated *>::iterator i = objects->begin();
  set<Animated *>::iterator end = objects->end();

  /* Compute visibility of all objects */
  int nelem = 0;
  for (; i != end; i++) {
    Animated *anim = *i;
    anim->onScreen = 0;

    /* Test if bounding box is visible */
    if (testBboxClip(anim->position[0] + anim->boundingBox[0][0],
                     anim->position[0] + anim->boundingBox[1][0],
                     anim->position[1] + anim->boundingBox[0][1],
                     anim->position[1] + anim->boundingBox[1][1],
                     anim->position[2] + anim->boundingBox[0][2],
                     anim->position[2] + anim->boundingBox[1][2], model_matrix, proj_matrix)) {
      anim->onScreen = 1;
      nelem++;
    }
  }

  /* Draw first pass of all objects */
  i = objects->begin();
  for (; i != end; i++) {
    Animated *anim = *i;
    if (anim->onScreen) anim->draw();
  }

  /* Draw second pass of all objects */
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.01);  // fixes some problems with overlapping translucent graphics
  i = objects->begin();
  for (; i != end; i++) {
    Animated *anim = *i;
    if (anim->onScreen) anim->draw2();
  }

  weather->draw2();
}

/* Draws the world as normal but with the assumption that we are drawing a reflected version
   of the world. Does not draw weather or objects far away to save some time.
*/
void Game::drawReflection(Coord3d focus) {
  GLint viewport[4];
  GLdouble model_matrix[16], proj_matrix[16];

  /* Some GL defaults the objects can rely on being setup */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);

  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, proj_matrix);
  Coord3d worldCoord, screenCoord;

  set<Animated *>::iterator i = objects->begin();
  set<Animated *>::iterator end = objects->end();

  /* Compute visibility of all objects */
  for (; i != end; i++) {
    Animated *anim = *i;
    anim->onScreen = 0;

    double tmp[3];
    sub(focus, anim->position, tmp);
    if (length(tmp) > 5.0) continue;

    /* Test all the corners of the bounding box */
    int dx, dy, dz;
    for (dx = 0; dx < 2; dx++)
      for (dy = 0; dy < 2; dy++)
        for (dz = 0; dz < 2; dz++) {
          worldCoord[0] = anim->position[0] + anim->boundingBox[dx][0];
          worldCoord[1] = anim->position[1] + anim->boundingBox[dy][1];
          worldCoord[2] = anim->position[2] + anim->boundingBox[dz][2];
        }
    gluProject(worldCoord[0], worldCoord[1], worldCoord[2], model_matrix, proj_matrix,
               viewport, &screenCoord[0], &screenCoord[1], &screenCoord[2]);
    if (screenCoord[0] >= -MARGIN && screenCoord[0] <= screenWidth + MARGIN &&
        screenCoord[1] >= -MARGIN && screenCoord[1] <= screenHeight + MARGIN)
      anim->onScreen = 1;
  }

  /* Draw first pass of all objects */
  i = objects->begin();
  for (; i != end; i++) {
    Animated *anim = *i;
    if (anim->onScreen) anim->draw();
  }

  /* Draw second pass of all objects */
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.01);  // fixes some problems with overlapping translucent graphics
  i = objects->begin();
  for (; i != end; i++) {
    Animated *anim = *i;
    if (anim->onScreen) anim->draw2();
  }
}

void Game::add(Animated *a) { objects->insert(a); }
void Game::add(GameHook *a) { hooks->insert(a); }
void Game::remove(Animated *a) { objects->erase(a); }
void Game::remove(GameHook *a) { hooks->erase(a); }
