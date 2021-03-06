/* highScore.h
   Keeps track of loading / saving highscores

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

#ifndef HIGHSCORE_H
#define HIGHSCORE_H

class HighScore {
 public:
  HighScore();

  static HighScore* init();
  static void cleanup();

  int points[256][10];
  char names[256][10][25];
  int dummy_player[256][10];

  int isHighScore(int levelSet, int points); /* returns true if this is a highscore */
  void addHighScore(int levelSet, int points, char* name);

 private:
  static void* load(void*);
};

#endif
