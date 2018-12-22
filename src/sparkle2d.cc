/* sparkle2d.cc
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

#include "sparkle2d.h"

#include "glHelp.h"

// initialize sparkle module
Sparkle2D::Sparkle2D() {
  // just be sure that this field is empty
  this->sparkle_first = NULL;
  nsparkles = 0;

  glGenBuffers(1, &idxs);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);
  glGenBuffers(1, &data);
  glBindBuffer(GL_ARRAY_BUFFER, data);
}

// destroy
Sparkle2D::~Sparkle2D() {
  // remove all remaining sparkle
  clear();

  glDeleteBuffers(1, &idxs);
  glDeleteBuffers(1, &data);
}

// remove all entries
void Sparkle2D::clear() {
  Sparkle *tmp = sparkle_first;
  while (tmp != NULL) {
    Sparkle *tmp2 = tmp->next;
    delete tmp;
    tmp = tmp2;
  }
  sparkle_first = NULL;
}

// remove a particular sparkle from the list
void Sparkle2D::remove_sparkle(Sparkle *sparkle) {
  if (sparkle == NULL) return;
  if (sparkle->next != NULL) sparkle->next->prev = sparkle->prev;
  if (sparkle->prev != NULL)
    sparkle->prev->next = sparkle->next;
  else
    sparkle_first = sparkle->next;
  delete sparkle;
  nsparkles--;
}

/*
 * create and insert a glitter
 */
Sparkle *Sparkle2D::create_and_insert() {
  Sparkle *tmp = new Sparkle;
  if (tmp == NULL) { return (NULL); }
  tmp->next = sparkle_first;
  tmp->prev = NULL;
  if (sparkle_first != NULL) { sparkle_first->prev = tmp; }
  sparkle_first = tmp;
  nsparkles++;
  return (tmp);
}

void Sparkle2D::add(float pos[2], float speed[2], float ttl, float size, float color[4]) {
  Sparkle *sparkle;

  if ((sparkle = create_and_insert()) == NULL) return;

  sparkle->pos[0] = pos[0];
  sparkle->pos[1] = pos[1];
  sparkle->speed[0] = speed[0];
  sparkle->speed[1] = speed[1];
  sparkle->color[0] = color[0];
  sparkle->color[1] = color[1];
  sparkle->color[2] = color[2];
  sparkle->color[3] = color[3];
  sparkle->size = size;
  sparkle->ttl = ttl;
  sparkle->age = 0.;
}
/*
 * draw existing glitters
 */
void Sparkle2D::draw() {
  Require2DMode();
  GLfloat *datl = new GLfloat[nsparkles * 8 * 4];
  ushort *idxl = new ushort[nsparkles * 6];

  Sparkle *skl = sparkle_first;
  int i = 0;
  for (int i = 0; skl != NULL; i++, skl = skl->next) {
    float age = skl->age;
    float tmp;
    if (age < 0.1) {
      tmp = 1. - age * 10.;
    } else
      tmp = age / skl->ttl;
    float alpha = 1.0 - tmp;
    tmp = alpha * alpha;
    float ex = tmp * skl->size * 0.8;
    float ey = tmp * skl->size * 1. + (0.02 * age);

    // 2x vert, 4x color, 2x texco
    int offsets[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    for (int k = 0; k < 4; k++) {
      int base = i * 8 * 4 + k * 8;
      datl[base + 0] = skl->pos[0] - ex + offsets[k][0] * (2 * ex);
      datl[base + 1] = skl->pos[1] - ey + offsets[k][1] * (2 * ey);
      datl[base + 2] = skl->color[0];
      datl[base + 3] = skl->color[1];
      datl[base + 4] = skl->color[2];
      datl[base + 5] = skl->color[3] * alpha;
      datl[base + 6] = (GLfloat)offsets[k][0];
      datl[base + 7] = (GLfloat)offsets[k][1];
    }
  }
  for (int i = 0; i < nsparkles; i++) {
    idxl[i * 6 + 0] = 4 * i + 0;
    idxl[i * 6 + 1] = 4 * i + 1;
    idxl[i * 6 + 2] = 4 * i + 2;
    idxl[i * 6 + 3] = 4 * i + 1;
    idxl[i * 6 + 4] = 4 * i + 2;
    idxl[i * 6 + 5] = 4 * i + 3;
  }

  // Input structure: 2x Position; 4x color; 2x texture coord
  glBindBuffer(GL_ARRAY_BUFFER, data);
  glBufferData(GL_ARRAY_BUFFER, nsparkles * 8 * 4 * sizeof(GLfloat), datl, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nsparkles * 6 * sizeof(ushort), idxl, GL_STATIC_DRAW);

  glBindTexture(GL_TEXTURE_2D, textureGlitter);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(2 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(6 * sizeof(GLfloat)));
  glDrawElements(GL_TRIANGLES, 6 * nsparkles, GL_UNSIGNED_SHORT, (void *)0);

  delete[] datl;
  delete[] idxl;
}

/*
 * update existing glitters
 */
void Sparkle2D::tick(Real t) {
  Sparkle *skl = sparkle_first;
  while (skl != NULL) {
    skl->age += t;

    if (skl->age > skl->ttl) {
      Sparkle *tmp = skl;
      skl = skl->next;
      remove_sparkle(tmp);
      continue;
    }
    /* update pos */
    skl->pos[0] += skl->speed[0] * t;
    skl->pos[1] += skl->speed[1] * t;
    skl->speed[1] += 200.0 * t;  // MB. fix for varying framerates

    skl = skl->next;
  }
}
