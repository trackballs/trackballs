/** \file general.h
   Contains header information for *all* source files in the project.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <math.h>
#include <time.h>
#include <iterator>
#include <sys/time.h>
#include "../gettext.h"
#define _(String) gettext(String)

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

//#include <g++-3/set.h>   //TODO: Fix configure.in
#include <set>  //TODO: Fix configure.in
#include <SDL/SDL.h>
#include <SDL/SDL_mouse.h>
#include <SDL/SDL_ttf.h>
#include "libguile.h"
#include "libguile/numbers.h"  // Eliminate this line if it gives you any troubles
#include <stdint.h>

#ifdef __MINGW32__
#include <io.h>
#define mkdir(a, ...) mkdir(a)
#endif

/* YP: if present, include the 'config.h' */
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#undef SHARE_DIR
extern char effectiveShareDir[];
#define SHARE_DIR effectiveShareDir

//#define SHARE_DIR "/home/matbr/development/trackballs/share"
//#define SHARE_DIR "./share/trackballs"

/*** More includes follows after typedeclarations ***/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef double Real;
typedef double Coord3d[3];
typedef double Matrix4d[4][4];
typedef double Matrix3d[3][3];
typedef GLfloat Color[3];
typedef int Boolean;

#include "glHelp.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif

#ifndef M_PI2
#define M_PI2 2.0 * M_PI
#endif

#define DEBUG(A)        \
  {                     \
    fprintf(stderr, A); \
    fflush(stderr);     \
  }

void generalInit();
double semiRand(int, int);
double semiRand(int, int, int);
double frandom();

int dirExists(char *);
int fileExists(char *);
int pathIsFile(char *path); /** Checks if the given path points to a true file (not links) */
int pathIsDir(
    char *path); /** Checks if the given path points to a true directory (not links) */
int pathIsLink(char *path); /** Checks if the given path points to a link */

/* Attempts to open a file (with given mode) in the following order:
   1. In $HOME/.trackballs/
   2. In ./
   3. In configured share directory
*/
char *locateFile(char *basename, char mode);

/* Sometimes these functions are already defined. If so, it is
   safe to remove them from below. */
/*
int inline min(int a,int b) {return a<b?a:b;}
int inline max(int a,int b) {return a>b?a:b;}
double inline min(double a,double b) {return a<b?a:b;}
double inline max(double a,double b) {return a>b?a:b;}
*/

/* A modulus operations which handles negative results safly */
int mymod(int v, int m);
/* Returns the real time right now measured in seconds. Mostly usefull for debugging and
 * optimizations */
double getSystemTime();

/*** Globals ***/
extern SDL_Surface *screen;
extern int silent;
extern int low_memory;  // if true, attempt to conserve memory
extern int debug_joystick;
extern int repair_joystick;
