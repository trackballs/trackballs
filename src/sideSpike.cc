/* sideSpikes.cc
   Dangerous spikes shooting from the sides

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

#include "sideSpike.h"

#include "animatedCollection.h"
#include "ball.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "spike.h"

SideSpike::SideSpike(Coord3d position, Real speed, Real phase, int side) {
  this->position = position;
  this->position[2] = Game::current->map->getHeight(position[0], position[1]) + 0.0;
  this->speed = speed;
  while (phase < 0.0) phase += 1.0;
  this->phase = phase;
  if ((side < 1) || (side > 4)) {
    warning("SideSpike (%f,%f) got invalide side (%d). Treated as '1'.", position[0],
            position[1], side);
    this->side = 1;
  } else
    this->side = side;
  this->soundDone = 0;
  this->offset = 0.;

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

int SideSpike::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  allocateBuffers(1, idxbufs, databufs);

  int nfacets = 6;
  GLfloat data[(4 * nfacets) * 8];
  ushort idxs[3 * nfacets][3];

  Matrix4d frommtx;
  identityMatrix(frommtx);

  Coord3d pos(position[0], position[1], position[2] + 0.25);
  switch (side) {
  case 1:
    rotateY(M_PI / 2, frommtx);
    pos[0] += -0.5 + offset;
    break;
  case 2:
    rotateY(-M_PI / 2, frommtx);
    pos[0] += 0.5 - offset;
    break;
  case 3:
    rotateX(M_PI / 2, frommtx);
    pos[1] += -0.5 + offset;
    break;
  case 4:
    rotateX(-M_PI / 2, frommtx);
    pos[1] += 0.5 - offset;
    break;
  }

  Matrix3d rotmtx;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) rotmtx[i][j] = frommtx[i][j];
  generateSpikeVBO(data, idxs, nfacets, rotmtx, pos, primaryColor, secondaryColor, 0.7);

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  return 1;
}

void SideSpike::drawBuffers1(GLuint *idxbufs, GLuint *databufs) {
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
void SideSpike::drawBuffers2(GLuint * /*idxbufs*/, GLuint * /*databufs*/) {}

void SideSpike::tick(Real t) {
  double dist, h;

  if (is_on) phase += t * this->speed;

  double z = position[2];
  while (phase > 1.0) phase -= 1.0;
  h = -0.3;
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
  }
  offset = z;

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

  if (offset <= 0.05) return;  // no collision possible

  double sx = 0., sy = 0.;
  double od = 0., aod;  // lateral distance to the spike
  int ol = 0;           // true if possible bounce

  const std::set<Animated *> &balls = Game::current->balls->bboxOverlapsWith(this);
  std::set<Animated *>::iterator iter = balls.begin();
  std::set<Animated *>::iterator end = balls.end();
  for (; iter != end; iter++) {
    Ball *ball = (Ball *)*iter;

    if (!ball->alive) continue;

    switch (side) {
    case 1:
      sx = position[0] - 0.5 + (offset + 0.29);
      sy = position[1];

      od = ball->position[1] - position[1];

      if ((ball->position[0] - (position[0] - 0.5) > offset + 0.29) ||
          (ball->position[0] - (position[0] - 0.5) < 0.))
        ol = 0;
      else
        ol = 1;
      break;
    case 2:
      sx = position[0] + 0.5 - (offset + 0.29);
      sy = position[1];

      od = ball->position[1] - position[1];

      if ((position[0] + 0.5 - ball->position[0] > offset + 0.29) ||
          (position[0] + 0.5 - ball->position[0] < 0.))
        ol = 0;
      else
        ol = 1;
      break;
    case 3:
      sx = position[0];
      sy = position[1] - 0.5 + (offset + 0.29);

      od = ball->position[0] - position[0];

      if ((ball->position[1] - (position[1] - 0.5) > offset + 0.29) ||
          (ball->position[1] - (position[1] - 0.5) < 0.))
        ol = 0;
      else
        ol = 1;
      break;
    case 4:
      sx = position[0];
      sy = position[1] + 0.5 - (offset + 0.29);

      od = ball->position[0] - position[0];

      if ((position[1] + 0.5 - ball->position[1] > offset + 0.29) ||
          (position[1] + 0.5 - ball->position[1] < 0.))
        ol = 0;
      else
        ol = 1;
      break;
    }
    dist = sqrt((sx - ball->position[0]) * (sx - ball->position[0]) +
                (sy - ball->position[1]) * (sy - ball->position[1]));

    // too far
    if (dist > ball->radius + 1. + 0.3) continue;

    // killed
    if (dist < ball->radius) { ball->die(DIE_CRASH); }

    if (od < 0.)
      aod = -od;
    else
      aod = od;
    // bounce ?
    if (ol && (aod < ball->radius + 0.1)) {
      switch (side) {
      case 1:
      case 2:
        ball->velocity[1] = -ball->velocity[1] * 1.15;
        if (od < 0.) {
          ball->position[1] -= 0.015;
        } else {
          ball->position[1] += 0.015;
        }
        break;
      case 3:
      case 4:
        ball->velocity[0] = -ball->velocity[0] * 1.15;
        if (od < 0.) {
          ball->position[0] -= 0.015;
        } else {
          ball->position[0] += 0.015;
        }
        break;
      }
    }
  }
}
