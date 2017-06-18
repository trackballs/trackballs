/** \file spikes.cc
   Dangerous spikes shooting from the ground
*/
/*
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

#include "spike.h"

#include "game.h"
#include "map.h"
#include "player.h"
#include "sound.h"

Spike::Spike(Coord3d position, Real speed, Real phase) {
  assign(position, this->position);
  this->position[2] = Game::current->map->getHeight(position[0], position[1]) + 0.0;
  this->speed = speed;
  while (phase < 0.0) phase += 1.0;
  this->phase = phase;
  this->soundDone = 0;

  primaryColor[0] = 0.9;
  primaryColor[1] = 0.5;
  primaryColor[2] = 0.4;

  secondaryColor[0] = 0.9;
  secondaryColor[1] = 0.8;
  secondaryColor[2] = 0.5;

  specularColor[0] = 0.1;
  specularColor[1] = 0.1;
  specularColor[2] = 0.1;
}

int Spike::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  allocateBuffers(1, idxbufs, databufs);

  int nfacets = 6;
  GLfloat data[(4 * nfacets) * 8];
  ushort idxs[3 * nfacets][3];

  Matrix3d rotmtx;
  Matrix4d frommtx;
  identityMatrix(frommtx);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) rotmtx[i][j] = frommtx[i][j];
  generateSpikeVBO(data, idxs, nfacets, rotmtx, position, primaryColor, secondaryColor, 2.0);

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  return 1;
}

void Spike::drawBuffers1(GLuint *idxbufs, GLuint *databufs) {
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  const int nfacets = 6;

  setActiveProgramAndUniforms(shaderObject);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0],
              specularColor[1], specularColor[2], specularColor[3]);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 128.f / 128.f);
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, (3 * 3 * nfacets), GL_UNSIGNED_SHORT, (void *)0);
}
void Spike::drawBuffers2(GLuint * /*idxbufs*/, GLuint * /*databufs*/) {}

void Spike::tick(Real t) {
  std::set<Ball *>::iterator iter = Ball::balls->begin();
  std::set<Ball *>::iterator end = Ball::balls->end();
  Ball *ball;
  double dist, dx, dy, speed, h;
  int is_sinking = 0;

  double x = position[0], y = position[1], z = position[2];
  if (is_on) phase += t * this->speed;
  while (phase > 1.0) phase -= 1.0;
  h = Game::current->map->getHeight(x, y) - 0.3;
  if (phase < 0.4) {
    /* spike is low */
    z = h;  //
  } else if (phase < 0.5) {
    /* spike rising */
    z = h + (phase - 0.4) / 0.1;
  } else if (phase < 0.9) {
    /* spike is high */
    z = h + 1.0;
    soundDone = 0;
  } else {
    /* spike is sinking */
    z = h + 1.0 - (phase - 0.9) / 0.1;
    is_sinking = 1;
  }
  position[2] = z;

  for (; iter != end; iter++) {
    ball = *iter;
    if (ball->alive && ball->position[0] > x - 1 && ball->position[0] < x + 1 &&
        ball->position[1] > y - 1 && ball->position[1] < y + 1) {
      dx = ball->position[0] - x;
      dy = ball->position[1] - y;
      dist = sqrt(dx * dx + dy * dy);

      if (dist < 0.1 + ball->radius) {
        h = ball->position[2] + dist;
        if (h < z + 0.05 + 0.35 * is_sinking) {
          /* Bounce against it */
          dx /= dist;
          dy /= dist;
          speed = dx * ball->velocity[0] + dy * ball->velocity[1];
          if (speed < 0) {
            ball->velocity[0] -= 2 * speed * dx;
            ball->velocity[1] -= 2 * speed * dy;
          }
        } else if (h < 0.4 + z)
          /* Die */
          ball->die(DIE_CRASH);
      }
    }
  }

  // play a 'rising' sound if the ball is in the round
  Player *player = Game::current->player1;
  dist = sqrt((position[0] - player->position[0]) * (position[0] - player->position[0]) +
              (position[1] - player->position[1]) * (position[1] - player->position[1]));
  if ((dist < 9.) && (!soundDone) && (phase >= 0.4) && (phase < 0.5)) {
    if (dist < 6.)
      playEffect(SFX_SPIKE, 0.66);
    else
      playEffect(SFX_SPIKE, 0.66 * (9. - dist) / 3.);
    soundDone = 1;
  }
}

void generateSpikeVBO(GLfloat *data, ushort idxs[][3], int nfacets, Matrix3d rotmtx,
                      Coord3d position, GLfloat sidec[4], GLfloat tipc[4], GLfloat length) {
  char *pos = (char *)data;

  double d1 = 1 / sqrt(10.0), d2 = 3 / sqrt(10.0);
  for (int i = 0; i < 4 * nfacets; i++) {
    Coord3d local;
    Coord3d normal;
    GLfloat *color = NULL;

    int step = i / nfacets;
    double angle = (i % nfacets) * 2. * M_PI / nfacets;
    local[0] = 0.1 * std::sin(angle);
    local[1] = 0.1 * std::cos(angle);
    if (step == 0) {
      color = tipc;
      local[2] = 0.;
      normal[0] = d2 * std::sin(angle);
      normal[1] = d2 * std::cos(angle);
      normal[2] = d1;
    } else if (step == 1) {
      color = sidec;
      local[2] = 0;
      normal[0] = std::sin(angle);
      normal[1] = std::cos(angle);
      normal[2] = 0.;
    } else if (step == 2) {
      color = sidec;
      local[2] = -length;
      normal[0] = std::sin(angle);
      normal[1] = std::cos(angle);
      normal[2] = 0.;
    } else {
      color = tipc;
      local[0] = 0.;
      local[1] = 0.;
      local[2] = 0.3;
      normal[0] = d2 * std::sin(angle + M_PI / nfacets);
      normal[1] = d2 * std::cos(angle + M_PI / nfacets);
      normal[2] = d1;
    }
    Coord3d tlocal, tnormal;
    useMatrix(rotmtx, local, tlocal);
    useMatrix(rotmtx, normal, tnormal);
    GLfloat flocal[3] = {(GLfloat)tlocal[0], (GLfloat)tlocal[1], (GLfloat)tlocal[2]};
    GLfloat fnormal[3] = {(GLfloat)tnormal[0], (GLfloat)tnormal[1], (GLfloat)tnormal[2]};
    pos += packObjectVertex(pos, position[0] + flocal[0], position[1] + flocal[1],
                            position[2] + flocal[2], 0., 0., color, fnormal);
  }

  for (int i = 0; i < nfacets; i++) {
    idxs[i][0] = 3 * nfacets + i;
    idxs[i][1] = (i + 1) % nfacets;
    idxs[i][2] = i;
  }
  for (int i = 0; i < nfacets; i++) {
    idxs[nfacets + i][0] = 2 * nfacets + i;
    idxs[nfacets + i][1] = nfacets + i;
    idxs[nfacets + i][2] = nfacets + (i + 1) % nfacets;
  }
  for (int i = 0; i < nfacets; i++) {
    idxs[2 * nfacets + i][0] = 2 * nfacets + (i + 1) % nfacets;
    idxs[2 * nfacets + i][1] = 2 * nfacets + i;
    idxs[2 * nfacets + i][2] = nfacets + (i + 1) % nfacets;
  }
}
