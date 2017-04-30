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

#ifndef GAMEMODE_H
#include "gameMode.h"
#endif

class MainMode : public GameMode {
 public:
  MainMode();
  ~MainMode();

  void display();
  void key(int);
  void special(int,int,int);
  void idle(Real td);
  void doExpensiveComputations();
  void mouse(int state,int x,int y);
  void mouseDown(int button,int x,int y);

  void activated();
  void deactivated();

  void playerLoose();
  void playerDie();
  void startGame();
  void restartPlayer();
  void showInfo();
  void showBonus();
  void levelComplete();
  void bonusLevelComplete();

  void renderEnvironmentTexture(GLuint texture,Coord3d focus);
  void convertToFisheye(char *convertedData,char *originalData,int originalSize);
  Real flash;

  static void init();
  static MainMode *mainMode;

  double zAngle, wantedZAngle, xyAngle, wantedXYAngle;  
  Coord3d camFocus, camDelta;
 protected:
 private:
  int gameStatus;
  double statusCount;
  double time;
  double pause_time;
  int go_to_pause;  

  static const int statusBeforeGame,statusGameOver,statusInGame;
  static const int statusRestartPlayer,statusNextLevel,statusVictory;
  static const int statusLevelComplete,statusBonusLevelComplete;
  static const int statusPaused;

  //static SDL_Surface *panel,*life,*nolife;
  static char *environmentTextureData, *viewportData;

  char *mapname;
};

#endif
