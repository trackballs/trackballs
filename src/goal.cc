/* template.cc
   Use to create new .cc project files

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

#include "goal.h"
#include "game.h"
#include "mainMode.h"
#include "map.h"
#include "player.h"

#include <cstring>

Goal::Goal(Game &g, Real x, Real y, int rotate, char *nextLevel)
    : Flag(g, x, y, 1000, 1, 0.2) {
  strncpy(this->nextLevel, nextLevel, sizeof(this->nextLevel));
  this->rotate = rotate;
  primaryColor = Color(SRGBColor(0.9, 0.8, 0.3, 1.0));
  specularColor = Color(SRGBColor(0.95, 0.9, 0.65, 1.0));
}
void Goal::onGet() {
  if (!game.player1->hasWon) {
    strncpy(game.nextLevel, nextLevel, sizeof(game.nextLevel));
    if (game.map->isBonus)
      MainMode::mainMode->bonusLevelComplete();
    else
      MainMode::mainMode->levelComplete();
  }
}

void Goal::updateBuffers(const GLuint *idxbufs, const GLuint *databufs, const GLuint *vaolist,
                         bool firstCall) {
  if (!firstCall || !visible) return;

  const int nfacets = 22;
  GLfloat inner_arc[2 + nfacets][2];
  GLfloat outer_arc[2 + nfacets][2];
  GLfloat normals[2 + nfacets][2];

  GLfloat width = 0.1f;
  GLfloat lift = 0.6f;
  GLfloat irad = 0.39f;
  GLfloat orad = 0.49f;

  inner_arc[0][0] = irad;
  inner_arc[0][1] = 0.0;
  outer_arc[0][0] = orad;
  outer_arc[0][1] = 0.0;
  normals[0][0] = -1.;
  normals[0][1] = 0.;
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = M_PI * i / (nfacets - 1);
    inner_arc[i + 1][0] = irad * std::cos(angle);
    inner_arc[i + 1][1] = lift + irad * std::sin(angle);
    outer_arc[i + 1][0] = orad * std::cos(angle);
    outer_arc[i + 1][1] = lift + orad * std::sin(angle);
    normals[i + 1][0] = std::sin(angle);
    normals[i + 1][1] = std::cos(angle);
  }
  inner_arc[nfacets + 1][0] = -irad;
  inner_arc[nfacets + 1][1] = 0.0;
  outer_arc[nfacets + 1][0] = -orad;
  outer_arc[nfacets + 1][1] = 0.0;
  normals[nfacets + 1][0] = 1.;
  normals[nfacets + 1][1] = 0.;

  GLfloat data[8 * (nfacets + 2) * 8];
  ushort idxs[8 * (nfacets + 1)][3];

  Color color = primaryColor.toOpaque();
  GLfloat loc[3] = {(GLfloat)position[0], (GLfloat)position[1], (GLfloat)position[2]};
  GLfloat flat[3] = {0.f, 0.f, 0.f};

  // Vertex trails
  char *pos = (char *)data;
  // One trail for each point of {+w,-w}x{in,out}x{flat,norm}
  for (int k = 0; k < 8; k++) {
    int curved = k / 4;
    GLfloat delta;
    int inner;
    if (curved) {
      delta = k % 2 ? width : -width;
      inner = ((k / 2) % 2);
    } else {
      delta = ((k / 2) % 2) ? width : -width;
      inner = k % 2;
    }

    for (int i = 0; i < nfacets + 2; i++) {
      GLfloat local[3] = {inner ? inner_arc[i][0] : outer_arc[i][0], delta,
                          inner ? inner_arc[i][1] : outer_arc[i][1]};
      GLfloat cnormal[3] = {normals[i][0], 0, normals[i][1]};
      if (!inner) cnormal[0] *= -1;
      if (!inner) cnormal[2] *= -1;
      if (rotate) std::swap(local[0], local[1]);
      if (rotate) std::swap(cnormal[0], cnormal[1]);
      GLfloat *normal = curved ? cnormal : flat;
      pos += packObjectVertex(pos, loc[0] + local[0], loc[1] + local[1], loc[2] + local[2],
                              0.f, 0.f, color, normal);
    }
  }
  // Triangle strips
  for (int k = 0; k < 4; k++) {
    int swap = (k == 3 || k == 0) ^ rotate;
    for (int i = 0; i < nfacets + 1; i++) {
      idxs[2 * k * (nfacets + 1) + i][0] = 2 * k * (nfacets + 2) + i;
      idxs[2 * k * (nfacets + 1) + i][1] = (2 * k + 1) * (nfacets + 2) + i + swap;
      idxs[2 * k * (nfacets + 1) + i][2] = (2 * k + 1) * (nfacets + 2) + i + 1 - swap;
      idxs[(2 * k + 1) * (nfacets + 1) + i][0] = 2 * k * (nfacets + 2) + i + 1 - swap;
      idxs[(2 * k + 1) * (nfacets + 1) + i][1] = 2 * k * (nfacets + 2) + i + swap;
      idxs[(2 * k + 1) * (nfacets + 1) + i][2] = (2 * k + 1) * (nfacets + 2) + i + 1;
    }
  }

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  configureObjectAttributes();
}

void Goal::drawBuffers1(const GLuint *vaolist) const {
  if (!visible) return;

  const int nfacets = 22;

  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  // Draw it!

  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, identity4, specularColor, 10.f, Lighting_Regular);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, 3 * 8 * (nfacets + 1), GL_UNSIGNED_SHORT, (void *)0);
}

void Goal::drawBuffers2(const GLuint * /*vaolist*/) const {}
