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

#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <cstdlib>

int low_memory;

double timeDilationFactor = 1.0;

void generalInit() {}

double frandom() { return (rand() % (1 << 30)) / ((double)(1 << 30)); }

int mymod(int v, int m) {
  int tmp = v % m;
  while (tmp < 0) tmp += m;
  return tmp;
}

int fileExists(char *name) {
  FILE *fp = fopen(name, "rb");
  if (fp) {
    fclose(fp);
    return 1;
  }
  return 0;
}
int dirExists(char *name) {
  DIR *dir = opendir(name);
  if (dir) closedir(dir);
  return dir ? 1 : 0;
}

int pathIsFile(char *path) {
  struct stat m;
  if (lstat(path, &m)) return 0;
  if (S_ISREG(m.st_mode)) return 1;
  return 0;
}
int pathIsDir(char *path) {
  struct stat m;
  if (lstat(path, &m)) return 0;
  if (S_ISDIR(m.st_mode)) return 1;
  return 0;
}
int pathIsLink(char *path) {
  struct stat m;
  if (lstat(path, &m)) return 0;
  if (S_ISLNK(m.st_mode)) return 1;
  return 0;
}

struct timespec getMonotonicTime() {
  struct timespec ts;
  int s = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (s) {
    warning("Failed to acquire monotonic time. %d %d", ts.tv_sec, ts.tv_nsec);
    ts.tv_nsec = 0;
    ts.tv_sec = 0;
  }
  return ts;
}

double getTimeDifference(const struct timespec &from, const struct timespec &to) {
  double elapsed = (to.tv_sec - from.tv_sec) + 1e-9 * (to.tv_nsec - from.tv_nsec);
  return timeDilationFactor * elapsed;
}

void error(const char *formatstr, ...) {
  char errmsg[256];
  va_list(args);
  snprintf(errmsg, 255, "[ERROR] %s\n", formatstr);
  errmsg[255] = '\0';
  va_start(args, formatstr);
  vfprintf(stderr, errmsg, args);
  exit(EXIT_FAILURE);
}
void warning(const char *formatstr, ...) {
  char errmsg[256];
  va_list(args);
  snprintf(errmsg, 255, "[WARNING] %s\n", formatstr);
  errmsg[255] = '\0';
  va_start(args, formatstr);
  vfprintf(stderr, errmsg, args);
  va_end(args);
}

float sRGBToLinear(float v) {
  if (v < 0.04045) {
    return v / 12.92f;
  } else {
    return std::pow(((v + 0.055f) / 1.055f), 2.4f);
  }
}
