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

#include "animatedCollection.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "sound.h"

Spike::Spike(const Coord3d &position, Real speed, Real phase)
    : Animated(Role_OtherAnimated, 1) {
  this->position = position;
  this->position[2] = Game::current->map->getHeight(position[0], position[1]) + 0.0;
  this->speed = speed;
  while (phase < 0.0) phase += 1.0;
  this->phase = phase;
  this->soundDone = 0;

  primaryColor = Color(0.9, 0.5, 0.4, 1.0);
  secondaryColor = Color(0.9, 0.8, 0.5, 1.0);
  specularColor = Color(0.1, 0.1, 0.1, 1.0);
}

void Spike::generateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                            const GLuint *vaolist, bool /*mustUpdate*/) const {
  const int nfacets = 6;
  GLfloat data[(4 * nfacets) * 8];
  ushort idxs[3 * nfacets][3];

  Matrix3d rotmtx = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  generateSpikeVBO(data, idxs, rotmtx, position, primaryColor, secondaryColor, 2.0);

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  configureObjectAttributes();
}

void Spike::drawBuffers1(const GLuint *vaolist) const {
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  const int nfacets = 6;

  if (activeView.calculating_shadows) {
    setActiveProgramAndUniforms(shaderObjectShadow);
  } else {
    setActiveProgramAndUniforms(shaderObject);
    glUniformC(glGetUniformLocation(shaderObject, "specular"), specularColor);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 128.f / 128.f);
  }
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, (3 * 3 * nfacets), GL_UNSIGNED_SHORT, (void *)0);
}
void Spike::drawBuffers2(const GLuint * /*vaolist*/) const {}

void Spike::tick(Real t) {
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

  Animated **balls;
  int nballs = Game::current->balls->bboxOverlapsWith(this, &balls);
  for (int i = 0; i < nballs; i++) {
    Ball *ball = (Ball *)balls[i];

    if (ball->position[0] > x - 1 && ball->position[0] < x + 1 && ball->position[1] > y - 1 &&
        ball->position[1] < y + 1) {
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

void generateSpikeVBO(GLfloat *data, ushort idxs[][3], Matrix3d rotmtx,
                      const Coord3d &position, Color sidec, Color tipc, GLfloat length) {
  const int nfacets = 6;
  char *pos = (char *)data;

  double d1 = 1 / sqrt(10.0), d2 = 3 / sqrt(10.0);
  for (int i = 0; i < 4 * nfacets; i++) {
    Coord3d local;
    Coord3d normal;
    bool is_tip;
    GLfloat *color = NULL;

    int step = i / nfacets;
    int j = i % nfacets;
    local[0] = 0.1 * sin6[j];
    local[1] = 0.1 * cos6[j];
    if (step == 0) {
      is_tip = true;
      local[2] = 0.;
      normal[0] = d2 * sin6[j];
      normal[1] = d2 * cos6[j];
      normal[2] = d1;
    } else if (step == 1) {
      is_tip = false;
      local[2] = 0;
      normal[0] = sin6[j];
      normal[1] = cos6[j];
      normal[2] = 0.;
    } else if (step == 2) {
      is_tip = false;
      local[2] = -length;
      normal[0] = sin6[j];
      normal[1] = cos6[j];
      normal[2] = 0.;
    } else {
      is_tip = true;
      local[0] = 0.;
      local[1] = 0.;
      local[2] = 0.3;
      normal[0] = d2 * sin12[2 * j + 1];
      normal[1] = d2 * cos12[2 * j + 1];
      normal[2] = d1;
    }
    Coord3d tlocal = useMatrix(rotmtx, local);
    Coord3d tnormal = useMatrix(rotmtx, normal);
    GLfloat flocal[3] = {(GLfloat)tlocal[0], (GLfloat)tlocal[1], (GLfloat)tlocal[2]};
    GLfloat fnormal[3] = {(GLfloat)tnormal[0], (GLfloat)tnormal[1], (GLfloat)tnormal[2]};
    pos += packObjectVertex(pos, position[0] + flocal[0], position[1] + flocal[1],
                            position[2] + flocal[2], 0., 0., is_tip ? tipc : sidec, fnormal);
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
