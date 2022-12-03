/** \file animated.cc
        The base class for all Animated (ie. drawable) objects.
*/
/*

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

#include "animated.h"
#include "game.h"
#include "map.h"
#include "scoreSign.h"

Animated::Animated(Game& game, int role, int maxVBOpairsOrVAOs)
    : GameHook(game, role), nVOs(maxVBOpairsOrVAOs) {
  primaryColor = Color(SRGBColor(0.8, 0.8, 0.8, 1.));
  secondaryColor = Color(SRGBColor(0.3, 0.3, 0.3, 1.));
  specularColor = Color(SRGBColor(0., 0., 0., 1.));

  texture = 0;

  boundingBox[0][0] = -1.0;
  boundingBox[0][1] = -1.0;
  boundingBox[0][2] = -1.0;
  boundingBox[1][0] = +1.0;
  boundingBox[1][1] = +1.0;
  boundingBox[1][2] = +1.0;

  scoreOnDeath = 0;
  timeOnDeath = 0;
  flags = 0;

  onScreen = true;
  drawChanged = true;

  glidxs = NULL;
  lastFrameNumber = -1;
}
Animated::~Animated() {
  if (glidxs) {
    glDeleteBuffers(nVOs, &glidxs[nVOs * 0]);
    glDeleteBuffers(nVOs, &glidxs[nVOs * 1]);
    glDeleteVertexArrays(nVOs, &glidxs[nVOs * 2]);
    delete[] glidxs;
    glidxs = NULL;
  }
}
void Animated::setupVBOs() {
  if (!glidxs) {
    glidxs = new GLuint[nVOs * 3];
    glGenBuffers(nVOs, &glidxs[nVOs * 0]);
    glGenBuffers(nVOs, &glidxs[nVOs * 1]);
    glGenVertexArrays(nVOs, &glidxs[nVOs * 2]);
    updateBuffers(&glidxs[nVOs * 0], &glidxs[nVOs * 1], &glidxs[nVOs * 2], true);
    drawChanged = true;
  }
}
void Animated::draw() {
  if (displayFrameNumber != lastFrameNumber) {
    lastFrameNumber = displayFrameNumber;
    setupVBOs();
    updateBuffers(&glidxs[nVOs * 0], &glidxs[nVOs * 1], &glidxs[nVOs * 2], false);
    drawChanged = false;
  }
  drawBuffers1(&glidxs[nVOs * 2]);
  /* For debug */
  if (0) drawBoundingBox();
}
void Animated::draw2() {
  if (displayFrameNumber != lastFrameNumber) {
    lastFrameNumber = displayFrameNumber;
    setupVBOs();
    updateBuffers(&glidxs[nVOs * 0], &glidxs[nVOs * 1], &glidxs[nVOs * 2], false);
    drawChanged = false;
  }
  drawBuffers2(&glidxs[nVOs * 2]);
}
void Animated::drawBoundingBox() const {
  if (activeView.calculating_shadows) return;

  /* Create and fill buffers */
  GLuint idxbuf, databuf, vao;
  glGenBuffers(1, &idxbuf);
  glGenBuffers(1, &databuf);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLfloat data[8][3];
  ushort idxs[12][2];
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 3; j++) data[i][j] = position[j] + boundingBox[(i >> j) & 1][j];

  ushort square[4][2] = {{0, 1}, {0, 2}, {2, 3}, {1, 3}};
  for (int i = 0; i < 4; i++) {
    idxs[i][0] = square[i][0];
    idxs[i][1] = square[i][1];
    idxs[i + 4][0] = i;
    idxs[i + 4][1] = i + 4;
    idxs[i + 8][0] = 4 + square[i][0];
    idxs[i + 8][1] = 4 + square[i][1];
  }

  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

  /* Draw buffers */
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  setActiveProgramAndUniforms(Shader_Line);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
  glEnableVertexAttribArray(0);

  glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);
  /* Cleanup buffers */
  glBindVertexArray(0);
  glDeleteBuffers(1, &idxbuf);
  glDeleteBuffers(1, &databuf);
  glDeleteVertexArrays(1, &vao);
}

void Animated::tick(Real dt) { GameHook::tick(dt); }

void Animated::die(int how) {
  (void)how;

  /* Trigger any callbacks to guile if registered */
  triggerHook(GameHookEvent_Death, NULL);

  Coord3d pos(position[0], position[1], position[2] + 0.7);

  if (scoreOnDeath != 0.0) {
    pos[2] += 0.5;
    game.add(new ScoreSign(game, (int)scoreOnDeath, pos, SCORESIGN_SCORE));
  }
  if (timeOnDeath != 0.0) {
    pos[2] += 0.5;
    game.add(new ScoreSign(game, (int)timeOnDeath, pos, SCORESIGN_TIME));
  }
}
