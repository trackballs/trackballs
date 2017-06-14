/* splash.cc
   A splash effect for when the ball hits a liquid. Eyecandy only.

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

#include "splash.h"

#include "settings.h"

Splash::Splash(Coord3d center, Coord3d velocity, GLfloat color[4], double strength,
               double radius) {
  timeLeft = 3.0;
  nDroplets = (int)strength;
  if (nDroplets > 32) nDroplets = 32;
  for (int i = 0; i < 4; i++) primaryColor[i] = color[i];
  for (int i = 0; i < nDroplets; i++) {
    positions[i][0] = center[0] + (frandom() - 0.5) * 2.0 * radius;
    positions[i][1] = center[1] + (frandom() - 0.5) * 2.0 * radius;
    positions[i][2] = center[2];
    velocities[i][0] = velocity[0] + (frandom() - 0.5) * 1.0;
    velocities[i][1] = velocity[1] + (frandom() - 0.5) * 1.0;
    velocities[i][2] = velocity[2] + frandom() * 1.0;
  }
}

int Splash::generateBuffers(GLuint*& idxbufs, GLuint*& databufs) {
  if (Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE) return 0;

  allocateBuffers(1, idxbufs, databufs);

  GLfloat* data = new GLfloat[3 * nDroplets];
  ushort* idxs = new ushort[nDroplets];
  for (int i = 0; i < nDroplets; i++) {
    data[3 * i + 0] = positions[i][0];
    data[3 * i + 1] = positions[i][1];
    data[3 * i + 2] = positions[i][2];
    idxs[i] = i;
  }

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * nDroplets * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nDroplets * sizeof(ushort), idxs, GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  return 1;
}

void Splash::drawBuffers1(GLuint* /*idxbufs*/, GLuint* /*databufs*/) {}

void Splash::drawBuffers2(GLuint* idxbufs, GLuint* databufs) {
  if (Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE) return;

  glEnable(GL_BLEND);

  glPointSize(1.5 * screenWidth / 600.);
  glUseProgram(shaderLine);
  glBindVertexArray(theVao);

  // Pos
  glEnableVertexAttribArray(0);
  setViewUniforms(shaderLine);
  glUniform4f(glGetUniformLocation(shaderLine, "line_color"), primaryColor[0], primaryColor[1],
              primaryColor[2], primaryColor[3]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glDrawElements(GL_POINTS, nDroplets, GL_UNSIGNED_SHORT, (void*)0);
}

void Splash::tick(Real t) {
  timeLeft -= t;
  if (t < 0.0) remove();
  for (int i = 0; i < nDroplets; i++) {
    for (int j = 0; j < 3; j++) positions[i][j] += velocities[i][j] * t;
    velocities[i][2] -= t * 2.0;
  }
}
