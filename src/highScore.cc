/** \file highScore.cc
   Saves and loads the highscore lists
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
#include "highScore.h"
#include <string>
#include "settings.h"
#include "game.h"

using namespace std;
char highScorePath[256];

HighScore *HighScore::highScore;
HighScore::HighScore() {
  int i,j;

  for(int levelSet=0;levelSet<Settings::settings->nLevelSets;levelSet++)
	for(i=0;i<10;i++) {
	  points[levelSet][i] = 1000 - i * 100;
	  snprintf(names[levelSet][i],sizeof(names[levelSet][i]),"Anonymous Coward");
	}

#ifdef ALT_HIGHSCORES

  /* Copy ALT_HIGHSCORES into highScorePath, substituting the '~' character for the HOME directory */
  if(ALT_HIGHSCORES[0] == '~') {
    if(ALT_HIGHSCORES[1] == 0) 
      snprintf(highScorePath,sizeof(highScorePath),"%s/.trackballs/highscores",getenv("HOME"));
    else if(ALT_HIGHSCORES[1] == '/')
      snprintf(highScorePath,sizeof(highScorePath),"%s%s",getenv("HOME"),&ALT_HIGHSCORES[1]);
    else { printf("Error, bad ALT_HIGHSCORES compiled into game '%s'\n",ALT_HIGHSCORES); exit(0); }
  } else {
    snprintf(highScorePath,sizeof(highScorePath),"%s",ALT_HIGHSCORES);
  }

#else
  snprintf(highScorePath,sizeof(highScorePath),"%s/highScores",SHARE_DIR);
#endif
  if(pathIsLink(highScorePath)) {
	fprintf(stderr,_("Error, %s is a symbolic link. Cannot save highscores\n"),highScorePath);
	return;
  }
  gzFile gp = gzopen(highScorePath,"rb");
  if(!gp) { printf("Warning. Could not find highscore file %s\n",highScorePath); return; }
  int nLevelSets,levelSet;
  Settings *settings=Settings::settings;
  gzread(gp,&nLevelSets,sizeof(int));
  for(;nLevelSets;nLevelSets--) {
	char name[256];
	gzread(gp,name,sizeof(char)*256);
	for(levelSet=0;levelSet<settings->nLevelSets;levelSet++)
	  if(strcmp(name,settings->levelSets[levelSet].name) == 0) break;
	if(levelSet == settings->nLevelSets) {
	  printf("Warning. Highscores contains info about unknown levelset %s\n",name);
	  gzclose(gp); return;
	}
	gzread(gp,&(points[levelSet][0]),sizeof(int)*10);
	gzread(gp,&(names[levelSet][0][0]),sizeof(char)*10*25);
  }
  gzclose(gp);
}
void HighScore::init() {highScore = new HighScore();}
int HighScore::isHighScore(int score) { 
  if(Game::current->currentLevelSet < 0) return 0;
  return score > points[Game::current->currentLevelSet][9]; 
}
void HighScore::addHighScore(int score,char *name) {
  int i,j;
  int levelSet=Game::current->currentLevelSet;
  if(levelSet < 0) return;

  for(i=0;i<10;i++) if(score > points[levelSet][i]) break;
  if(i < 10) {
	for(j=9;j>i;j--) {
	  strcpy(names[levelSet][j],names[levelSet][j-1]);
	  points[levelSet][j] = points[levelSet][j-1];
	}
	strcpy(names[levelSet][i],name);
	points[levelSet][i] = score;
  }

  if(pathIsLink(highScorePath)) {
	fprintf(stderr,_("Error, %s is a symbolic link. Cannot save highscores\n"),highScorePath);
	return;
  }

  Settings *settings=Settings::settings;
  gzFile gp = gzopen(highScorePath,"wb9");
  if(!gp) { printf("Warning. Cannot save highscores at %s, check file permissions\n",highScorePath); return; }
  gzwrite(gp,&(Settings::settings->nLevelSets),sizeof(int));
  for(levelSet=0;levelSet<Settings::settings->nLevelSets;levelSet++) {
	gzwrite(gp,&settings->levelSets[levelSet].name,256);
	gzwrite(gp,&(points[levelSet][0]),sizeof(int)*10);
	gzwrite(gp,&(names[levelSet][0][0]),sizeof(char)*10*25);
  }
  gzclose(gp);
}
