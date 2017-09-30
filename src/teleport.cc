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

Teleport::Teleport(Real x, Real y, Real dx, Real dy, Real radius) {
  this->x = x;
  this->y = y;
  this->dx = dx;
  this->dy = dy;
  this->radius = radius;
  position[0] = x;
  position[1] = y;
  position[2] = Game::current->map->getHeight(position[0], position[1]);
  primaryColor[0] = .5;
  primaryColor[1] = .7;
  primaryColor[2] = .6;
  primaryColor[3] = 1.0;
  secondaryColor[0] = 1.0;
  secondaryColor[1] = 0.9;
  secondaryColor[2] = 0.4;
  secondaryColor[3] = 1.0;
  is_on = 1;
  boundingBox[0][0] = -2 * radius;
  boundingBox[1][0] = 2 * radius;
  boundingBox[0][1] = -2 * radius;
  boundingBox[1][1] = 2 * radius;
  boundingBox[0][2] = -2 * radius;
  boundingBox[1][2] = 2 * radius + 0.5;
}

int Teleport::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  allocateBuffers(2, idxbufs, databufs);

  GLfloat cent_height = 0.5f;
  {
    GLfloat data[(9 * NFACETS + 1) * 8];
    ushort idxs[9 * NFACETS][3];

    GLfloat irad = 4.f / 3.f * radius;
    GLfloat orad = 5.f / 3.f * radius;
    GLfloat width = 0.5f / 3.f * radius;

    // Draw torus
    char *pos = (char *)data;
    for (int i = 0; i < NFACETS; i++) {
      GLfloat angle = 2 * i * M_PI / NFACETS;
      GLfloat nnormal[3] = {0.f, 1.f, 0.f};
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] + width,
                              position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                              primaryColor, nnormal);
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * orad, position[1] + width,
                              position[2] + std::cos(angle) * orad + cent_height, 0., 0.,
                              primaryColor, nnormal);
    }
    for (int i = 0; i < NFACETS; i++) {
      GLfloat angle = 2 * i * M_PI / NFACETS;
      GLfloat snormal[3] = {0.f, -1.f, 0.f};
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] - width,
                              position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                              primaryColor, snormal);
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * orad, position[1] - width,
                              position[2] + std::cos(angle) * orad + cent_height, 0., 0.,
                              primaryColor, snormal);
    }
    for (int i = 0; i < NFACETS; i++) {
      GLfloat angle = 2 * i * M_PI / NFACETS;
      GLfloat inormal[3] = {-std::sin(angle), -std::cos(angle), 0.f};
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] - width,
                              position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                              primaryColor, inormal);
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] + width,
                              position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                              primaryColor, inormal);
    }
    for (int i = 0; i < NFACETS; i++) {
      GLfloat angle = 2 * i * M_PI / NFACETS;
      GLfloat onormal[3] = {std::sin(angle), std::cos(angle), 0.f};
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * orad, position[1] - width,
                              position[2] + std::cos(angle) * orad + cent_height, 0., 0.,
                              primaryColor, onormal);
      pos += packObjectVertex(pos, position[0] + std::sin(angle) * orad, position[1] + width,
                              position[2] + std::cos(angle) * orad + cent_height, 0., 0.,
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
    GLfloat white[4] = {1.f, 1.f, 1.f, 1.f};
    for (int i = 0; i < NFACETS; i++) {
      GLfloat angle = 2 * i * M_PI / NFACETS;
      GLfloat inormal[3] = {std::cos(angle), std::sin(angle), 0.4f};
      pos += packObjectVertex(pos, position[0] + radius * std::cos(angle),
                              position[1] + radius * std::sin(angle), position[2] + 0.1f, 0.,
                              0., white, inormal);
    }
    GLfloat vnormal[3] = {0.f, 0.f, 1.f};
    pos += packObjectVertex(pos, position[0], position[1], position[2] + 0.3, 0., 0., white,
                            vnormal);

    for (int i = 0; i < NFACETS; i++) {
      idxs[8 * NFACETS + i][0] = 9 * NFACETS;
      idxs[8 * NFACETS + i][1] = 8 * NFACETS + i;
      idxs[8 * NFACETS + i][2] = 8 * NFACETS + (i + 1) % NFACETS;
    }

    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  }
  {
    GLfloat color[4] = {secondaryColor[0], secondaryColor[1], secondaryColor[2],
                        secondaryColor[3] * (0.4f + 0.2f * (GLfloat)frandom())};
    GLfloat flat[3] = {0., 0., 0.};
    GLfloat rad = (4.0 / 3.0) * radius;

    GLfloat data[(NFACETS + 1) * 8];
    ushort idxs[NFACETS][3];
    char *pos = (char *)data;
    pos += packObjectVertex(pos, position[0], position[1], position[2] + cent_height, 0., 0.,
                            color, flat);
    for (int i = 0; i < NFACETS; i++) {
      GLfloat angle = 2 * i * M_PI / NFACETS;

      pos += packObjectVertex(pos, position[0] + std::sin(angle) * rad, position[1],
                              position[2] + std::cos(angle) * rad + cent_height, 0., 0., color,
                              flat);
      idxs[i][0] = 0;
      idxs[i][1] = i + 1;
      idxs[i][2] = (i + 1) % NFACETS + 1;
    }

    glBindBuffer(GL_ARRAY_BUFFER, databufs[1]);
    glBufferData(GL_ARRAY_BUFFER, (NFACETS + 1) * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, NFACETS * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
  }

  return 2;
}

void Teleport::drawBuffers1(GLuint *idxbufs, GLuint *databufs) {
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  // Transfer
  setActiveProgramAndUniforms(shaderObject);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0],
              specularColor[1], specularColor[2], specularColor[3]);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 15.f / 128.f);
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 3 * 9 * NFACETS, GL_UNSIGNED_SHORT, (void *)0);
}

void Teleport::drawBuffers2(GLuint *idxbufs, GLuint *databufs) {
  if (activeView.calculating_shadows) return;

  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  setActiveProgramAndUniforms(shaderObject);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0., 0., 0., 1.);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.);
  glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), -1.);
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[1]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[1]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, NFACETS * 3, GL_UNSIGNED_SHORT, (void *)0);
}

void Teleport::tick(Real /*t*/) {
  position[2] = Game::current->map->getHeight(position[0], position[1]);
  Player *p = Game::current->player1;
  Coord3d diff;

  sub(position, p->position, diff);
  if (length(diff) < p->radius + radius) {
    p->position[0] = dx + .5;
    p->position[1] = dy + .5;
    p->position[2] = Game::current->map->getHeight(p->position[0], p->position[1]);
    // generate a sound for the teleportation
    // playEffect(SFX_TELEPORT);
  }
}
