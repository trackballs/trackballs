/* teleport.cc
   teleport the ball to the destination

   Copyright (C) 2000-2003  Mathias Broxvall
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

#include "teleport.h"

#include "game.h"
#include "map.h"
#include "player.h"

#define NFACETS 14
#define sinN sin14
#define cosN cos14

Teleport::Teleport(Game &g, Real x, Real y, Real dx, Real dy, Real radius)
    : Animated(g, Role_OtherAnimated, 2) {
  this->x = x;
  this->y = y;
  this->dx = dx;
  this->dy = dy;
  this->radius = radius;
  position[0] = x;
  position[1] = y;
  position[2] = game.map->getHeight(position[0], position[1]);
  primaryColor = Color(SRGBColor(0.5, 0.7, 0.6, 1.0));
  secondaryColor = Color(SRGBColor(1.0, 0.9, 0.4, 1.0));
  is_on = true;
  boundingBox[0][0] = -2 * radius;
  boundingBox[1][0] = 2 * radius;
  boundingBox[0][1] = -2 * radius;
  boundingBox[1][1] = 2 * radius;
  boundingBox[0][2] = -2 * radius;
  boundingBox[1][2] = 2 * radius + 0.5;
}

void Teleport::updateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                             const GLuint *vaolist, bool firstCall) {
  GLfloat cent_height = 0.5f;
  if (firstCall || (primaryColor != bufferRingColor || radius != bufferRadius)) {
    bufferRingColor = primaryColor;
    bufferRadius = radius;
    GLfloat data[(9 * NFACETS + 1) * 8];
    ushort idxs[9 * NFACETS][3];

    GLfloat irad = 4.f / 3.f * radius;
    GLfloat orad = 5.f / 3.f * radius;
    GLfloat width = 0.5f / 3.f * radius;

    // Draw torus
    char *pos = (char *)data;
    for (int i = 0; i < NFACETS; i++) {
      GLfloat nnormal[3] = {0.f, 1.f, 0.f};
      pos += packObjectVertex(pos, sinN[i] * irad, width, cosN[i] * irad + cent_height, 0., 0.,
                              primaryColor, nnormal);
      pos += packObjectVertex(pos, sinN[i] * orad, width, cosN[i] * orad + cent_height, 0., 0.,
                              primaryColor, nnormal);
    }
    for (int i = 0; i < NFACETS; i++) {
      GLfloat snormal[3] = {0.f, -1.f, 0.f};
      pos += packObjectVertex(pos, sinN[i] * irad, -width, cosN[i] * irad + cent_height, 0.,
                              0., primaryColor, snormal);
      pos += packObjectVertex(pos, sinN[i] * orad, -width, cosN[i] * orad + cent_height, 0.,
                              0., primaryColor, snormal);
    }
    for (int i = 0; i < NFACETS; i++) {
      GLfloat inormal[3] = {(GLfloat)-sinN[i], (GLfloat)-cosN[i], 0.f};
      pos += packObjectVertex(pos, sinN[i] * irad, -width, cosN[i] * irad + cent_height, 0.,
                              0., primaryColor, inormal);
      pos += packObjectVertex(pos, sinN[i] * irad, +width, cosN[i] * irad + cent_height, 0.,
                              0., primaryColor, inormal);
    }
    for (int i = 0; i < NFACETS; i++) {
      GLfloat onormal[3] = {(GLfloat)sinN[i], (GLfloat)cosN[i], 0.f};
      pos += packObjectVertex(pos, sinN[i] * orad, -width, cosN[i] * orad + cent_height, 0.,
                              0., primaryColor, onormal);
      pos += packObjectVertex(pos, sinN[i] * orad, width, cosN[i] * orad + cent_height, 0., 0.,
                              primaryColor, onormal);
    }

    for (int k = 0; k < 4; k++) {
      int swap = (k == 2 || k == 0);
      for (int i = 0; i < NFACETS; i++) {
        int base = 2 * k * NFACETS;
        int j = (i + 1) % NFACETS;
        int l = i;
        if (swap) std::swap(l, j);
        idxs[base + 2 * i][0] = base + 2 * l;
        idxs[base + 2 * i][1] = base + 2 * j + 1;
        idxs[base + 2 * i][2] = base + 2 * l + 1;

        idxs[base + 2 * i + 1][0] = base + 2 * l;
        idxs[base + 2 * i + 1][1] = base + 2 * j;
        idxs[base + 2 * i + 1][2] = base + 2 * j + 1;
      }
    }

    // Draw base
    Color white(1., 1., 1., 1.);
    for (int i = 0; i < NFACETS; i++) {
      GLfloat inormal[3] = {(GLfloat)cosN[i], (GLfloat)sinN[i], 0.4f};
      pos += packObjectVertex(pos, radius * cosN[i], radius * sinN[i], 0.1f, 0., 0., white,
                              inormal);
    }
    GLfloat vnormal[3] = {0.f, 0.f, 1.f};
    pos += packObjectVertex(pos, 0., 0., 0.3, 0., 0., white, vnormal);

    for (int i = 0; i < NFACETS; i++) {
      idxs[8 * NFACETS + i][0] = 9 * NFACETS;
      idxs[8 * NFACETS + i][1] = 8 * NFACETS + i;
      idxs[8 * NFACETS + i][2] = 8 * NFACETS + (i + 1) % NFACETS;
    }

    glBindVertexArray(vaolist[0]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
    configureObjectAttributes();
  }

  {
    Color color = secondaryColor;
    color.v[3] *= (0.4f + 0.2f * (GLfloat)frandom());
    GLfloat flat[3] = {0., 0., 0.};
    GLfloat rad = (4.0 / 3.0) * radius;

    GLfloat data[(NFACETS + 1) * 8];
    ushort idxs[NFACETS][3];
    char *pos = (char *)data;
    pos += packObjectVertex(pos, 0., 0., cent_height, 0., 0., color, flat);
    for (int i = 0; i < NFACETS; i++) {
      pos += packObjectVertex(pos, sinN[i] * rad, 0., cosN[i] * rad + cent_height, 0., 0.,
                              color, flat);
      idxs[i][0] = 0;
      idxs[i][1] = i + 1;
      idxs[i][2] = (i + 1) % NFACETS + 1;
    }

    glBindVertexArray(vaolist[1]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[1]);
    glBufferData(GL_ARRAY_BUFFER, (NFACETS + 1) * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NFACETS * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
    configureObjectAttributes();
  }
}

void Teleport::drawBuffers1(const GLuint *vaolist) const {
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  // Transfer
  Matrix4d transform;
  affineMatrix(transform, identity3, position);
  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, transform, specularColor, 0.12, Lighting_Regular);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, 3 * 9 * NFACETS, GL_UNSIGNED_SHORT, (void *)0);
}

void Teleport::drawBuffers2(const GLuint *vaolist) const {
  if (activeView.calculating_shadows) return;

  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  Matrix4d transform;
  affineMatrix(transform, identity3, position);
  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, transform, Color(0., 0., 0., 1.), 0., Lighting_None);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[1]);
  glDrawElements(GL_TRIANGLES, NFACETS * 3, GL_UNSIGNED_SHORT, (void *)0);
}

void Teleport::tick(Real t) {
  Animated::tick(t);

  position[2] = game.map->getHeight(position[0], position[1]);
  Player *p = game.player1;
  Coord3d diff = position - p->position;
  if (length(diff) < p->radius + radius) {
    p->position[0] = dx + .5;
    p->position[1] = dy + .5;
    p->position[2] = game.map->getHeight(p->position[0], p->position[1]) + p->radius;
    // generate a sound for the teleportation
    // playEffect(SFX_TELEPORT);
  }
}
