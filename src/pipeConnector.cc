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

#include "pipeConnector.h"

#include "game.h"

PipeConnector::PipeConnector(const Coord3d &pos, Real r)
    : Animated(Role_PipeConnector, 1), radius(r) {
  position = pos;
  primaryColor = Color(0.6, 0.6, 0.6, 1.0);

  boundingBox[0][0] = -radius;
  boundingBox[0][1] = -radius;
  boundingBox[0][2] = -radius;
  boundingBox[1][0] = radius;
  boundingBox[1][1] = radius;
  boundingBox[1][2] = radius;
}

void PipeConnector::generateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                                    const GLuint *vaolist, bool mustUpdate) const {
  if (!mustUpdate) return;

  int ntries = 0;
  int nverts = 0;
  int detail = 6;
  countObjectSpherePoints(&ntries, &nverts, detail);
  GLfloat *data = new GLfloat[nverts * 8];
  ushort *idxs = new ushort[ntries * 3];
  GLfloat pos[3] = {(GLfloat)position[0], (GLfloat)position[1], (GLfloat)position[2]};
  Matrix3d identity = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};

  placeObjectSphere(data, idxs, 0, pos, identity, radius, detail, primaryColor);

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, nverts * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, ntries * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
  configureObjectAttributes();
  delete[] data;
  delete[] idxs;
}
void PipeConnector::drawBuffers1(const GLuint *vaolist) const {
  if (primaryColor.v[3] >= 65535) drawMe(vaolist);
}
void PipeConnector::drawBuffers2(const GLuint *vaolist) const {
  if (activeView.calculating_shadows && primaryColor.v[3] < 45000) return;
  if (primaryColor.v[3] < 65535) drawMe(vaolist);
}
void PipeConnector::drawMe(const GLuint *vaolist) const {
  if (primaryColor.v[3] < 65535) {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
  } else {
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
  }

  int ntries = 0;
  int nverts = 0;
  int detail = 6;
  countObjectSpherePoints(&ntries, &nverts, detail);

  if (activeView.calculating_shadows) {
    setActiveProgramAndUniforms(shaderObjectShadow);
  } else {
    setActiveProgramAndUniforms(shaderObject);
    glUniformC(glGetUniformLocation(shaderObject, "specular"), specularColor);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 128.f / 128.f);
  }
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);
}
void PipeConnector::tick(Real t) {}
