/* mainMode.h
   The main operation mode of the game.

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

#ifndef MAINMODE_H
#define MAINMODE_H

#include "gameMode.h"
#include "glHelp.h"

class Gamer;

class MainMode : public GameMode {
 public:
  MainMode();
  virtual ~MainMode();

  void display() const;
  void key(int);
  void tick(Real td);
  void doExpensiveComputations();
  void mouse(int state, int x, int y);
  void mouseDown(int button, int x, int y);

  void activated();
  void deactivated();

  void playerLose();
  void playerDie();
  void startGame();
  void restartPlayer();
  void showInfo() const;
  void showBonus() const;
  void levelComplete();
  void bonusLevelComplete();

  static void setupLighting(const Game *game, bool isNight);
  void renderEnvironmentTexture(GLuint texture, const Coord3d &focus) const;
  Real flash;

  static MainMode *init();
  static void cleanup();
  static MainMode *mainMode;

  Game *game;

  double zAngle, wantedZAngle, xyAngle, wantedXYAngle;
  Coord3d camFocus, camDelta;
  void getCamera(Matrix4d modelView, Matrix4d projection) const;

 protected:
 private:
  double statusCount;
  double time;
  double pause_time;
  int go_to_pause;

  enum {
    statusBeforeGame,
    statusGameOver,
    statusInGame,
    statusRestartPlayer,
    statusNextLevel,
    statusVictory,
    statusLevelComplete,
    statusBonusLevelComplete,
    statusPaused
  } gameStatus;
};

#endif
