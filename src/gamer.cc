/* gamer.cc
   Represents a gamer

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
#include "gamer.h"
#include "settings.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
using namespace std;

Gamer::Gamer() {
  strcpy(name,"John Doe");
  if(NULL != getenv("USER")){
    snprintf(name,20,"%s",getenv("USER"));
  }
#ifdef WIN32
  else{
    if(NULL != getenv("USERNAME")){
      snprintf(name,20,"%s",getenv("USERNAME"));
    }
  }
#endif

  for(int i=0;i<Settings::settings->nLevelSets;i++) 
	levels[i] = new KnownLevel[256];

  setDefaults();
  update();
  reloadNames();
  if(nNames>0) strncpy(name,names[0],20);
  update();
}
void Gamer::setDefaults() {
  color=0;
  totalScore=0;
  nLevelsCompleted=0;
  timesPlayed=0;
  nLevelsCompleted=0;
  for(int i=0;i<Settings::settings->nLevelSets;i++) {
	nKnownLevels[i]=1;
	strcpy(levels[i][0].name,Settings::settings->levelSets[i].startLevelName);
	strcpy(levels[i][0].fileName,Settings::settings->levelSets[i].startLevel);
  }
  textureNum=0;
}
void Gamer::levelStarted() {
  int i;
  char *level = Game::current->levelName;
  if(Game::current->currentLevelSet < 0) return;         // don't modify profile when cheating
  if(Game::current->map->isBonus) return; // bonus levels are not added to known levels 
  for(i=0;i<nKnownLevels[Game::current->currentLevelSet];i++)
	if(strcmp(levels[Game::current->currentLevelSet][i].fileName,level) == 0) break;
  if(i == nKnownLevels[Game::current->currentLevelSet]) {
	strncpy(levels[Game::current->currentLevelSet][i].fileName,level,64);
	strncpy(levels[Game::current->currentLevelSet][i].name,Game::current->map->mapname,64);
	nKnownLevels[Game::current->currentLevelSet]++;
	nLevelsCompleted++;
	save();
  }
}

/* Note. save/update need not use a platform independent format since the save
   files are meant to be local. */
void Gamer::save() {
  char str[256];
  int version=3;
  int levelSet;

  Settings *settings=Settings::settings;

  snprintf(str,sizeof(str)-1,"%s/.trackballs",getenv("HOME"));
  if(pathIsLink(str)) { 
	fprintf(stderr,_("Error, %s/.trackballs is a symbolic link. Cannot save settings\n"),getenv("HOME"));
	return;
  }
  if(!pathIsDir(str)) 
	mkdir(str,S_IXUSR|S_IRUSR|S_IWUSR|S_IXGRP|S_IRGRP|S_IWGRP);
  snprintf(str,sizeof(str)-1,"%s/.trackballs/%s.gmr",getenv("HOME"),name);
  if(pathIsLink(str)) { 
	fprintf(stderr,_("Error, %s/.trackballs/%s.gmr is a symbolic link. Cannot save settings\n"),getenv("HOME"),name);
	return;
  }

  gzFile gp = gzopen(str,"wb9");
  if(gp) {
	gzwrite(gp,&version,sizeof(int));
	gzwrite(gp,&color,sizeof(int));
	gzwrite(gp,&totalScore,sizeof(int));
	gzwrite(gp,&nLevelsCompleted,sizeof(int));
	gzwrite(gp,&timesPlayed,sizeof(int));
	gzwrite(gp,&textureNum,sizeof(int));
	gzwrite(gp,&(Settings::settings->difficulty),sizeof(int));
	gzwrite(gp,&(Settings::settings->nLevelSets),sizeof(int));
	for(levelSet=0;levelSet<Settings::settings->nLevelSets;levelSet++) {
	  gzwrite(gp,&settings->levelSets[levelSet].name,256);
	  gzwrite(gp,&nKnownLevels[levelSet],sizeof(int));
	  gzwrite(gp,levels[levelSet],sizeof(KnownLevel)*nKnownLevels[levelSet]);
	}
	gzwrite(gp,&(Settings::settings->sandbox),sizeof(int));
	gzclose(gp);
  }
}
void Gamer::update() {
  char str[256];
  int version;
  
  Settings *settings=Settings::settings;
  snprintf(str,sizeof(str)-1,"%s/.trackballs/%s.gmr",getenv("HOME"),name);
  gzFile gp = gzopen(str,"rb");
  if(!gp) { setDefaults(); return; }
  gzread(gp,&version,sizeof(int));
  if(version < 2) {printf("Warning. Ignoring outdated player profile for player %s\n",name); return;}
  gzread(gp,&color,sizeof(int));
  gzread(gp,&totalScore,sizeof(int));
  gzread(gp,&nLevelsCompleted,sizeof(int));
  gzread(gp,&timesPlayed,sizeof(int));
  gzread(gp,&textureNum,sizeof(int));
  gzread(gp,&(Settings::settings->difficulty),sizeof(int));
  int nLevelSets,levelSet;
  gzread(gp,&nLevelSets,sizeof(int));
  for(;nLevelSets;nLevelSets--) {
	char name[256];
	gzread(gp,name,256);
	for(levelSet=0;levelSet<settings->nLevelSets;levelSet++)
	  if(strcmp(name,settings->levelSets[levelSet].name) == 0) break;
	if(levelSet == settings->nLevelSets) {
	  printf("Error: Profile for %s contains info for unknown levelset %s\n",str,name);
	  gzclose(gp);
	  return;
	}
	gzread(gp,&nKnownLevels[levelSet],sizeof(int));
	gzread(gp,levels[levelSet],sizeof(KnownLevel)*nKnownLevels[levelSet]);   
  }
  if(version >= 3) {
    gzread(gp,&(Settings::settings->sandbox),sizeof(int));
  }
  gzclose(gp);
}
void Gamer::playerLoose() {
  timesPlayed++;
  totalScore += Game::current->player1->score;
  save();
}
void Gamer::reloadNames() {
  char str[256];
  int i;

  nNames=0;
  snprintf(str,sizeof(str)-1,"%s/.trackballs",getenv("HOME"));
  DIR *dir = opendir(str);
  if(dir) {
	struct dirent *dirent;
	while((dirent = readdir(dir))) {
	  if(strlen(dirent->d_name) > 4 && strcmp(&dirent->d_name[strlen(dirent->d_name)-4],".gmr") == 0) {
		for(i=0;i<strlen(dirent->d_name)-4;i++)
		  names[nNames][i] = dirent->d_name[i];
		names[nNames][i] = 0;
		nNames++;
	  }
	}
	closedir(dir);
  }
}
