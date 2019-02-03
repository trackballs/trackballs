/* sparkle2d.h
   manage 2D sparkles (in 2D mode, of course)

   Copyright (C) 2000  Mathias Broxvall
                       Yannick Perret

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

#ifndef SPARKLE2D_H
#define SPARKLE2D_H

#include "glHelp.h"

// There are typically ~30 sparkles
#define MAX_DRAWN_SPARKLES 64

/* structure of a sparkle */
typedef struct _sparkle {
  float pos[3];
  float speed[3];
  float color[4];
  float size;
  float age;
  float ttl;
  struct _sparkle *next, *prev;
} Sparkle;

class Sparkle2D {
 public:
  Sparkle2D();
  ~Sparkle2D();

  // update sparkles
  void tick(Real t);
  // draw sparkles
  void draw();
  // add a new sparkle
  void add(float pos[2], float speed[2], float ttl, float size, float color[4]);
  // remove all current sparkles (i.e. before changing screen)
  void clear();
  static Sparkle2D *sparkle2D;

 private:
  // add a sparkle in the list
  Sparkle *create_and_insert();
  // remove a specific sparkle
  void remove_sparkle(Sparkle *sparkle);
  // list of sparkles
  Sparkle *sparkle_first;
  int nsparkles;

  GLuint vao;
  GLuint idxs_buf;
  GLuint data_buf;

  GLfloat data[8 * 4 * MAX_DRAWN_SPARKLES];
};

#endif
