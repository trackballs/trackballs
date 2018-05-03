/* fountain.cc
   Decoration that continously creates droplets of specific color.

   Copyright (C) 2003  Mathias Broxvall

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

#include "fountain.h"
#include "game.h"
#include "player.h"
#include "settings.h"

Fountain::Fountain(const Coord3d &pos, double randomSpeed, double radius, double strength)
    : Animated(Role_OtherAnimated),
      randomSpeed(randomSpeed),
      radius(radius),
      strength(strength) {
  primaryColor[0] = 0.4;
  primaryColor[1] = 0.4;
  primaryColor[2] = 0.8;
  primaryColor[3] = 0.5;
  velocity[0] = velocity[1] = 0.0;
  velocity[2] = 0.3;
  position = pos;
  createPoint = 0.0;
  nextPoint = 0;
  drawFrom = 0;
  memset(positions, 0, sizeof(positions));
  memset(velocities, 0, sizeof(velocities));
  memset(creationTime, 0, sizeof(creationTime));
}

int Fountain::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) const {
  if (Settings::settings->gfx_details <= GFX_DETAILS_MINIMALISTIC) return 0;
  allocateBuffers(1, idxbufs, databufs);

  int skip = Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE ? 2 : 1;
  if (fps < 5) skip *= 2;
  int npoints = 0;
  if (nextPoint < drawFrom) {
    for (int i = drawFrom; i < 800; i += skip) npoints++;
    for (int i = 0; i < nextPoint; i += skip) npoints++;
  } else
    for (int i = drawFrom; i < nextPoint; i += skip) npoints++;

  GLfloat *data = new GLfloat[3 * npoints];
  ushort *idxs = new ushort[npoints];
  int j = 0;
  if (nextPoint < drawFrom) {
    for (int i = drawFrom; i < 800; i += skip) {
      data[3 * j + 0] = positions[i][0];
      data[3 * j + 1] = positions[i][1];
      data[3 * j + 2] = positions[i][2];
      j++;
    }
    for (int i = 0; i < nextPoint; i += skip) {
      data[3 * j + 0] = positions[i][0];
      data[3 * j + 1] = positions[i][1];
      data[3 * j + 2] = positions[i][2];
      j++;
    }
  } else {
    for (int i = drawFrom; i < nextPoint; i += skip) {
      data[3 * j + 0] = positions[i][0];
      data[3 * j + 1] = positions[i][1];
      data[3 * j + 2] = positions[i][2];
      j++;
    }
  }

  for (int i = 0; i < npoints; i++) { idxs[i] = i; }

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * npoints * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, npoints * sizeof(ushort), idxs, GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  return 1;
}

void Fountain::drawBuffers1(GLuint * /*idxbufs*/, GLuint * /*databufs*/) const {}

void Fountain::drawBuffers2(GLuint *idxbufs, GLuint *databufs) const {
  if (Settings::settings->gfx_details <= GFX_DETAILS_MINIMALISTIC) return;
  if (activeView.calculating_shadows) return;

  int skip = Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE ? 2 : 1;
  if (fps < 5) skip *= 2;
  int npoints = 0;
  if (nextPoint < drawFrom) {
    for (int i = drawFrom; i < 800; i += skip) npoints++;
    for (int i = 0; i < nextPoint; i += skip) npoints++;
  } else
    for (int i = drawFrom; i < nextPoint; i += skip) npoints++;

  glEnable(GL_BLEND);
  glPointSize(1.5 * screenWidth / 600.);
  setActiveProgramAndUniforms(shaderLine);
  glUniform4f(glGetUniformLocation(shaderLine, "line_color"), primaryColor[0], primaryColor[1],
              primaryColor[2], primaryColor[3]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glDrawElements(GL_POINTS, npoints, GL_UNSIGNED_SHORT, (void *)0);
}

void Fountain::tick(Real t) {
  int i, j;
  int skip = Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE ? 2 : 1;

  if (nextPoint < drawFrom) {
    for (i = drawFrom; i < 800; i += skip) {
      for (j = 0; j < 3; j++) positions[i][j] += velocities[i][j];
      velocities[i][2] -= t * 1.0;
    }
    for (i = 0; i < nextPoint; i += skip) {
      for (j = 0; j < 3; j++) positions[i][j] += velocities[i][j];
      velocities[i][2] -= t * 1.0;
    }
  } else
    for (i = drawFrom; i < nextPoint; i += skip) {
      for (j = 0; j < 3; j++) positions[i][j] += velocities[i][j];
      velocities[i][2] -= t * 1.0;
    }

  Real timeNow = Game::current->gameTime;
  createPoint += t * strength;
  while (createPoint > 0.0) {
    createPoint -= 1.0;
    //	if(drawFrom == nextPoint) drawFrom=(drawFrom+1)%800;
    positions[nextPoint][0] = position[0] + (frandom() - .5) * 2. * radius;
    positions[nextPoint][1] = position[1] + (frandom() - .5) * 2. * radius;
    positions[nextPoint][2] = position[2] + (frandom() - .5) * 2. * radius;
    velocities[nextPoint][0] = velocity[0] + (frandom() - .5) * 2. * randomSpeed;
    velocities[nextPoint][1] = velocity[1] + (frandom() - .5) * 2. * randomSpeed;
    velocities[nextPoint][2] = velocity[2] + (frandom() - .5) * 2. * randomSpeed;
    creationTime[nextPoint] = timeNow;
    nextPoint = (nextPoint + 1) % 800;
  }

  // YP: 'push' player if above
  if (strength > 0.) {
    // position[2] = Game::current->map->getHeight(position[0],position[1]);   //MB. No thanks,
    // let's keep the old position it messes with some maps otherwise
    Player *p = Game::current->player1;
    Coord3d diff;

    diff = position - p->position;
    if (length(diff) < p->radius + 0.2) {
      for (i = 0; i < 3; i++)
        p->velocity[i] +=
            velocity[i] / 0.3 * strength * 0.5 * (p->radius + 0.2 - length(diff)) * t;
    }
  }

  while (creationTime[drawFrom] < Game::current->gameTime - 1.5) {
    if (drawFrom == nextPoint) break;
    drawFrom = (drawFrom + 1) % 800;
  }
}
