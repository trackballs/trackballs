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

Animated::Animated() : GameHook() {
  Game::current->add(this);
  zero(position);
  for (int i = 0; i < 3; i++) {
    specularColor[i] = 0.0;
    primaryColor[i] = 0.8;
    secondaryColor[i] = 0.3;
  }
  specularColor[3] = primaryColor[3] = secondaryColor[3] = 1.0;
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
void Animated::onRemove() {
  GameHook::onRemove();
  Game::current->remove(this);
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

  double pos[3];

  /* Trigger any callbacks to guile if registered */
  triggerHook(GameHookEvent_Death, NULL);

  pos[0] = position[0];
  pos[1] = position[1];
  pos[2] = position[2] + 0.7;

  if (scoreOnDeath != 0.0) {
    pos[2] += 0.5;
    new ScoreSign((int)scoreOnDeath, pos, SCORESIGN_SCORE);
  }
  if (timeOnDeath != 0.0) {
    pos[2] += 0.5;
    new ScoreSign((int)timeOnDeath, pos, SCORESIGN_TIME);
  }
}
