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

#include <libintl.h>
#include <stdio.h>
#define _(String) gettext(String)

#ifdef __MINGW32__
#include <io.h>
#define mkdir(a, ...) mkdir(a)
#endif

extern char effectiveShareDir[];

/*** More includes follows after typedeclarations ***/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef double Real;
typedef double Coord3d[3];
typedef double Matrix4d[4][4];
typedef double Matrix3d[3][3];
typedef int Boolean;

#ifndef M_PI
#define M_PI 3.14159265
#endif

#ifndef M_PI2
#define M_PI2 2.0 * M_PI
#endif

void generalInit();
double semiRand(int, int);
double semiRand(int, int, int);
double frandom();

int fileExists(char *);
int dirExists(char *);
int pathIsFile(char *path); /** Checks if the given path points to a true file (not links) */
int pathIsDir(
    char *path); /** Checks if the given path points to a true directory (not links) */
int pathIsLink(char *path); /** Checks if the given path points to a link */

/* A modulus operations which handles negative results safly */
int mymod(int v, int m);
/* Returns the real time right now measured in seconds. Mostly usefull for debugging and
 * optimizations */
double getSystemTime();

/* Printfs for when things go wrong. Errors are fatal. _Never_ translate input. */
void error(const char *formatstr, ...) __attribute__((__noreturn__));
void warning(const char *formatstr, ...);

/*** Globals ***/
extern int low_memory;  // if true, attempt to conserve memory
extern int debug_joystick;
extern int repair_joystick;
