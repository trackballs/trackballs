/* flag.cc
   Gives points when captured

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

#include "flag.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "sound.h"

Flag::Flag(Real x, Real y, int points, int visible, Real radius)
    : Animated(Role_OtherAnimated, 1) {
  scoreOnDeath = points;
  timeOnDeath = Game::defaultScores[SCORE_FLAG][1];

  this->visible = visible;
  this->radius = radius;
  position[0] = x;
  position[1] = y;
  position[2] = Game::current->map->getHeight(position[0], position[1]);
  primaryColor = Color(0.5, 0.5, 1.0, 1.0);
  secondaryColor = Color(0.8, 0.8, 0.8, 1.0);
}

void Flag::generateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                           const GLuint *vaolist, bool /*mustUpdate*/) const {
  if (!visible) return;

  GLfloat data[14 * 8];
  memset(data, 0, sizeof(data));
  ushort idxs[12][3] = {{0, 1, 2},  {1, 2, 3},  //
                        {4, 5, 6},  {5, 6, 7},   {6, 7, 8},    {7, 8, 9},
                        {8, 9, 10}, {9, 10, 11}, {10, 11, 12}, {11, 12, 13}};

  // The tail
  GLfloat perp[3] = {-1., -1., 0.};
  char *pos = (char *)data;
  GLfloat ox = 0.03;
  pos += packObjectVertex(pos, position[0] + ox, position[1], position[2], 0., 0.,
                          secondaryColor, perp);
  pos += packObjectVertex(pos, position[0] - ox, position[1], position[2], 0., 0.,
                          secondaryColor, perp);
  pos += packObjectVertex(pos, position[0] + ox, position[1], position[2] + 0.71, 0., 0.,
                          secondaryColor, perp);
  pos += packObjectVertex(pos, position[0] - ox, position[1], position[2] + 0.71, 0., 0.,
                          secondaryColor, perp);

  float d1 = Game::current->gameTime * 1.0f, d2 = 3.0f, d3 = 0.5f;
  GLfloat dx[5] = {0.0f, 0.1f * d3 * std::sin(d1 + d2 * 0.1f),
                   0.2f * d3 * std::sin(d1 + d2 * 0.2f), 0.3f * d3 * std::sin(d1 + d2 * 0.3f),
                   0.4f * d3 * std::sin(d1 + d2 * 0.4f)};
  GLfloat dbx[5] = {dx[1] - dx[0], dx[2] - dx[0], dx[3] - dx[1], dx[4] - dx[2], dx[4] - dx[3]};
  GLfloat dby[5] = {0.1f, 0.2f, 0.2f, 0.2f, 0.1f};
  Color color = primaryColor.toOpaque();
  for (int i = 0; i < 5; i++) {
    Coord3d b(dbx[i], dby[i], 0.0);
    Coord3d up(0.0, 0.0, 1.0);
    Coord3d normal = crossProduct(up, b);
    normal = normal / length(normal);
    GLfloat fnorm[3] = {(GLfloat)normal[0], (GLfloat)normal[1], (GLfloat)normal[2]};
    pos += packObjectVertex(pos, position[0] + dx[i], position[1] - 0.1 * i, position[2] + 0.7,
                            0., 0., color, fnorm);
    pos += packObjectVertex(pos, position[0] + dx[i], position[1] - 0.1 * i, position[2] + 0.5,
                            0., 0., color, fnorm);
  }

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 14 * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
  configureObjectAttributes();
}

void Flag::drawBuffers1(const GLuint *vaolist) const {
  if (!visible) return;

  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(specularColor, 10.f / 128.f, Lighting_Regular);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_SHORT, (void *)0);
}

void Flag::drawBuffers2(const GLuint * /*vaolist*/) const {}

void Flag::tick(Real t) {
  Animated::tick(t);

  position[2] = Game::current->map->getHeight(position[0], position[1]);
  Player *p = Game::current->player1;
  Coord3d diff;

  diff = position - p->position;
  if (!visible) diff[2] = 0.0;
  if (length(diff) < p->radius + radius) onGet();
}

void Flag::onGet() {
  /* Triggers any events and creates scores/time bonuses */
  die(DIE_OTHER);
  /* Play the sound effect */
  playEffect(SFX_GOT_FLAG);
  remove();
}
