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

#include "general.h"
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "teleport.h"
#include "ball.h"
#include "player.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"

using namespace std;

Teleport::Teleport(int x, int y, int dx, int dy, Real radius) {
  this->x = x;
  this->y = y;
  this->dx = dx;
  this->dy = dy;
  this->radius = radius;
  position[0] = x + 0.5;
  position[1] = y + 0.5;
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
}

void Teleport::draw() {
  glPushAttrib(GL_ENABLE_BIT);

  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  const int nfacets = 14;
  GLfloat data[(9 * nfacets + 1) * 8];
  ushort idxs[9 * nfacets][3];

  GLfloat irad = 4.f / 3.f * radius;
  GLfloat orad = 5.f / 3.f * radius;
  GLfloat width = 0.5f / 3.f * radius;
  GLfloat cent_height = 0.5f;

  // Draw torus
  char *pos = (char *)data;
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
    GLfloat nnormal[3] = {0.f, 1.f, 0.f};
    pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] + width,
                            position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                            primaryColor, nnormal);
    pos += packObjectVertex(pos, position[0] + std::sin(angle) * orad, position[1] + width,
                            position[2] + std::cos(angle) * orad + cent_height, 0., 0.,
                            primaryColor, nnormal);
  }
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
    GLfloat snormal[3] = {0.f, -1.f, 0.f};
    pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] - width,
                            position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                            primaryColor, snormal);
    pos += packObjectVertex(pos, position[0] + std::sin(angle) * orad, position[1] - width,
                            position[2] + std::cos(angle) * orad + cent_height, 0., 0.,
                            primaryColor, snormal);
  }
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
    GLfloat inormal[3] = {-std::sin(angle), -std::cos(angle), 0.f};
    pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] - width,
                            position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                            primaryColor, inormal);
    pos += packObjectVertex(pos, position[0] + std::sin(angle) * irad, position[1] + width,
                            position[2] + std::cos(angle) * irad + cent_height, 0., 0.,
                            primaryColor, inormal);
  }
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
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
    for (int i = 0; i < nfacets; i++) {
      int base = 2 * k * nfacets;
      int j = (i + 1) % nfacets;
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
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
    GLfloat inormal[3] = {std::cos(angle), std::sin(angle), 0.4f};
    pos += packObjectVertex(pos, position[0] + radius * std::cos(angle),
                            position[1] + radius * std::sin(angle), position[2] + 0.1f, 0., 0.,
                            white, inormal);
  }
  GLfloat vnormal[3] = {0.f, 0.f, 1.f};
  pos += packObjectVertex(pos, position[0], position[1], position[2] + 0.3, 0., 0., white,
                          vnormal);

  for (int i = 0; i < nfacets; i++) {
    idxs[8 * nfacets + i][0] = 9 * nfacets;
    idxs[8 * nfacets + i][1] = 8 * nfacets + i;
    idxs[8 * nfacets + i][2] = 8 * nfacets + (i + 1) % nfacets;
  }

  // Transfer
  setupObjectRenderState();

  GLint fogActive = (Game::current && Game::current->fogThickness != 0);
  glUniform1i(glGetUniformLocation(shaderObject, "fog_active"), fogActive);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0],
              specularColor[1], specularColor[2], specularColor[3]);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 15.f / 128.f);

  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  GLuint databuf, idxbuf;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);

  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 3 * 9 * nfacets, GL_UNSIGNED_SHORT, (void *)0);

  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);

  glUseProgram(0);

  glPopAttrib();
}

void Teleport::draw2() {
  if (!is_on) return;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  setupObjectRenderState();

  GLint fogActive = (Game::current && Game::current->fogThickness != 0);
  glUniform1i(glGetUniformLocation(shaderObject, "fog_active"), fogActive);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0., 0., 0., 1.);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.);
  glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), -1.);

  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  int nfacets = 12;

  GLfloat color[4] = {secondaryColor[0], secondaryColor[1], secondaryColor[2],
                      secondaryColor[3] * (0.4f + 0.2f * (GLfloat)frandom())};
  GLfloat flat[3] = {0., 0., 0.};
  GLfloat rad = (4.0 / 3.0) * radius;
  GLfloat cent_height = 0.5;

  GLfloat data[(nfacets + 1) * 8];
  ushort idxs[nfacets][3];
  char *pos = (char *)data;
  pos += packObjectVertex(pos, position[0], position[1], position[2] + cent_height, 0., 0.,
                          color, flat);
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;

    pos += packObjectVertex(pos, position[0] + std::sin(angle) * rad, position[1],
                            position[2] + std::cos(angle) * rad + cent_height, 0., 0., color,
                            flat);
    idxs[i][0] = 0;
    idxs[i][1] = i + 1;
    idxs[i][2] = (i + 1) % nfacets + 1;
  }

  GLuint databuf, idxbuf;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);

  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, (nfacets + 1) * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nfacets * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);

  configureObjectAttributes();

  glDrawElements(GL_TRIANGLES, nfacets * 3, GL_UNSIGNED_SHORT, (void *)0);

  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);

  glUseProgram(0);

  glPopAttrib();
}

void Teleport::tick(Real t) {
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
