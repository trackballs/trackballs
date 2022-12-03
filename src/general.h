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

#ifndef GENERAL_H
#define GENERAL_H

#include <libintl.h>
#include <stdint.h>
#include <stdio.h>
#include <cmath>
#define _(String) gettext(String)

#ifdef __MINGW32__
#include <io.h>
#define mkdir(a, ...) mkdir(a)
#endif

extern char effectiveShareDir[], effectiveLocalDir[], username[];

/*** More includes follows after typedeclarations ***/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef double Real;
typedef Real Matrix4d[4][4];
typedef Real Matrix3d[3][3];
typedef int ICoord2d[2];

// Simple vector type to make geometry code readable
class Coord3d {
 public:
  inline Coord3d() {
    data[0] = 0.;
    data[1] = 0.;
    data[2] = 0.;
  }
  explicit inline Coord3d(double x[3]) {
    data[0] = x[0];
    data[1] = x[1];
    data[2] = x[2];
  }
  inline Coord3d(double x, double y, double z) {
    data[0] = x;
    data[1] = y;
    data[2] = z;
  };
  inline double &operator[](size_t idx) { return data[idx]; }
  inline double operator[](size_t idx) const { return data[idx]; }
  inline Coord3d operator+(const Coord3d &other) const {
    return Coord3d(this->data[0] + other.data[0], this->data[1] + other.data[1],
                   this->data[2] + other.data[2]);
  }
  inline Coord3d operator-(const Coord3d &other) const {
    return Coord3d(this->data[0] - other.data[0], this->data[1] - other.data[1],
                   this->data[2] - other.data[2]);
  }
  inline Coord3d operator-() const {
    return Coord3d(-this->data[0], -this->data[1], -this->data[2]);
  }
  inline Coord3d operator*(double scale) const {
    return Coord3d(this->data[0] * scale, this->data[1] * scale, this->data[2] * scale);
  }
  inline Coord3d operator/(double scale) const {
    return Coord3d(this->data[0] / scale, this->data[1] / scale, this->data[2] / scale);
  }

 private:
  Real data[3];
};
inline Coord3d operator*(double scale, const Coord3d &vec) { return vec * scale; }

class SRGBColor {
 public:
  SRGBColor() {
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    w[3] = 65535;
  }
  SRGBColor(float fr, float fg, float fb, float fa) {
    w[0] = std::round(65535.f * fr);
    w[1] = std::round(65535.f * fg);
    w[2] = std::round(65535.f * fb);
    w[3] = std::round(65535.f * fa);
  };
  float f0() const { return w[0] / 65535.f; }
  float f1() const { return w[1] / 65535.f; }
  float f2() const { return w[2] / 65535.f; }
  float f3() const { return w[3] / 65535.f; }
  uint16_t w[4];
};

float sRGBToLinear(float v);

class Color {
 public:
  Color() {
    v[0] = 0.f;
    v[1] = 0.f;
    v[2] = 0.f;
    v[3] = 1.f;
  }
  Color(float fr, float fg, float fb, float fa) {
    v[0] = fr;
    v[1] = fg;
    v[2] = fb;
    v[3] = fa;
  };
  explicit Color(SRGBColor c) {
    v[0] = sRGBToLinear(c.f0());
    v[1] = sRGBToLinear(c.f1());
    v[2] = sRGBToLinear(c.f2());
    v[3] = c.f3();
  }
  Color toOpaque() const {
    Color c(*this);
    c.v[3] = 1.f;
    return c;
  }
  static Color mix(float blend, const Color &col0, const Color &col1) {
    Color c;
    for (int i = 0; i < 4; i++) { c.v[i] = col0.v[i] * (1. - blend) + col1.v[i] * blend; }
    return c;
  }
  float f0() const { return v[0]; }
  float f1() const { return v[1]; }
  float f2() const { return v[2]; }
  float f3() const { return v[3]; }
  float v[4];
  bool operator==(const Color &b) const {
    return v[0] == b.v[0] && v[1] == b.v[1] && v[2] == b.v[2] && v[3] == b.v[3];
  }
  bool operator!=(const Color &b) const { return !(*this == b); }
};

#ifndef M_PI
#define M_PI 3.14159265358979323846264
#endif

#ifndef M_PI2
#define M_PI2 2.0 * M_PI
#endif

void generalInit();
double frandom();

int fileExists(char *);
int dirExists(char *);
int pathIsFile(char *path); /** Checks if the given path points to a true file (not links) */
int pathIsDir(
    char *path); /** Checks if the given path points to a true directory (not links) */
int pathIsLink(char *path); /** Checks if the given path points to a link */

/* A modulus operations which handles negative results safely */
int mymod(int v, int m);
/* Return the current time, and calculate elapsed time in seconds */
extern double timeDilationFactor;
struct timespec getMonotonicTime();
double getTimeDifference(const struct timespec &from, const struct timespec &to);

/* Printfs for when things go wrong. Errors are fatal. _Never_ translate input. */
void error(const char *formatstr, ...) __attribute__((__noreturn__));
void warning(const char *formatstr, ...);

/*** Globals ***/
extern int low_memory;  // if true, attempt to conserve memory
extern int debug_joystick;
extern int repair_joystick;
extern int displayFrameNumber;

#endif /*GENERAL_H*/
