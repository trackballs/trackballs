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

#include <cstring>

Flag::Flag(Game &g, Real x, Real y, int points, int visible, Real radius, int role)
    : Animated(g, role, 1) {
  scoreOnDeath = points;
  timeOnDeath = Game::defaultScores[SCORE_FLAG][1];

  this->visible = visible;
  this->radius = radius;
  position[0] = x;
  position[1] = y;
  position[2] = game.map->getHeight(position[0], position[1]);
  primaryColor = Color(SRGBColor(0.5, 0.5, 1.0, 1.0));
  secondaryColor = Color(SRGBColor(0.8, 0.8, 0.8, 1.0));
}

void Flag::tick(Real t) {
  Animated::tick(t);

  position[2] = game.map->getHeight(position[0], position[1]);
  Player *p = game.player1;
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

FlagRenderer::FlagRenderer() {
  ushort idxs[12][3] = {{0, 1, 2},  {1, 2, 3},  //
                        {4, 5, 6},  {5, 6, 7},   {6, 7, 8},    {7, 8, 9},
                        {8, 9, 10}, {9, 10, 11}, {10, 11, 12}, {11, 12, 13}};

  glGenBuffers(1, &indexBuffer);

  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
}

FlagRenderer::~FlagRenderer() {
  for (std::map<struct FlagDrawState, struct FlagBuffer>::iterator i = buffers.begin();
       i != buffers.end(); i++) {
    std::pair<struct FlagDrawState, struct FlagBuffer> v = *i;
    glDeleteVertexArrays(1, &v.second.vao);
    glDeleteBuffers(1, &v.second.vertexBuffer);
  }
  glDeleteBuffers(1, &indexBuffer);
}

static bool operator<(const struct FlagDrawState &a, const struct FlagDrawState &b) {
  // note: this works since `FlagDrawState` has no padding
  int cmp = memcmp(&a, &b, sizeof(struct FlagDrawState));
  return cmp < 0;
}

void FlagRenderer::draw(std::vector<GameHook *> flags) {
  warnForGLerrors("Starting flag draw");

  // first, drop all buffers which no longer have _any_ matching flags.
  // todo: only do this once per frame
  for (std::map<struct FlagDrawState, struct FlagBuffer>::iterator i = buffers.begin();
       i != buffers.end(); i++) {
    std::pair<struct FlagDrawState, struct FlagBuffer> v = *i;
    buffers[v.first].active = false;
  }
  for (std::vector<GameHook *>::iterator g = flags.begin(); g != flags.end(); g++) {
    Flag *f = dynamic_cast<Flag *>(*g);
    if (!f) {
      warning("Input array contained something that was not a flag");
      return;
    }
    struct FlagDrawState state;
    state.flagColor = f->primaryColor.toOpaque();
    state.poleColor = f->secondaryColor.toOpaque();
    if (buffers.count(state)) { buffers[state].active = true; }
  }
  for (std::map<struct FlagDrawState, struct FlagBuffer>::iterator i = buffers.begin();
       i != buffers.end(); i++) {
    if (!i->second.active) {
      glDeleteVertexArrays(1, &i->second.vao);
      glDeleteBuffers(1, &i->second.vertexBuffer);
      buffers.erase(i);
    }
  }

  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  // then, create or update buffers in all desired configurations
  for (std::vector<GameHook *>::iterator g = flags.begin(); g != flags.end(); g++) {
    Flag *f = (Flag *)(*g);
    if (!f->visible || !f->onScreen) { continue; }

    struct FlagDrawState state;
    state.flagColor = f->primaryColor.toOpaque();
    state.poleColor = f->secondaryColor.toOpaque();
    struct FlagBuffer &buf = buffers[state];
    bool first_use = buf.vertexBuffer == 0;
    if (!first_use && buf.lastTime == f->game.gameTime) {
      // buffer already updated
      continue;
    }

    buf.lastTime = f->game.gameTime;

    // compute the vertex data
    GLfloat data[14 * 8];
    memset(data, 0, sizeof(data));

    // The tail
    GLfloat perp[3] = {-1., -1., 0.};
    char *pos = (char *)data;
    GLfloat ox = 0.03;
    pos += packObjectVertex(pos, ox, 0., 0., 0., 0., state.poleColor, perp);
    pos += packObjectVertex(pos, -ox, 0., 0., 0., 0., state.poleColor, perp);
    pos += packObjectVertex(pos, +ox, 0., 0.71, 0., 0., state.poleColor, perp);
    pos += packObjectVertex(pos, -ox, 0., 0.71, 0., 0., state.poleColor, perp);

    float d1 = buf.lastTime * 1.0f, d2 = 3.0f, d3 = 0.5f;
    GLfloat dx[5] = {
        0.0f, 0.1f * d3 * std::sin(d1 + d2 * 0.1f), 0.2f * d3 * std::sin(d1 + d2 * 0.2f),
        0.3f * d3 * std::sin(d1 + d2 * 0.3f), 0.4f * d3 * std::sin(d1 + d2 * 0.4f)};
    GLfloat dbx[5] = {dx[1] - dx[0], dx[2] - dx[0], dx[3] - dx[1], dx[4] - dx[2],
                      dx[4] - dx[3]};
    GLfloat dby[5] = {0.1f, 0.2f, 0.2f, 0.2f, 0.1f};
    for (int i = 0; i < 5; i++) {
      Coord3d b(dbx[i], dby[i], 0.0);
      Coord3d up(0.0, 0.0, 1.0);
      Coord3d normal = crossProduct(up, b);
      normal = normal / length(normal);
      GLfloat fnorm[3] = {(GLfloat)normal[0], (GLfloat)normal[1], (GLfloat)normal[2]};
      pos += packObjectVertex(pos, dx[i], -0.1 * i, 0.7, 0., 0., state.flagColor, fnorm);
      pos += packObjectVertex(pos, dx[i], -0.1 * i, 0.5, 0., 0., state.flagColor, fnorm);
    }

    if (first_use) {
      glGenBuffers(1, &buf.vertexBuffer);
      glGenVertexArrays(1, &buf.vao);

      glBindVertexArray(buf.vao);
      glBindBuffer(GL_ARRAY_BUFFER, buf.vertexBuffer);
      glBufferData(GL_ARRAY_BUFFER, 14 * 8 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
      configureObjectAttributes();
    } else {
      glBindVertexArray(buf.vao);
      glBindBuffer(GL_ARRAY_BUFFER, buf.vertexBuffer);
      glBufferSubData(GL_ARRAY_BUFFER, 0, 14 * 8 * sizeof(GLfloat), data);
    }
  }

  // second, run the draw operation
  for (std::vector<GameHook *>::iterator g = flags.begin(); g != flags.end(); g++) {
    Flag *f = (Flag *)(*g);
    if (!f->visible || !f->onScreen) { continue; }

    struct FlagDrawState state;
    state.flagColor = f->primaryColor.toOpaque();
    state.poleColor = f->secondaryColor.toOpaque();
    const struct FlagBuffer buf = buffers[state];
    if (buf.vertexBuffer == 0) {
      warning("Failed to prepare buffer for flag");
      continue;
    }

    Matrix4d transform;
    affineMatrix(transform, identity3, f->position);
    setObjectUniforms(uloc, transform, f->specularColor, 10.f / 128.f, Lighting_Regular);
    glBindVertexArray(buf.vao);
    glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_SHORT, (void *)0);
  }

  warnForGLerrors("Ending flag draw");
}
