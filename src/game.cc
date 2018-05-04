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

#define MARGIN 10

#include "game.h"

#include "animatedCollection.h"
#include "ball.h"
#include "forcefield.h"
#include "gamer.h"
#include "guile.h"
#include "map.h"
#include "pipe.h"
#include "player.h"
#include "settings.h"
#include "sound.h"
#include "weather.h"

Game *Game::current = NULL;

extern Color colors[5];

double Game::defaultScores[SCORE_MAX][2];

Game::Game(const char *name, Gamer *g) {
  balls = new AnimatedCollection();

  current = this;

  map = NULL;
  player1 = NULL;
  gameTime = 0.0;
  nextLevel[0] = 0;
  setDefaults();
  gamer = g;

  localPlayers = 1;
  edit_mode = 0;

  weather = new Weather();

  /* Load the bootup script */
  char scmname[256];
  snprintf(scmname, sizeof(scmname), "%s/levels/boot.scm", effectiveShareDir);
  loadScript(scmname);

  player1 = new Player();
  add(player1);

  loadLevel(name);
  player1->restart(Game::current->map->startPosition);

  player1->restart(Game::current->map->startPosition);
  player1->timeLeft = startTime;
  player1->lives = 4 - Settings::settings->difficulty;
}

Game::Game(Map *editmap) {
  balls = new AnimatedCollection();
  localPlayers = 0;
  player1 = NULL;
  gameTime = 0.0;
  nextLevel[0] = 0;
  weather = NULL;
  map = editmap;
  edit_mode = 1;
  setDefaults();

  current = this;
}

Game::~Game() {
  clearLevel();

  delete weather;
  delete balls;
  if (hooks[Role_Player][0] == player1) { hooks[Role_Player].clear(); }
  delete player1;

  if (!edit_mode) delete map;
  if (current == this) { current = NULL; }

  for (int i = 0; i < Role_MaxTypes; i++) {
    if (i == Role_Player) continue;
    int n = hooks[i].size();
    for (int k = 0; k < n; k++) { delete hooks[i][k]; }
    hooks[i].clear();
  }
}

void Game::loadLevel(const char *name) {
  char mapname[256];
  char scmname[256];

  if (player1 && gamer) {
    /* level scripts might have modified our appearance. Reset them */
    player1->primaryColor = colors[gamer->color];
    player1->texture = gamer->textureNum;
  } else {
    /* when there is no gamer (e.g, in help) pick random value */
    player1->primaryColor = Color(frandom(), frandom(), frandom(), 1.0);
    player1->primaryColor.v[((size_t)rand()) % 3] = 1.0;
    player1->texture = loadTexture("blank.png");
  }

  setDefaults();

  /* Find path to map, default is load any maps and/or scripts in
   * the home directory first (eg. ~/.trackballs/levels/{name}.map)
   * It is possible to mix using a map file in the home dir and a
   * script in the share dir or viceversa */
  snprintf(mapname, sizeof(mapname) - 1, "%s/.trackballs/levels/%s.map", getenv("HOME"), name);
  snprintf(scmname, sizeof(scmname) - 1, "%s/.trackballs/levels/%s.scm", getenv("HOME"), name);
  if (!fileExists(mapname))
    snprintf(mapname, sizeof(mapname), "%s/levels/%s.map", effectiveShareDir, name);
  if (!fileExists(scmname))
    snprintf(scmname, sizeof(scmname), "%s/levels/%s.scm", effectiveShareDir, name);
  snprintf(levelName, sizeof(levelName), "%s", name);

  if (map) delete map;
  map = new Map(mapname);
  loadScript(scmname);

  for (int j = 0; j < newHooks.size(); j++) {
    hooks[newHooks[j]->entity_role].push_back(newHooks[j]);
  }
  newHooks.clear();

  if (player1) player1->timeLeft = startTime;

  fogThickness = wantedFogThickness;
}

