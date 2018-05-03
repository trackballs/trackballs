/* forcefield.cc
   Implements the forcefield class, preventing passage of ball in one or both directions.

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

#include "forcefield.h"

#include "game.h"

ForceField::ForceField(const Coord3d &pos, const Coord3d &dir, Real h, int a)
    : Animated(Role_Forcefield) {
  position = pos;
  direction = dir;
  height = h;
  allow = a;
  primaryColor[0] = 0.3;
  primaryColor[1] = 1.0;
  primaryColor[2] = 0.3;
  secondaryColor[0] = 1.0;
  secondaryColor[1] = 1.0;
  secondaryColor[2] = 1.0;
  bounceFactor = 2.5;
  boundingBox[0][0] = -abs(dir[0]);
  boundingBox[0][1] = -abs(dir[1]);
  boundingBox[0][2] = -abs(dir[2]) - h;
  boundingBox[1][0] = +abs(dir[0]);
  boundingBox[1][1] = +abs(dir[1]);
  boundingBox[1][2] = +abs(dir[2]) + h;
}
int ForceField::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) const {
  if (!is_on) return 0;
  allocateBuffers(1, idxbufs, databufs);

  Coord3d ndir = direction;
  GLfloat len = length(ndir);
  ndir = ndir / length(ndir);
  // Bound edge width by available space
  GLfloat edge = std::min(std::abs(len), std::abs((GLfloat)height));
  edge = std::min(edge / 5, 0.03f);

  GLfloat color[4] = {primaryColor[0], primaryColor[1], primaryColor[2],
                      0.4f + 0.2f * (GLfloat)frandom()};
  GLfloat rimco[4] = {secondaryColor[0], secondaryColor[1], secondaryColor[2], 0.6};

  GLfloat xycoords[12][2] = {{edge, edge},       {edge, (GLfloat)height - edge},
                             {len - edge, edge}, {len - edge, (GLfloat)height - edge},
                             {edge, edge},       {edge, (GLfloat)height - edge},
                             {len - edge, edge}, {len - edge, (GLfloat)height - edge},
                             {0., 0.},           {0., (GLfloat)height},
                             {len, 0.},          {len, (GLfloat)height}};
  GLfloat *colors[12] = {color, color, color, color, rimco, rimco,
                         rimco, rimco, rimco, rimco, rimco, rimco};

  GLfloat data[12 * 8];
  char *pos = (char *)data;
  for (int i = 0; i < 12; i++) {
    GLfloat flat[3] = {0., 0., 0.};
    pos += packObjectVertex(
        pos, position[0] + xycoords[i][0] * ndir[0], position[1] + xycoords[i][0] * ndir[1],
        position[2] + xycoords[i][0] * ndir[2] + xycoords[i][1], 0., 0., colors[i], flat);
  }

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 12 * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);

  ushort idxs[10][3] = {{0, 2, 1},  {1, 2, 3},   {4, 9, 8},  {4, 5, 9},  {9, 5, 11},
                        {5, 7, 11}, {7, 10, 11}, {7, 6, 10}, {6, 8, 10}, {6, 4, 8}};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 10 * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);

  return 1;
}

void ForceField::drawBuffers1(GLuint * /*idxbufs*/, GLuint * /*databufs*/) const {}

void ForceField::drawBuffers2(GLuint *idxbufs, GLuint *databufs) const {
  if (!is_on) return;
  if (activeView.calculating_shadows) return;

  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);

  setActiveProgramAndUniforms(shaderObject);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0., 0., 0., 1.);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.);
  glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), -1.);
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 10 * 3, GL_UNSIGNED_SHORT, (void *)0);
}

void ForceField::tick(Real /*t*/) {}
