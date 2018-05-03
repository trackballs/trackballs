/* game.h
   Collects information about the current game

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

#ifndef GAME_H
#define GAME_H

#define SCORE_PLAYER 0
#define SCORE_BLACK 1
#define SCORE_BABY 2
#define SCORE_BIRD 3
#define SCORE_CACTUS 4
#define SCORE_FLAG 5
#define SCORE_MAX 6

#include <vector>
#include "gameHook.h"
#include "general.h"
#include "glHelp.h"

class AnimatedCollection;
class Map;
class Player;
class Weather;
class EditMode;
class GameHook;
class Gamer;

class Game {
 public:
  Game(const char *, Gamer *gamer);
  explicit Game(Map *editmap);
  virtual ~Game();

  void tick(Real);
  void draw();                               /* Used for normal drawing mode */
  void drawReflection(const Coord3d &focus); /* Used when drawing environmentmaps */
  void doExpensiveComputations();

  void add(GameHook *);
  void queueCall(SCM fun);
  void queueCall(SCM fun, SCM argA);
  void queueCall(SCM fun, SCM argA, SCM argB);
  void loadLevel(const char *level);
  void clearLevel();

  GLfloat fogColor[4];
  int localPlayers, isNight, startTime, currentLevelSet, useGrid;
  double fogThickness, wantedFogThickness;
  double jumpFactor, oxygenFactor;
  int edit_mode;

  Player *player1;
  Map *map;
  Real gameTime;
  Gamer *gamer;
  Weather *weather;

  static Game *current;
  AnimatedCollection *balls;

  char levelName[256];
  char nextLevel[256];
  char returnLevel[256]; /* When returning from bonus levels */

  /** Records current default settings for score/time when killing
      units and triggering other events */
  static double defaultScores[SCORE_MAX][2];

 protected:
  friend class Ball;
  friend class EditMode;
  std::vector<GameHook *> hooks[Role_MaxTypes];

 private:
  typedef struct {
    SCM fun;
    SCM argA;
    SCM argB;
  } QueuedCall;
  std::vector<QueuedCall> queuedCalls;
  std::vector<GameHook *> newHooks;

  void setDefaults();
};

#endif
