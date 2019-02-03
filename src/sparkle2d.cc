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

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glGenBuffers(1, &idxs_buf);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs_buf);
  glGenBuffers(1, &data_buf);
  glBindBuffer(GL_ARRAY_BUFFER, data_buf);

  ushort *idxs = new ushort[6 * MAX_DRAWN_SPARKLES];
  for (int k = 0; k < MAX_DRAWN_SPARKLES; k++) {
    idxs[k * 6 + 0] = 4 * k + 0;
    idxs[k * 6 + 1] = 4 * k + 1;
    idxs[k * 6 + 2] = 4 * k + 2;
    idxs[k * 6 + 3] = 4 * k + 1;
    idxs[k * 6 + 4] = 4 * k + 2;
    idxs[k * 6 + 5] = 4 * k + 3;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs_buf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_DRAWN_SPARKLES * 6 * sizeof(ushort), idxs,
               GL_STATIC_DRAW);
  delete[] idxs;

  glBufferData(GL_ARRAY_BUFFER, MAX_DRAWN_SPARKLES * 8 * 4 * sizeof(GLfloat), NULL,
               GL_STREAM_DRAW);

  configureUIAttributes();
}

// destroy
Sparkle2D::~Sparkle2D() {
  // remove all remaining sparkle
  clear();

  glDeleteBuffers(1, &idxs_buf);
  glDeleteBuffers(1, &data_buf);
  glDeleteVertexArrays(1, &vao);
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
  Sparkle *skl = sparkle_first;
  for (int i = 0; skl != NULL; i++, skl = skl->next) {
    if (i >= MAX_DRAWN_SPARKLES) { continue; }
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
      data[base + 0] = skl->pos[0] - ex + offsets[k][0] * (2 * ex);
      data[base + 1] = skl->pos[1] - ey + offsets[k][1] * (2 * ey);
      data[base + 2] = skl->color[0];
      data[base + 3] = skl->color[1];
      data[base + 4] = skl->color[2];
      data[base + 5] = skl->color[3] * alpha;
      data[base + 6] = (GLfloat)offsets[k][0];
      data[base + 7] = (GLfloat)offsets[k][1];
    }
  }

  int ndrawn = nsparkles > MAX_DRAWN_SPARKLES ? MAX_DRAWN_SPARKLES : nsparkles;

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, data_buf);
  glBufferSubData(GL_ARRAY_BUFFER, 0, ndrawn * 8 * 4 * sizeof(GLfloat), data);
  glBindTexture(GL_TEXTURE_2D, textureGlitter);
  glDrawElements(GL_TRIANGLES, 6 * ndrawn, GL_UNSIGNED_SHORT, (void *)0);
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
