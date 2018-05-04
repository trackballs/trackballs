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

Animated::Animated(int role) : GameHook(role) {
  primaryColor = Color(0.8, 0.8, 0.8, 1.);
  secondaryColor = Color(0.3, 0.3, 0.3, 1.);
  specularColor = Color(0., 0., 0., 1.);

  texture = 0;
  // Don't call any inherited version of the computeBoundBox yet to avoid problems
  // with uninitialized variables.
  Animated::computeBoundingBox();

  scoreOnDeath = 0;
  timeOnDeath = 0;
  flags = 0;
  onScreen = 0;

  idxVBOs = NULL;
  dataVBOs = NULL;
  lastFrameNumber = -1;
  nVBOs = 0;
}
Animated::~Animated() {
  if (nVBOs > 0) {
    glDeleteBuffers(nVBOs, idxVBOs);
    glDeleteBuffers(nVBOs, dataVBOs);
    delete[] idxVBOs;
    delete[] dataVBOs;
    nVBOs = 0;
  }
}
void Animated::has_moved() {
  position[2] = Game::current->map->getHeight(position[0], position[1]);
}

void Animated::allocateBuffers(int N, GLuint*& idxbufs, GLuint*& databufs) {
  idxbufs = new GLuint[N];
  databufs = new GLuint[N];
  glGenBuffers(N, idxbufs);
  glGenBuffers(N, databufs);
}

void Animated::draw() {
  if (theFrameNumber != lastFrameNumber) {
    lastFrameNumber = theFrameNumber;
    if (nVBOs > 0) {
      glDeleteBuffers(nVBOs, idxVBOs);
      glDeleteBuffers(nVBOs, dataVBOs);
      delete[] idxVBOs;
      delete[] dataVBOs;
      nVBOs = 0;
      idxVBOs = NULL;
      dataVBOs = NULL;
    }
    nVBOs = generateBuffers(idxVBOs, dataVBOs);
  }
  drawBuffers1(idxVBOs, dataVBOs);
  /* For debug */
  if (0) drawBoundingBox();
}
void Animated::draw2() {
  if (theFrameNumber != lastFrameNumber) {
    lastFrameNumber = theFrameNumber;
    if (nVBOs > 0) {
      glDeleteBuffers(nVBOs, idxVBOs);
      glDeleteBuffers(nVBOs, dataVBOs);
      delete[] idxVBOs;
      delete[] dataVBOs;
      nVBOs = 0;
      idxVBOs = NULL;
      dataVBOs = NULL;
    }
    nVBOs = generateBuffers(idxVBOs, dataVBOs);
  }
  drawBuffers2(idxVBOs, dataVBOs);
}
void Animated::drawBoundingBox() const {
  if (activeView.calculating_shadows) return;

  /* Create and fill buffers */
  GLuint idxbuf, databuf;
  glGenBuffers(1, &idxbuf);
  glGenBuffers(1, &databuf);

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

  setActiveProgramAndUniforms(shaderLine);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, (void*)0);
  /* Cleanup buffers */
  glDeleteBuffers(1, &idxbuf);
  glDeleteBuffers(1, &databuf);
}
void Animated::computeBoundingBox() {
  /* Use a default size 2x2x2 boundingbox around object */
  boundingBox[0][0] = -1.0;
  boundingBox[0][1] = -1.0;
  boundingBox[0][2] = -1.0;
  boundingBox[1][0] = +1.0;
  boundingBox[1][1] = +1.0;
  boundingBox[1][2] = +1.0;
}

void Animated::tick(Real dt) { GameHook::tick(dt); }

void Animated::die(int how) {
  (void)how;

  /* Trigger any callbacks to guile if registered */
  triggerHook(GameHookEvent_Death, NULL);

  Coord3d pos(position[0], position[1], position[2] + 0.7);

  if (scoreOnDeath != 0.0) {
    pos[2] += 0.5;
    Game::current->add(new ScoreSign((int)scoreOnDeath, pos, SCORESIGN_SCORE));
  }
  if (timeOnDeath != 0.0) {
    pos[2] += 0.5;
    Game::current->add(new ScoreSign((int)timeOnDeath, pos, SCORESIGN_TIME));
  }
}
