/* pipeConnector.cc
   Connects pipes

   Copyright (C) 2000  Mathias Broxvall

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
#include "animated.h"
#include "ball.h"
#include "game.h"
#include "pipeConnector.h"

using namespace std;

class set<PipeConnector *> *PipeConnector::connectors;
void PipeConnector::init() { connectors = new set<PipeConnector *>(); }
void PipeConnector::reset() {
  delete connectors;
  connectors = new set<PipeConnector *>();
}
PipeConnector::PipeConnector(Coord3d pos, Real r) : Animated() {
  assign(pos, position);
  radius = r;
  primaryColor[0] = primaryColor[1] = primaryColor[2] = 0.6;
  connectors->insert(this);
}
void PipeConnector::draw() {
  if (primaryColor[3] >= 1.0) drawMe();
}
void PipeConnector::draw2() {
  if (primaryColor[3] < 1.0) drawMe();
}
void PipeConnector::drawMe() {
  if (primaryColor[3] < 1.0f) {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
  } else {
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
  }

  // Create sphere
  int ntries = 0;
  int nverts = 0;
  int detail = 4;
  countObjectSpherePoints(&ntries, &nverts, detail);
  GLfloat *data = new GLfloat[nverts * 8];
  ushort *idxs = new ushort[ntries * 3];
  GLfloat pos[3] = {(GLfloat)position[0], (GLfloat)position[1], (GLfloat)position[2]};
  Matrix3d identity = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};

  placeObjectSphere(data, idxs, 0, pos, identity, radius, detail, primaryColor);

  // Transfer
  setupObjectRenderState();

  GLint fogActive = (Game::current && Game::current->fogThickness != 0);
  glUniform1i(glGetUniformLocation(shaderObject, "fog_active"), fogActive);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0] * 0.1,
              specularColor[1] * 0.1, specularColor[2] * 0.1, 1.);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 128.f / 128.f);

  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  GLuint databuf, idxbuf;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);

  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, nverts * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ntries * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  configureObjectAttributes();

  glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);

  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);
}
void PipeConnector::tick(Real t) {
  boundingBox[0][0] = -radius;
  boundingBox[0][1] = -radius;
  boundingBox[0][2] = -radius;
  boundingBox[1][0] = radius;
  boundingBox[1][1] = radius;
  boundingBox[1][2] = radius;
}
void PipeConnector::onRemove() {
  Animated::onRemove();
  connectors->erase(this);
}
