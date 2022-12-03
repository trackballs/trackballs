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

Spike::Spike(Game &g, const Coord3d &pos, Real speed, Real phase)
    : Animated(g, Role_OtherAnimated, 1) {
  this->position = pos;
  this->position[2] = game.map->getHeight(pos[0], pos[1]) + 0.0;
  this->speed = speed;
  while (phase < 0.0) phase += 1.0;
  this->phase = phase;
  this->soundDone = 0;

  primaryColor = Color(SRGBColor(0.9, 0.5, 0.4, 1.0));
  secondaryColor = Color(SRGBColor(0.9, 0.8, 0.5, 1.0));
  specularColor = Color(SRGBColor(0.1, 0.1, 0.1, 1.0));
}

void Spike::updateBuffers(const GLuint *idxbufs, const GLuint *databufs, const GLuint *vaolist,
                          bool firstCall) {
  if (firstCall || primaryColor != bufferPrimaryColor ||
      secondaryColor != bufferSecondaryColor) {
    const int nfacets = 6;
    GLfloat data[(4 * nfacets) * 8];
    ushort idxs[3 * nfacets][3];

    generateSpikeVBO(data, idxs, primaryColor, secondaryColor, 2.0);

    glBindVertexArray(vaolist[0]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
    configureObjectAttributes();

    bufferPrimaryColor = primaryColor;
    bufferSecondaryColor = secondaryColor;
  }
}

void Spike::drawBuffers1(const GLuint *vaolist) const {
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  const int nfacets = 6;

  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);

  Matrix4d offset;
  affineMatrix(offset, identity3, position);
  setObjectUniforms(uloc, offset, specularColor, 1., Lighting_Regular);

  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, (3 * 3 * nfacets), GL_UNSIGNED_SHORT, (void *)0);
}
void Spike::drawBuffers2(const GLuint * /*vaolist*/) const {}

void Spike::tick(Real t) {
  Animated::tick(t);

  int is_sinking = 0;

  if (is_on) phase += t * this->speed;
  while (phase > 1.0) phase -= 1.0;
  double h = game.map->getHeight(position[0], position[1]) - 0.3;
  double z;
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
  double x = position[0], y = position[1];
  int nballs = game.balls->bboxOverlapsWith(this, &balls);
  for (int i = 0; i < nballs; i++) {
    Ball *ball = (Ball *)balls[i];

    if (ball->position[0] > x - 1 && ball->position[0] < x + 1 && ball->position[1] > y - 1 &&
        ball->position[1] < y + 1) {
      double dx = ball->position[0] - x;
      double dy = ball->position[1] - y;
      double dist = std::sqrt(dx * dx + dy * dy);

      if (dist < 0.1 + ball->radius) {
        h = ball->position[2] + dist;
        if (h < z + 0.05 + 0.35 * is_sinking) {
          /* Bounce against it */
          dx /= dist;
          dy /= dist;
          double bounce_speed = dx * ball->velocity[0] + dy * ball->velocity[1];
          if (bounce_speed < 0) {
            ball->velocity[0] -= 2 * bounce_speed * dx;
            ball->velocity[1] -= 2 * bounce_speed * dy;
          }
        } else if (h < 0.4 + z)
          /* Die */
          ball->die(DIE_CRASH);
      }
    }
  }

  // play a 'rising' sound if the ball is in the round
  Player *player = game.player1;
  double dist =
      std::sqrt((position[0] - player->position[0]) * (position[0] - player->position[0]) +
                (position[1] - player->position[1]) * (position[1] - player->position[1]));
  if ((dist < 9.) && (!soundDone) && (phase >= 0.4) && (phase < 0.5)) {
    if (dist < 6.)
      playEffect(SFX_SPIKE, 0.66);
    else
      playEffect(SFX_SPIKE, 0.66 * (9. - dist) / 3.);
    soundDone = 1;
  }
}

void generateSpikeVBO(GLfloat *data, ushort idxs[][3], Color sidec, Color tipc,
                      GLfloat length) {
  const int nfacets = 6;
  char *pos = (char *)data;

  double d1 = 1 / std::sqrt(10.0), d2 = 3 / std::sqrt(10.0);
  for (int i = 0; i < 4 * nfacets; i++) {
    Coord3d local;
    Coord3d normal;
    bool is_tip;

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
    GLfloat flocal[3] = {(GLfloat)local[0], (GLfloat)local[1], (GLfloat)local[2]};
    GLfloat fnormal[3] = {(GLfloat)normal[0], (GLfloat)normal[1], (GLfloat)normal[2]};
    pos += packObjectVertex(pos, flocal[0], flocal[1], flocal[2], 0., 0.,
                            is_tip ? tipc : sidec, fnormal);
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
