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

#include "general.h"
#include "animated.h"
#include "splash.h"
#include "glHelp.h"
#include "settings.h"

using namespace std;

Splash::Splash(Coord3d center, Coord3d velocity, GLfloat color[4], double strength,
               double radius) {
  int i;

  timeLeft = 3.0;
  nDroplets = (int)strength;
  if (nDroplets > 32) nDroplets = 32;
  for (i = 0; i < 4; i++) primaryColor[i] = color[i];
  for (i = 0; i < nDroplets; i++) {
    positions[i][0] = center[0] + (frandom() - 0.5) * 2.0 * radius;
    positions[i][1] = center[1] + (frandom() - 0.5) * 2.0 * radius;
    positions[i][2] = center[2];
    velocities[i][0] = velocity[0] + (frandom() - 0.5) * 1.0;
    velocities[i][1] = velocity[1] + (frandom() - 0.5) * 1.0;
    velocities[i][2] = velocity[2] + frandom() * 1.0;
  }
}

void Splash::draw() {}
void Splash::draw2() {
  if (Settings::settings->gfx_details <= GFX_DETAILS_SIMPLE) return;

  glEnable(GL_BLEND);

  glPointSize(1.5 * screenWidth / 600.);

  glUseProgram(shaderLine);

  glBindVertexArray(theVao);

  // Pos
  glEnableVertexAttribArray(0);

  GLfloat proj[16];
  GLfloat model[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  glGetFloatv(GL_MODELVIEW_MATRIX, model);
  glUniformMatrix4fv(glGetUniformLocation(shaderLine, "proj_matrix"), 1, GL_FALSE,
                     (GLfloat *)&proj[0]);
  glUniformMatrix4fv(glGetUniformLocation(shaderLine, "model_matrix"), 1, GL_FALSE,
                     (GLfloat *)&model[0]);
  glUniform4f(glGetUniformLocation(shaderLine, "line_color"), primaryColor[0], primaryColor[1],
              primaryColor[2], primaryColor[3]);

  GLfloat *data = new GLfloat[3 * nDroplets];
  ushort *idxs = new ushort[nDroplets];
  for (int i = 0; i < nDroplets; i++) {
    data[3 * i + 0] = positions[i][0];
    data[3 * i + 1] = positions[i][1];
    data[3 * i + 2] = positions[i][2];
    idxs[i] = i;
  }

  GLuint databuf, idxbuf;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);

  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, 3 * nDroplets * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nDroplets * sizeof(ushort), idxs, GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

  glDrawElements(GL_POINTS, nDroplets, GL_UNSIGNED_SHORT, (void *)0);

  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);
}

void Splash::tick(Real t) {
  int i, j;

  timeLeft -= t;
  if (t < 0.0) remove();
  for (i = 0; i < nDroplets; i++) {
    for (j = 0; j < 3; j++) positions[i][j] += velocities[i][j] * t;
    velocities[i][2] -= t * 2.0;
  }
}
