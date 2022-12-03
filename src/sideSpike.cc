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

SideSpike::SideSpike(Game &g, const Coord3d &position, Real speed, Real phase, int side)
    : Animated(g, Role_OtherAnimated, 1) {
  this->position = position;
  this->position[2] = game.map->getHeight(position[0], position[1]) + 0.0;
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

  primaryColor = Color(SRGBColor(0.9, 0.5, 0.4, 1.0));
  secondaryColor = Color(SRGBColor(0.9, 0.8, 0.5, 1.0));
  specularColor = Color(SRGBColor(0.1, 0.1, 0.1, 1.0));
}

void SideSpike::updateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                              const GLuint *vaolist, bool firstCall) {
  if (firstCall || primaryColor != bufferPrimaryColor ||
      secondaryColor != bufferSecondaryColor) {
    const int nfacets = 6;
    GLfloat data[(4 * nfacets) * 8];
    ushort idxs[3 * nfacets][3];

    generateSpikeVBO(data, idxs, primaryColor, secondaryColor, 0.7);

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

void SideSpike::drawBuffers1(const GLuint *vaolist) const {
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  const int nfacets = 6;

  Matrix3d rotmtx1 = {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}};
  Matrix3d rotmtx2 = {{0, 0, -1}, {0, 1, 0}, {1, 0, 0}};
  Matrix3d rotmtx3 = {{1, 0, 0}, {0, 0, 1}, {0, -1, 0}};
  Matrix3d rotmtx4 = {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}};

  Matrix3d *rot;

  Coord3d pos(position[0], position[1], position[2] + 0.25);
  switch (side) {
  case 1:
    rot = &rotmtx1;
    pos[0] += -0.5 + offset;
    break;
  case 2:
    rot = &rotmtx2;
    pos[0] += 0.5 - offset;
    break;
  case 3:
    rot = &rotmtx3;
    pos[1] += -0.5 + offset;
    break;
  case 4:
    rot = &rotmtx4;
    pos[1] += 0.5 - offset;
    break;
  }

  Matrix4d transform;
  affineMatrix(transform, *rot, pos);

  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, transform, specularColor, 1., Lighting_Regular);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, (3 * 3 * nfacets), GL_UNSIGNED_SHORT, (void *)0);
}
void SideSpike::drawBuffers2(const GLuint * /*vaolist*/) const {}

void SideSpike::tick(Real t) {
  Animated::tick(t);

  if (is_on) phase += t * this->speed;

  double z;
  while (phase > 1.0) phase -= 1.0;
  double h = -0.3;
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

  if (offset <= 0.05) return;  // no collision possible

  double sx = 0., sy = 0.;
  double od = 0., aod;  // lateral distance to the spike
  int ol = 0;           // true if possible bounce

  Animated **balls;
  int nballs = game.balls->bboxOverlapsWith(this, &balls);
  for (int i = 0; i < nballs; i++) {
    Ball *ball = (Ball *)balls[i];

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
    double dist = std::sqrt((sx - ball->position[0]) * (sx - ball->position[0]) +
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
