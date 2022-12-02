/* switch.cc
   A switch which can be triggered by user to accomplish something

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

#include "switch.h"

#include "game.h"
#include "guile.h"
#include "map.h"
#include "player.h"
#include "sound.h"

CSwitch::CSwitch(Game &g, Real x, Real y, SCM on, SCM off)
    : Animated(g, Role_OtherAnimated, 1) {
  position[0] = x;
  position[1] = y;
  position[2] = game.map->getHeight(x, y);
  primaryColor = Color(SRGBColor(0.8, 0.8, 0.8, 1.0));
  secondaryColor = Color(0., 0., 0., 1.);
  is_touched = false;
  this->on = on;
  this->off = off;
  if (on) scm_gc_protect_object(on);
  if (off) scm_gc_protect_object(off);
}
CSwitch::~CSwitch() { CSwitch::releaseCallbacks(); }
void CSwitch::releaseCallbacks() {
  if (on) scm_gc_unprotect_object(on);
  if (off) scm_gc_unprotect_object(off);
  on = NULL;
  off = NULL;
}

void CSwitch::generateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                              const GLuint *vaolist, bool mustUpdate) const {
  const int nfacets = 16;
  GLfloat lever_length = 0.3f;
  GLfloat lever_end = 0.03f;
  GLfloat body_wid = 0.15f;
  GLfloat body_rad = 0.15f;

  GLfloat data[4 * nfacets + 7][8];
  ushort idxs[4 * nfacets + 6][3];

  Color side_color = is_on ? primaryColor : secondaryColor;

  GLfloat flat[3] = {0.f, 0.f, 0.f};
  char *pos = (char *)data;
  // Top curve
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = i * M_PI / (nfacets - 1);
    GLfloat norm[3] = {std::cos(angle), 0., std::sin(angle)};
    pos +=
        packObjectVertex(pos, position[0] + body_wid, position[1] + body_rad * std::cos(angle),
                         position[2] + body_rad * std::sin(angle), 0., 0., primaryColor, norm);
    pos +=
        packObjectVertex(pos, position[0] - body_wid, position[1] + body_rad * std::cos(angle),
                         position[2] + body_rad * std::sin(angle), 0., 0., primaryColor, norm);
  }
  // Side panels
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = i * M_PI / (nfacets - 1);
    pos +=
        packObjectVertex(pos, position[0] + body_wid, position[1] + body_rad * std::cos(angle),
                         position[2] + body_rad * std::sin(angle), 0., 0., side_color, flat);
  }
  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = i * M_PI / (nfacets - 1);
    pos +=
        packObjectVertex(pos, position[0] - body_wid, position[1] + body_rad * std::cos(angle),
                         position[2] + body_rad * std::sin(angle), 0., 0., side_color, flat);
  }
  pos += packObjectVertex(pos, position[0] + body_wid, position[1], position[2], 0., 0.,
                          side_color, flat);
  pos += packObjectVertex(pos, position[0] - body_wid, position[1], position[2], 0., 0.,
                          side_color, flat);

  for (int i = 0; i < nfacets; i++) {
    int j = (i + 1) % nfacets;
    idxs[2 * i][0] = 2 * i;
    idxs[2 * i][1] = 2 * i + 1;
    idxs[2 * i][2] = 2 * j + 1;
    idxs[2 * i + 1][0] = 2 * j;
    idxs[2 * i + 1][1] = 2 * i;
    idxs[2 * i + 1][2] = 2 * j + 1;
  }

  for (int i = 0; i < nfacets; i++) {
    idxs[2 * nfacets + i][0] = 2 * nfacets + i;
    idxs[2 * nfacets + i][1] = 2 * nfacets + (i + 1) % nfacets;
    idxs[2 * nfacets + i][2] = 4 * nfacets;
  }

  for (int i = 0; i < nfacets; i++) {
    idxs[3 * nfacets + i][0] = 3 * nfacets + (i + 1) % nfacets;
    idxs[3 * nfacets + i][1] = 3 * nfacets + i;
    idxs[3 * nfacets + i][2] = 4 * nfacets + 1;
  }

  // Draw Lever
  int sgn = is_on ? 1 : -1;
  pos +=
      packObjectVertex(pos, position[0], position[1], position[2], 0., 0., primaryColor, flat);
  pos += packObjectVertex(
      pos, position[0] + lever_end * 1.4f, position[1] + sgn * (lever_length - lever_end),
      position[2] + (lever_length + lever_end), 0., 0., primaryColor, flat);
  pos += packObjectVertex(
      pos, position[0] - lever_end * 1.4f, position[1] + sgn * (lever_length - lever_end),
      position[2] + (lever_length + lever_end), 0., 0., primaryColor, flat);
  pos += packObjectVertex(
      pos, position[0] - lever_end * 1.4f, position[1] + sgn * (lever_length + lever_end),
      position[2] + (lever_length - lever_end), 0., 0., primaryColor, flat);
  pos += packObjectVertex(
      pos, position[0] + lever_end * 1.4f, position[1] + sgn * (lever_length + lever_end),
      position[2] + (lever_length - lever_end), 0., 0., primaryColor, flat);
  int vstart = 4 * nfacets + 2;
  int istart = 4 * nfacets;
  ushort local[6][3] = {{0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 1}, {1, 3, 2}, {1, 4, 3}};
  for (int i = 0; i < 6; i++) {
    idxs[istart + i][0] = local[i][0] + vstart;
    idxs[istart + i][1] = local[i][1] + vstart;
    idxs[istart + i][2] = local[i][2] + vstart;
  }

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  configureObjectAttributes();
}

void CSwitch::drawBuffers1(const GLuint *vaolist) const {
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  const int nfacets = 16;
  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, specularColor, 0.12, Lighting_Regular);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, (4 * nfacets + 6) * 3, GL_UNSIGNED_SHORT, (void *)0);
}

void CSwitch::drawBuffers2(const GLuint * /*vaolist*/) const {}

void CSwitch::tick(Real t) {
  Animated::tick(t);

  Coord3d v = position - game.player1->position;
  Player *player = game.player1;

  double dist = length(v);
  if (dist < player->radius + 0.3) {
    if (!is_touched) {
      if (is_on) {
        is_on = false;
        game.queueCall(off);
        drawChanged = true;
      } else {
        is_on = true;
        game.queueCall(on);
        drawChanged = true;
      }
      playEffect(SFX_SWITCH);
    }
    is_touched = true;
  } else
    is_touched = false;
}
