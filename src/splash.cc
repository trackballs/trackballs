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

#include "game.h"
#include "settings.h"

Splash::Splash(Coord3d center, Coord3d velocity, Color color, double strength, double radius)
    : Animated(Role_OtherAnimated, 1) {
  timeLeft = 3.0;
  nDroplets = (int)strength;
  if (nDroplets > 32) nDroplets = 32;
  primaryColor = color;
  for (int i = 0; i < nDroplets; i++) {
    positions[i][0] = center[0] + (frandom() - 0.5) * 2.0 * radius;
    positions[i][1] = center[1] + (frandom() - 0.5) * 2.0 * radius;
    positions[i][2] = center[2];
    velocities[i][0] = velocity[0] + (frandom() - 0.5) * 1.0;
    velocities[i][1] = velocity[1] + (frandom() - 0.5) * 1.0;
    velocities[i][2] = velocity[2] + frandom() * 1.0;
  }
}

void Splash::generateBuffers(const GLuint* idxbufs, const GLuint* databufs,
                             const GLuint* vaolist, bool /*mustUpdate*/) const {
  if (Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE) return;

  GLfloat* data = new GLfloat[3 * nDroplets];
  ushort* idxs = new ushort[nDroplets];
  for (int i = 0; i < nDroplets; i++) {
    data[3 * i + 0] = positions[i][0];
    data[3 * i + 1] = positions[i][1];
    data[3 * i + 2] = positions[i][2];
    idxs[i] = i;
  }

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * nDroplets * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nDroplets * sizeof(ushort), idxs, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);
  delete[] data;
  delete[] idxs;
}

void Splash::drawBuffers1(const GLuint* /*vaolist*/) const {}

void Splash::drawBuffers2(const GLuint* vaolist) const {
  if (Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE) return;
  if (activeView.calculating_shadows) return;

  glEnable(GL_BLEND);

  glPointSize(1.5 * screenWidth / 600.);
  setActiveProgramAndUniforms(Shader_Line);
  glUniformC(uniformLocations.line_color, primaryColor);

  glBindVertexArray(vaolist[0]);
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
