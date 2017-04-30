/* gamer.h
   Represent a gamer (name etc.)

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

#ifndef GAMER_H
#define GAMER_H

class KnownLevel {
 public:
  char name[64];
  char fileName[64];  
};

class Gamer {
 public:
  Gamer();
  void levelStarted();               // Called when a new level is started, adds to knownLevels
  void save();                       // Saves info to disk
  void update();                     // Reload info from disk
  void playerLoose();                // Called when game is over, updates statistics etc.
  void setDefaults();                // Sets defaults for all values
  void reloadNames();

  char name[20];
  int color;
  int timesPlayed, totalScore, nLevelsCompleted;
  int nNames,textureNum;
  char names[256][20];             // possible names for gamers

  int currentLevelSet;
  int nKnownLevels[256];
  KnownLevel *levels[256];
};


#endif