void Game::setDefaults() {
  isNight = 0;
  fogThickness = 0.0;
  wantedFogThickness = 0.0;
  startTime = 0;
  fogColor[0] = fogColor[1] = fogColor[2] = fogColor[3] = 1.0;
  useGrid = 1;
  jumpFactor = 1.0;
  oxygenFactor = 1.0;

  for (int i = 0; i < SCORE_MAX; i++) {
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

void Game::add(GameHook *hook) { newHooks.push_back(hook); }

void Game::queueCall(SCM fun) {
  QueuedCall q;
  q.fun = fun;
  q.argA = NULL;
  q.argB = NULL;
  queuedCalls.push_back(q);
}
void Game::queueCall(SCM fun, SCM argA) {
  scm_gc_protect_object(argA);
  QueuedCall q;
  q.fun = fun;
  q.argA = argA;
  q.argB = NULL;
  queuedCalls.push_back(q);
}
void Game::queueCall(SCM fun, SCM argA, SCM argB) {
  scm_gc_protect_object(argA);
  scm_gc_protect_object(argB);
  QueuedCall q;
  q.fun = fun;
  q.argA = argA;
  q.argB = argB;
  queuedCalls.push_back(q);
}

void Game::clearLevel() {
  if (balls) {
    delete balls;
    balls = NULL;
  }
  balls = new AnimatedCollection();

  if (weather) weather->clear();
  clearMusicPreferences();

  /* Clear up dead entities once scripting no longer can use them*/
  for (int i = 0; i < hooks[Role_Dead].size(); i++) { delete hooks[Role_Dead][i]; }
  hooks[Role_Dead].clear();

  /* remove and delete existing non-player entities */
  for (int i = Role_GameHook; i < Role_MaxTypes; i++) {
    if (i == Role_Player) continue;
    int n = hooks[i].size();
    for (int k = 0; k < n; k++) {
      hooks[i][k]->remove();
      delete hooks[i][k];
    }
    hooks[i].clear();
  }
}

void Game::tick(Real t) {
  int steps = std::max(1, int(t / PHYSICS_RESOLUTION + 0.5));
  double origtime = gameTime;
  double tstep = t / steps;

  /* The game ticks run at a faster time scale so that the interaction
   * of different moving objects is realistic */
  for (int i = 0; i < steps; i++) {
    gameTime += tstep;

    /* Update intersection information */
    balls->clear();
    for (int j = Role_Ball; j <= Role_Player; j++) {
      int nb = hooks[j].size();
      for (int k = 0; k < nb; k++) { balls->add((Animated *)hooks[j][k]); }
    }
    balls->recalculateBboxMap();

    /* update active entities */
    for (int j = Role_GameHook; j < Role_MaxTypes; j++) {
      int n = hooks[j].size();
      for (int k = 0; k < n; k++) { hooks[j][k]->tick(tstep); }
    }

    /* add new entities */
    for (int j = 0; j < newHooks.size(); j++) {
      hooks[newHooks[j]->entity_role].push_back(newHooks[j]);
    }
    newHooks.clear();

    /* run queued callbacks */
    for (int j = 0; j < queuedCalls.size(); j++) {
      QueuedCall call = queuedCalls[j];
      /* the functions are owned by GameHooks; arguments by Game */
      if (!call.argA) {
        scm_catch_call_0(call.fun);
      } else if (!call.argB) {
        scm_catch_call_1(call.fun, call.argA);
        scm_gc_unprotect_object(call.argA);
      } else {
        scm_catch_call_2(call.fun, call.argA, call.argB);
        scm_gc_unprotect_object(call.argA);
        scm_gc_unprotect_object(call.argB);
      }
    }
    queuedCalls.clear();

    /* filter out dead entities, except players */
    for (int j = Role_GameHook; j < Role_MaxTypes; j++) {
      if (j == Role_Player) continue;

      int n = hooks[j].size();
      int ndead = 0;
      for (int k = 0; k < n; k++) {
        if (hooks[j][k - ndead]->invalid) {
          GameHook *hook = hooks[j][k - ndead];
          hook->releaseCallbacks();
          hooks[Role_Dead].push_back(hook);
          hooks[j][k - ndead] = hooks[j][n - 1 - ndead];
          hooks[j].pop_back();
          ndead++;
        }
      }
    }
  }
  gameTime = origtime + t;

  /* Weather updates have no physical effect */
  if (fogThickness < wantedFogThickness)
    fogThickness += std::min(0.3 * t, wantedFogThickness - fogThickness);
  if (fogThickness > wantedFogThickness)
    fogThickness -= std::min(0.3 * t, fogThickness - wantedFogThickness);
  weather->tick(t);
}
void Game::doExpensiveComputations() {
  for (int i = 0; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int k = 0; k < n; k++) { hooks[i][k]->doExpensiveComputations(); }
  }
}

/* Draw all the objects in the world in two stages (nontransparent objects first, transparent
 * objects second) */
void Game::draw() {
  /* Some GL defaults the objects can rely on being setup */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);

  Matrix4d mvp;
  matrixMult(activeView.modelview, activeView.projection, mvp);
  for (int i = Role_OtherAnimated; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int j = 0; j < n; j++) {
      Animated *anim = (Animated *)hooks[i][j];

      anim->onScreen = testBboxClip(anim->position[0] + anim->boundingBox[0][0],
                                    anim->position[0] + anim->boundingBox[1][0],
                                    anim->position[1] + anim->boundingBox[0][1],
                                    anim->position[1] + anim->boundingBox[1][1],
                                    anim->position[2] + anim->boundingBox[0][2],
                                    anim->position[2] + anim->boundingBox[1][2], mvp);
    }
  }

  /* Draw first pass of all objects */
  for (int i = Role_OtherAnimated; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int j = 0; j < n; j++) {
      Animated *anim = (Animated *)hooks[i][j];
      if (anim->onScreen) anim->draw();
    }
  }

  /* Draw second pass of all objects */
  for (int i = Role_OtherAnimated; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int j = 0; j < n; j++) {
      Animated *anim = (Animated *)hooks[i][j];
      if (anim->onScreen) anim->draw2();
    }
  }

  if (weather) weather->draw2();

  warnForGLerrors("Game drawing");
}

