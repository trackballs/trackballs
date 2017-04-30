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

class Game {
 public:
  Game(char *,class Gamer *gamer);
  virtual ~Game();

  void tick(Real);
  void draw();                                   /* Used for normal drawing mode */
  void drawReflection(Coord3d focus);            /* Used when drawing environmentmaps */
  void doExpensiveComputations();
  void add(class Animated *);
  void add(class GameHook *);
  void remove(class Animated *);
  void remove(class GameHook *);
  void loadLevel(char *level);
  void clearLevel();

  GLfloat fogColor[4];
  int localPlayers,isNight,startTime,currentLevelSet,useGrid;
  double fogThickness,wantedFogThickness;
  double restartBonusTime;
  double jumpFactor,oxygenFactor;

  class Player *player1;
  class Map *map;
  Real gameTime;
  class Gamer *gamer;
  class Weather *weather;

  static Game *current;

  char levelName[256];
  char nextLevel[256]; 
  char returnLevel[256]; /* When returning from bonus levels */

 protected:
 private:
  void setDefaults();

  class std::set<class Animated *> *objects;
  class std::set<class GameHook *> *hooks;
};

#endif
