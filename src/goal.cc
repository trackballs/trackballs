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

Goal::Goal(Real x, Real y, int rotate, char *nextLevel) : Flag(x, y, 1000, 1, 0.2) {
  strncpy(this->nextLevel, nextLevel, sizeof(this->nextLevel));
  this->rotate = rotate;
  primaryColor[0] = 0.9;
  primaryColor[1] = 0.8;
  primaryColor[2] = 0.3;
  specularColor[0] = 0.9 * 2.;
  specularColor[1] = 0.8 * 2.;
  specularColor[2] = 0.3 * 2.;

  /* goal is a flag, hence already registered */
}
void Goal::onGet() {
  // TODO. Make sure player is entering the goal from the right direction
  if (!Game::current->player1->hasWon) {
    strncpy(Game::current->nextLevel, nextLevel, sizeof(Game::current->nextLevel));
    if (Game::current->map->isBonus)
      MainMode::mainMode->bonusLevelComplete();
    else
      MainMode::mainMode->levelComplete();
  }
}

int Goal::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) const {
  if (!visible) return 0;
  allocateBuffers(1, idxbufs, databufs);

  const int nfacets = 11;
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

  GLfloat color[4] = {primaryColor[0], primaryColor[1], primaryColor[2], 1.f};
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
      ;
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

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  return 1;
}

void Goal::drawBuffers1(GLuint *idxbufs, GLuint *databufs) const {
  if (!visible) return;

  const int nfacets = 11;

  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  // Draw it!
  if (activeView.calculating_shadows) {
    setActiveProgramAndUniforms(shaderObjectShadow);
  } else {
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0] * 0.1,
                specularColor[1] * 0.1, specularColor[2] * 0.1, 1.);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 128.f / 128.f);
  }
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 3 * 8 * (nfacets + 1), GL_UNSIGNED_SHORT, (void *)0);
}

void Goal::drawBuffers2(GLuint * /*idxbufs*/, GLuint * /*databufs*/) const {}