/* Draws the world as normal but with the assumption that we are drawing a reflected version
   of the world. Does not draw weather or objects far away to save some time.
*/
void Game::drawReflection(const Coord3d &focus) {
  /* Some GL defaults the objects can rely on being setup */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthFunc(GL_LEQUAL);

  Matrix4d mvp;
  matrixMult(activeView.modelview, activeView.projection, mvp);
  for (int i = Role_OtherAnimated; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int j = 0; j < n; j++) {
      Animated *anim = (Animated *)hooks[i][j];

      Coord3d tmp = focus - anim->position;
      if (length(tmp) > 5.0) {
        anim->onScreen = false;
        continue;
      }

      anim->onScreen = testBboxClip(anim->position[0] + anim->boundingBox[0][0],
                                    anim->position[0] + anim->boundingBox[1][0],
                                    anim->position[1] + anim->boundingBox[0][1],
                                    anim->position[1] + anim->boundingBox[1][1],
                                    anim->position[2] + anim->boundingBox[0][2],
                                    anim->position[2] + anim->boundingBox[1][2], mvp);
    }
  }

  /* Draw first pass of all objects */
  for (int i = Role_OtherAnimated; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int j = 0; j < n; j++) {
      Animated *anim = (Animated *)hooks[i][j];
      if (anim->onScreen) anim->draw();
    }
  }

  /* Draw second pass of all objects */
  for (int i = Role_OtherAnimated; i < Role_MaxTypes; i++) {
    int n = hooks[i].size();
    for (int j = 0; j < n; j++) {
      Animated *anim = (Animated *)hooks[i][j];
      if (anim->onScreen) anim->draw2();
    }
  }
}
