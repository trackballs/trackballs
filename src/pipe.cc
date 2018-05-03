/* pipe.cc
   One-way pipes dragging balls along them

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

#include "pipe.h"

#include "game.h"

Pipe::Pipe(const Coord3d &f, const Coord3d &t, Real r)
    : Animated(Role_Pipe), from(f), to(t), radius(r) {
  /* Note that the position attribute of Pipes are not used, use rather the to/from values */
  position = 0.5 * (from + to);
  primaryColor[0] = primaryColor[1] = primaryColor[2] = 0.6;

  windBackward = 0.0;
  windForward = 0.0;

  for (int i = 0; i < 3; i++) {
    boundingBox[0][i] = fmin(from[i] - radius, to[i] - radius) - position[i];
    boundingBox[1][i] = fmax(from[i] + radius, to[i] + radius) - position[i];
  }
}

int Pipe::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) const {
  allocateBuffers(1, idxbufs, databufs);
  Coord3d up(0., 0., 0.);
  Coord3d dir = to - from;
  dir = dir / length(dir);
  Coord3d right(dir[1], -dir[0], 0.0);
  right = right / length(right);
  up = crossProduct(dir, right);
  if (up[2] < 0.0) up = -up;

  const int nfacets = 24;

  // Draw pipe
  GLfloat data[2 * nfacets][8];
  ushort idxs[2 * nfacets][3];
  char *pos = (char *)data;
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
    GLfloat norm[3] = {std::sin(angle) * (GLfloat)up[0] + std::cos(angle) * (GLfloat)right[0],
                       std::sin(angle) * (GLfloat)up[1] + std::cos(angle) * (GLfloat)right[1],
                       std::sin(angle) * (GLfloat)up[2] + std::cos(angle) * (GLfloat)right[2]};
    pos += packObjectVertex(
        pos, from[0] + radius * std::sin(angle) * up[0] + radius * std::cos(angle) * right[0],
        from[1] + radius * std::sin(angle) * up[1] + radius * std::cos(angle) * right[1],
        from[2] + radius * std::sin(angle) * up[2] + radius * std::cos(angle) * right[2], 0.,
        0., primaryColor, norm);
    pos += packObjectVertex(
        pos, to[0] + radius * std::sin(angle) * up[0] + radius * std::cos(angle) * right[0],
        to[1] + radius * std::sin(angle) * up[1] + radius * std::cos(angle) * right[1],
        to[2] + radius * std::sin(angle) * up[2] + radius * std::cos(angle) * right[2], 0., 0.,
        primaryColor, norm);
  }
  for (int i = 0; i < nfacets; i++) {
    int j = (i + 1) % nfacets;
    idxs[2 * i][0] = 2 * j;
    idxs[2 * i][1] = 2 * i;
    idxs[2 * i][2] = 2 * j + 1;
    idxs[2 * i + 1][0] = 2 * i;
    idxs[2 * i + 1][1] = 2 * i + 1;
    idxs[2 * i + 1][2] = 2 * j + 1;
  }

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  return 1;
}

void Pipe::drawBuffers1(GLuint *idxbufs, GLuint *databufs) const {
  if (primaryColor[3] >= 1.0) drawTrunk(idxbufs, databufs);
}
void Pipe::drawBuffers2(GLuint *idxbufs, GLuint *databufs) const {
  if (activeView.calculating_shadows && primaryColor[3] < 0.7) return;
  if (primaryColor[3] < 1.0) drawTrunk(idxbufs, databufs);
}
void Pipe::drawTrunk(GLuint *idxbufs, GLuint *databufs) const {
  // Keep off unconditionally since pipe ends show both sides
  glDisable(GL_CULL_FACE);
  if (primaryColor[3] < 1.f) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }
  if (activeView.calculating_shadows) {
    setActiveProgramAndUniforms(shaderObjectShadow);
  } else {
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0] * 0.1,
                specularColor[1] * 0.1, specularColor[2] * 0.1, 1.);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 128.f / 128.f);
  }
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  int nfacets = 24;

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 3 * 2 * nfacets, GL_UNSIGNED_SHORT, (void *)0);
}
void Pipe::tick(Real /*t*/) {}
