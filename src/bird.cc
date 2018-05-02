/** \file bird.cc
   Travels along their path and kill player if touched.
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

#include "bird.h"

#include "animatedCollection.h"
#include "ball.h"
#include "game.h"
#include "map.h"
#include "sound.h"

Bird::Bird(Real x, Real y, Real dx, Real dy, Real size, Real speed)
    : Animated(Role_OtherAnimated) {
  this->x = x;
  this->y = y;
  this->dx = dx - x;
  this->dy = dy - y;
  this->size = size;
  this->speed = speed;
  this->lng = sqrt((double)(this->dx * this->dx + this->dy * this->dy));
  position[0] = x;
  position[1] = y;
  position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;
  primaryColor[0] = 1.;
  primaryColor[1] = 1.;
  primaryColor[2] = 1.;
  secondaryColor[0] = .6;
  secondaryColor[1] = 0.8;
  secondaryColor[2] = 0.9;
  animation = 0.0;
  rotation = M_PI - atan2(this->dx / lng, this->dy / lng);
  scoreOnDeath = Game::defaultScores[SCORE_BIRD][0];
  timeOnDeath = Game::defaultScores[SCORE_BIRD][1];
  boundingBox[0][0] = -size;
  boundingBox[1][0] = size;
  boundingBox[0][1] = -size;
  boundingBox[1][1] = size;
  boundingBox[0][2] = -size;
  boundingBox[1][2] = size;
}

int Bird::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  if (hide > 0.) return 0;

  allocateBuffers(1, idxbufs, databufs);

  GLfloat color[4];
  for (int i = 0; i < 3; i++) color[i] = primaryColor[i];
  color[3] = 1.0;

  GLfloat data[4 * 8];
  char *pos = (char *)data;
  GLfloat flat[3] = {0., 0., 0.};
  double angle =
      (0.8 - (animation < 0.5 ? animation * 2.0 : 2.0 - animation * 2.0)) * M_PI / 2.0;

  GLfloat dz = 0.5f * size * std::sin(angle);
  GLfloat loc[4][2] = {{0.f, (GLfloat)size},
                       {0.f, 0.f},
                       {-0.5f * (GLfloat)size * std::cos((GLfloat)angle), (GLfloat)size},
                       {0.5f * (GLfloat)size * std::cos((GLfloat)angle), (GLfloat)size}};
  for (int i = 0; i < 4; i++) {
    GLfloat ox = loc[i][0], oy = loc[i][1];
    loc[i][0] = -std::cos(rotation) * ox - std::sin(rotation) * oy;
    loc[i][1] = -std::sin(rotation) * ox + std::cos(rotation) * oy;
  }

  pos += packObjectVertex(pos, position[0] + loc[0][0], position[1] + loc[0][1], position[2],
                          1., 1., color, flat);
  pos += packObjectVertex(pos, position[0] + loc[1][0], position[1] + loc[1][1], position[2],
                          1., 0., color, flat);
  pos += packObjectVertex(pos, position[0] + loc[2][0], position[1] + loc[2][1],
                          position[2] + dz, 0., 1., color, flat);
  pos += packObjectVertex(pos, position[0] + loc[3][0], position[1] + loc[3][1],
                          position[2] + dz, 0., 1., color, flat);

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 4 * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);

  ushort idxs[2][3] = {{0, 1, 2}, {0, 3, 1}};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(ushort), idxs, GL_STATIC_DRAW);

  return 1;
}

void Bird::drawBuffers1(GLuint * /*idxbufs*/, GLuint * /*databufs*/) {}

void Bird::drawBuffers2(GLuint *idxbufs, GLuint *databufs) {
  if (hide > 0.) return;

  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  if (activeView.calculating_shadows) {
    setActiveProgramAndUniforms(shaderObjectShadow);
  } else {
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0],
                specularColor[1], specularColor[2], 1.);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 10.f / 128.f);
  }
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("wings.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0);
}

void Bird::tick(Real t) {
  Animated::tick(t);

  animation = fmod(animation + t / 0.8, 1.0);

  // if hiden, just count down the time
  if (hide > 0.) {
    hide -= t;
    return;
  }

  // update the position on the line
  position[0] += speed * t * dx / lng;
  position[1] += speed * t * dy / lng;
  if (!(flags & BIRD_CONSTANT_HEIGHT))
    position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;

  // check for collisions with balls
  Ball *ball;
  Coord3d diff;
  const std::vector<Animated *> &balls = Game::current->balls->bboxOverlapsWith(this);
  std::vector<Animated *>::const_iterator iter = balls.begin();
  std::vector<Animated *>::const_iterator end = balls.end();

  for (; iter != end; iter++) {
    ball = (Ball *)*iter;
    if (!ball->alive) continue;
    if (ball->no_physics) continue;

    diff = position - ball->position;
    // TODO: improve the collision detection
    if (length(diff) < ball->radius + size * .75) {
      if (ball->modTimeLeft[MOD_SPIKE]) {
        // the ball kills the bird !!!
        Animated::die(DIE_OTHER);

        playEffect(SFX_BIRD_DIE);
        // restart the bird
        position[0] = x;
        position[1] = y;
        position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;
        hide = 1.;
        triggerHook(GameHookEvent_Death, NULL);
        triggerHook(GameHookEvent_Spawn, NULL);
        return;
      }
      // kill the ball
      ball->die(DIE_CRASH);
    }
  }

  // check if the bird is outside its path
  Real cdx, cdy;

  cdx = position[0] - x;
  cdy = position[1] - y;
  if (((dx < 0.) && (cdx <= dx)) || ((dx > 0.) && (cdx >= dx)) || ((dy < 0.) && (cdy <= dy)) ||
      ((dy > 0.) && (cdy >= dy))) {
    // restart
    position[0] = x;
    position[1] = y;
    position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;
    hide = .5;  // wait .5 sec before restarting
    triggerHook(GameHookEvent_Spawn, NULL);
  }
}
