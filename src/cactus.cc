/* cactus.cc
   Hurts when touched

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

#include "cactus.h"

#include "animatedCollection.h"
#include "ball.h"
#include "game.h"
#include "map.h"
#include "sound.h"

Cactus::Cactus(Real x, Real y, Real radius) : Animated(Role_OtherAnimated) {
  position[0] = x;
  position[1] = y;
  position[2] = Game::current->map->getHeight(position[0], position[1]);
  primaryColor[0] = .3;
  primaryColor[1] = 1.;
  primaryColor[2] = .4;
  secondaryColor[0] = 0.0;
  secondaryColor[1] = 0.0;
  secondaryColor[2] = 0.0;
  this->killed = false;
  this->killed_time = 1.;
  this->base_radius = radius;

  boundingBox[0][0] = -base_radius;
  boundingBox[1][0] = base_radius;
  boundingBox[0][1] = -base_radius;
  boundingBox[1][1] = base_radius;
  boundingBox[0][2] = -base_radius;
  boundingBox[1][2] = std::max(1., base_radius);

  scoreOnDeath = Game::defaultScores[SCORE_CACTUS][0];
  timeOnDeath = Game::defaultScores[SCORE_CACTUS][1];
}

int Cactus::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  allocateBuffers(1, idxbufs, databufs);

  GLfloat radius = killed_time * base_radius;

  const int nsides = 6;
  // Body: 4N+1 verts, 7N faces
  // Spikes: 16N verts, 12N faces
  GLfloat data[nsides * 20 + 1][8];
  ushort idxs[nsides * 19][3];

  GLfloat color[4] = {primaryColor[0], primaryColor[1], primaryColor[2], 1.f};
  GLfloat spkco[4] = {secondaryColor[0], secondaryColor[1], secondaryColor[2], 1.f};

  GLfloat radii[4] = {0.5f, 1.0f, 0.9f, 0.6f};
  GLfloat heights[5] = {0., 0.35, 0.6, 0.8, 1.0};
  GLfloat norm[4][2] = {{1.f, 0.f}, {1.f, 0.1f}, {1.f, 0.3f}, {1.f, 0.7f}};
  for (int i = 0; i < 4; i++) {
    norm[i][0] /= std::sqrt(norm[i][0] * norm[i][0] + norm[i][1] * norm[i][1]);
    norm[i][1] /= std::sqrt(norm[i][0] * norm[i][0] + norm[i][1] * norm[i][1]);
  }

  // Draw body
  char *pos = (char *)data;
  for (int h = 0; h < 4; h++) {
    for (int i = 0; i < nsides; i++) {
      GLfloat angle = 2 * i * M_PI / nsides;
      GLfloat loc[3] = {radii[h] * radius * std::cos(angle),
                        radii[h] * radius * std::sin(angle), heights[h]};
      GLfloat normal[3] = {norm[h][0] * std::cos(angle), norm[h][0] * std::sin(angle),
                           norm[h][1]};
      pos += packObjectVertex(pos, position[0] + loc[0], position[1] + loc[1],
                              position[2] + loc[2], 0., 0., color, normal);
    }
  }
  GLfloat vnormal[3] = {0.f, 0.f, 1.f};
  pos += packObjectVertex(pos, position[0], position[1], position[2] + heights[4], 0., 0.,
                          color, vnormal);

  for (int k = 0; k < 3; k++) {
    for (int i = 0; i < nsides; i++) {
      idxs[2 * k * nsides + 2 * i][0] = k * nsides + i;
      idxs[2 * k * nsides + 2 * i][1] = (k + 1) * nsides + (i + 1) % nsides;
      idxs[2 * k * nsides + 2 * i][2] = (k + 1) * nsides + i;
      idxs[2 * k * nsides + 2 * i + 1][0] = k * nsides + i;
      idxs[2 * k * nsides + 2 * i + 1][1] = k * nsides + (i + 1) % nsides;
      idxs[2 * k * nsides + 2 * i + 1][2] = (k + 1) * nsides + (i + 1) % nsides;
    }
  }
  for (int i = 0; i < nsides; i++) {
    idxs[6 * nsides + i][0] = 3 * nsides + i;
    idxs[6 * nsides + i][1] = 3 * nsides + (i + 1) % nsides;
    idxs[6 * nsides + i][2] = 4 * nsides;
  }

  // Draw spikes
  GLfloat spikewid = 0.05f;
  GLfloat flat[3] = {0.f, 0.f, 0.f};
  GLfloat spike_height[4] = {0.17f, 0.39f, 0.68f, 0.89f};
  GLfloat spike_rad[4] = {0.89f, 1.2f, 0.99f, 0.44f};
  for (int h = 0; h < 4; h++) {
    for (int i = 0; i < nsides; i++) {
      GLfloat angle = 2 * i * M_PI / nsides + M_PI / nsides;
      GLfloat end[3] = {spike_rad[h] * radius * std::cos(angle),
                        spike_rad[h] * radius * std::sin(angle), spike_height[h]};
      GLfloat dhs[3][2] = {{0, spikewid}, {-spikewid, -spikewid}, {spikewid, -spikewid}};
      for (int k = 0; k < 3; k++) {
        GLfloat central[3] = {-std::sin(angle) * dhs[k][0], std::cos(angle) * dhs[k][0],
                              dhs[k][1] + spike_height[h]};
        pos += packObjectVertex(pos, position[0] + central[0], position[1] + central[1],
                                position[2] + central[2], 0., 0., spkco, flat);
      }
      pos += packObjectVertex(pos, position[0] + end[0], position[1] + end[1],
                              position[2] + end[2], 0., 0., spkco, flat);
    }
  }

  int vbase = 4 * nsides + 1;
  int ibase = 7 * nsides;
  for (int i = 0; i < 4 * nsides; i++) {
    idxs[ibase][0] = vbase + 0;
    idxs[ibase][1] = vbase + 1;
    idxs[ibase][2] = vbase + 3;
    idxs[ibase + 1][0] = vbase + 1;
    idxs[ibase + 1][1] = vbase + 2;
    idxs[ibase + 1][2] = vbase + 3;
    idxs[ibase + 2][0] = vbase + 2;
    idxs[ibase + 2][1] = vbase + 0;
    idxs[ibase + 2][2] = vbase + 3;
    ibase += 3;
    vbase += 4;
  }

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  return 1;
}

void Cactus::drawBuffers1(GLuint *idxbufs, GLuint *databufs) {
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  int nsides = 6;
  if (activeView.calculating_shadows) {
    setActiveProgramAndUniforms(shaderObjectShadow);
  } else {
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0],
                specularColor[1], specularColor[2], specularColor[3]);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 15.f / 128.f);
  }
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 19 * nsides * 3, GL_UNSIGNED_SHORT, (void *)0);
}

void Cactus::drawBuffers2(GLuint * /*idxbufs*/, GLuint * /*databufs*/) {}

void Cactus::tick(Real t) {
  position[2] = Game::current->map->getHeight(position[0], position[1]);

  if (killed) {
    killed_time -= t;
    if (killed_time <= 0.) { remove(); }
    return;
  }

  // do I parse all the balls (incl. Mr Black) or just the player ?
  const std::vector<Animated *> &balls = Game::current->balls->bboxOverlapsWith(this);
  std::vector<Animated *>::const_iterator iter = balls.begin();
  std::vector<Animated *>::const_iterator end = balls.end();
  for (; iter != end; iter++) {
    Ball *ball = (Ball *)*iter;
    if (!ball->alive) continue;
    if (ball->no_physics) continue;

    Coord3d diff = position - ball->position;
    if (length(diff) < ball->radius + base_radius) {
      if (ball->modTimeLeft[MOD_SPIKE]) {
        Animated::die(DIE_OTHER);
        killed = true;
        playEffect(SFX_CACTUS_DIE);
        return;
      }

      ball->die(DIE_CRASH);
    }
  }
}
