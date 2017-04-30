/* general.cc
   Some quite general utility algorithms.

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
#include "gameMode.h"
#include "menuMode.h"
#include "SDL/SDL_opengl.h"
#include "font.h"
#include "glHelp.h"
#include "SDL/SDL_image.h"
#include <sys/time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;

int randData1[4711];
int randData2[4827];
int randData3[5613];
int low_memory;

void generalInit() {
  int i;
  for(i=0;i<4711;i++) randData1[i] = rand() % 65536;
  for(i=0;i<4827;i++) randData2[i] = rand() % 65536;
  for(i=0;i<5613;i++) randData3[i] = rand() % 65536;
}
double semiRand(int x,int y) {
  return (randData2[(randData1[x%4711]+y)%4827])/65536.0;
}
double semiRand(int x,int y,int z) {
  return randData3[(randData2[(randData1[x%4711]+y)%4827]+z)%5613]/65536.0;
}
double frandom() { return (rand() % (1<<30)) / ((double) (1<<30)); }

int mymod(int v,int m) {
  int tmp=v % m;
  while(tmp < 0) tmp += m;
  return tmp;
}

int fileExists(char *name) {
  FILE *fp = fopen(name,"rb");
  if(fp) { fclose(fp); return 1; }
  return 0;
}
int dirExists(char *name) {
  DIR *dir=opendir(name);
  if(dir) closedir(dir);
  return dir ? 1 : 0;
}

int pathIsFile(char *path) {
  struct stat m;
  if(lstat(path,&m)) return 0;
  if(S_ISREG(m.st_mode)) return 1;
  return 0;
}
int pathIsDir(char *path) {
  struct stat m;
  if(lstat(path,&m)) return 0;
  if(S_ISDIR(m.st_mode)) return 1;
  return 0;
}
int pathIsLink(char *path) {
  struct stat m;
  if(lstat(path,&m)) return 0;
  if(S_ISLNK(m.st_mode)) return 1;
  return 0;
}


/* Note. this function is currently not used anywhere */
char *locateFile(char *basename,char *mode) {
  FILE *fp;
  static char str[512];
  char *home = getenv("HOME");

  /* First attempt $HOME/.trackballs */
  if(home) {
	snprintf(str,511,"%s/.trackballs/%s",home,basename);
	fp = fopen(str,mode);
	if(fp) { fclose(fp); return str; }
  }

  /* Next at the current dir */
  snprintf(str,511,"./%s",basename);
  fp = fopen(str,mode);
  if(fp) { fclose(fp); return str; }
  
  /* Lastly, in the configured share directory */
  snprintf(str,511,"%s/%s",effectiveShareDir,basename);
  fp = fopen(str,mode);
  if(fp) { fclose(fp); return str; }

  fprintf(stderr,"Warning, failed to open resource file %s in mode '%s'\n",basename,mode);
  return ""; 
}

/* Returns the real time right now measured in seconds. Mostly useful for debugging and optimizations */
double getSystemTime() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec + 1e-6 * tv.tv_usec;  
}
