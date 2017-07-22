/* ball.cc
   The class ball is used by all animated objects representing balls of different radiuses

   Copyright (C) 2000  Mathias Broxvall
                       Yannick Perret

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

#include "ball.h"

#include "debris.h"
#include "forcefield.h"
#include "game.h"
#include "mainMode.h"
#include "map.h"
#include "pipe.h"
#include "pipeConnector.h"
#include "settings.h"
#include "sound.h"
#include "splash.h"
#include "trampoline.h"

#include <math.h>
#include <set>

class std::set<Ball *> *Ball::balls;
GLfloat Ball::dizzyTexCoords[4] = {0.f, 0.f, 1.f, 1.f};
extern GLuint hiresSphere;

void Ball::init() {
  balls = new std::set<Ball *>();
  loadTexture("dizzy.png");
  dizzyTexCoords[0] = 0.f;
  dizzyTexCoords[1] = 0.f;
  dizzyTexCoords[2] = 1.f;
  dizzyTexCoords[3] = 1.f;
}
void Ball::reset() {
  delete balls;
  balls = new std::set<Ball *>();
}
void Ball::onRemove() {
  Animated::onRemove();
  balls->erase(this);
}

Ball::Ball() : Animated() {
  sink = 0.0;

  ballResolution = BALL_LORES;
  zero(position);
  zero(velocity);
  friction = 1.0;
  rotation[0] = rotation[1] = 0.0;

  primaryColor[0] = 0.8;
  primaryColor[1] = 0.8;
  primaryColor[2] = 0.8;
  secondaryColor[0] = 0.1;
  secondaryColor[1] = 0.1;
  secondaryColor[2] = 0.1;

  gravity = 8.0;
  bounceFactor = 0.8;
  crashTolerance = 7;
  balls->insert(this);
  no_physics = 0;
  inPipe = 0;

  for (int i = 0; i < NUM_MODS; i++) {
    modTimeLeft[i] = 0.0;
    modTimePhaseIn[i] = 0.0;
  }

  identityMatrix(rotations);
  texture = loadTexture("ice.png");
  nitroDebrisCount = 0.0;

  environmentTexture = 0;
  reflectivity = 0.0;
  metallic = 0;
  dontReflectSelf = 0;
}
Ball::~Ball() {
  setReflectivity(0.0, 0);  // This effectivly deallocates all environment maps */
}

int Ball::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  if (!alive) return 0;

  /* We generate the maximum number of buffers, even if they aren't used */
  int N = 7;
  allocateBuffers(N, idxbufs, databufs);

  GLfloat color[4] = {
      primaryColor[0], primaryColor[1], primaryColor[2], 1.,
  };
  GLfloat loc[3] = {(GLfloat)position[0], (GLfloat)position[1], (GLfloat)(position[2] - sink)};
  if (modTimeLeft[MOD_GLASS]) {
    double phase = std::min(modTimePhaseIn[MOD_GLASS] / 2.0, 1.0);
    if (modTimeLeft[MOD_GLASS] > 0)
      phase = std::min(modTimeLeft[MOD_GLASS] / 2.0, phase);
    else
      phase = 1.0;
    double blend = phase;
    color[0] = 0.8 * blend + color[0] * (1.0 - blend);
    color[1] = 0.8 * blend + color[1] * (1.0 - blend);
    color[2] = 0.8 * blend + color[2] * (1.0 - blend);
    color[3] = 0.5 * blend + 1.0 * (1.0 - blend);
  } else if (modTimeLeft[MOD_FROZEN]) {
    double phase = std::min(modTimePhaseIn[MOD_FROZEN] / 2.0, 1.0);
    if (modTimeLeft[MOD_FROZEN] > 0)
      phase = std::min(modTimeLeft[MOD_FROZEN] / 2.0, phase);
    else
      phase = 1.0;
    double blend = phase;
    color[0] = 0.4 * blend + color[0] * (1.0 - blend);
    color[1] = 0.4 * blend + color[1] * (1.0 - blend);
    color[2] = 0.9 * blend + color[2] * (1.0 - blend);
    color[3] = 0.6 * blend + 1.0 * (1.0 - blend);
  }

  {
    /* Construct VBO for main ball */
    int ntries = 0;
    int nverts = 0;
    int detail;
    switch (ballResolution) {
    case BALL_HIRES:
      detail = 8;
      break;
    default:
    case BALL_NORMAL:
      detail = 6;
      break;
    case BALL_LORES:
      detail = 3;
      break;
    }
    countObjectSpherePoints(&ntries, &nverts, detail);
    GLfloat *data = new GLfloat[nverts * 8];
    ushort *idxs = new ushort[ntries * 3];
    Matrix3d rotation;
    if (modTimeLeft[MOD_EXTRA_LIFE]) {
      Matrix4d ref;
      identityMatrix(ref);
      rotateY(M_PI / 3, ref);
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) rotation[i][j] = ref[i][j];
    } else {
      for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) rotation[j][i] = rotations[i][j];
    }
    placeObjectSphere(data, idxs, 0, loc, rotation, radius, detail, color);

    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
    glBufferData(GL_ARRAY_BUFFER, nverts * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ntries * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
    delete[] data;
    delete[] idxs;
  }

  if (modTimeLeft[MOD_SPIKE]) {
    /* spikes correspond to icosahedral faces */
    const GLfloat w = (1 + std::sqrt(5)) / 2;
    Coord3d icoverts[12] = {
        {-1, w, 0},  {1, w, 0},  {-1, -w, 0}, {1, -w, 0}, {0, -1, w},  {0, 1, w},
        {0, -1, -w}, {0, 1, -w}, {w, 0, -1},  {w, 0, 1},  {-w, 0, -1}, {-w, 0, 1},
    };
    ushort icofaces[20][3] = {
        {0, 11, 5},  {0, 5, 1},  {0, 1, 7},  {0, 7, 10}, {0, 10, 11}, {1, 5, 9}, {5, 11, 4},
        {11, 10, 2}, {10, 7, 6}, {7, 1, 8},  {3, 9, 4},  {3, 4, 2},   {3, 2, 6}, {3, 6, 8},
        {3, 8, 9},   {4, 9, 5},  {2, 4, 11}, {6, 2, 10}, {8, 6, 7},   {9, 8, 1},
    };

    GLfloat phase = std::min(modTimePhaseIn[MOD_SPIKE] / 2.0f, 1.0f);
    if (modTimeLeft[MOD_SPIKE] > 0)
      phase = std::min(modTimeLeft[MOD_SPIKE] / 2.0f, phase);
    else
      phase = 1.0;
    GLfloat scale = radius * (0.5 + 0.5 * phase);

    GLfloat sco[4] = {secondaryColor[0], secondaryColor[1], secondaryColor[2], 1.0};
    GLfloat flat[3] = {0.f, 0.f, 0.f};
    Matrix3d trot;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++) { trot[i][j] = rotations[j][i]; }

    GLfloat data[20 * 4][8];
    ushort idxs[20 * 3][3];
    char *pos = (char *)data;
    for (int i = 0; i < 20; i++) {
      Coord3d centroid = {0., 0., 0.};
      for (int j = 0; j < 3; j++) { add(centroid, icoverts[icofaces[i][j]], centroid); }
      for (int k = 0; k < 3; k++) centroid[k] /= 3.0;

      Coord3d spike;
      assign(centroid, spike);
      for (int k = 0; k < 3; k++) spike[k] *= 0.87 * scale;
      Coord3d sub;
      useMatrix(trot, spike, sub);
      pos += packObjectVertex(pos, loc[0] + sub[0], loc[1] + sub[1], loc[2] + sub[2], 0., 0.,
                              sco, flat);
      for (int j = 2; j >= 0; j--) {
        Coord3d edge;
        assign(icoverts[icofaces[i][j]], edge);
        for (int k = 0; k < 3; k++)
          edge[k] = 0.3 * scale * (0.1 * centroid[k] + 0.9 * edge[k]);
        useMatrix(trot, edge, sub);
        pos += packObjectVertex(pos, loc[0] + sub[0], loc[1] + sub[1], loc[2] + sub[2], 0., 0.,
                                sco, flat);
      }
    }

    for (int i = 0; i < 20; i++) {
      idxs[3 * i][0] = 4 * i;
      idxs[3 * i][1] = 4 * i + 2;
      idxs[3 * i][2] = 4 * i + 1;

      idxs[3 * i + 1][0] = 4 * i;
      idxs[3 * i + 1][1] = 4 * i + 3;
      idxs[3 * i + 1][2] = 4 * i + 2;

      idxs[3 * i + 2][0] = 4 * i;
      idxs[3 * i + 2][1] = 4 * i + 1;
      idxs[3 * i + 2][2] = 4 * i + 3;
    }

    glBindBuffer(GL_ARRAY_BUFFER, databufs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  }

  if (modTimeLeft[MOD_SPEED]) {
    const int nlines = 8;

    GLfloat data[2 * nlines][3];
    ushort idxs[2 * nlines];
    for (int i = 0; i < nlines; i++) {
      double angle = i * M_PI / (nlines - 1) - M_PI / 2;

      Coord3d v;
      assign(velocity, v);
      v[2] = 0.0;
      if (length(v) > 0.8) {
        normalize(v);
        double z = (rand() % 1000) / 1000.0;

        data[2 * i][0] = loc[0] + std::sin(angle) * radius * v[1];
        data[2 * i][1] = loc[1] + std::sin(angle) * radius * v[1];
        data[2 * i][2] = loc[2] + std::cos(angle) * radius * z;
        data[2 * i + 1][0] =
            loc[0] + std::sin(angle) * radius * v[1] - velocity[0] * 0.4 * std::cos(angle);
        data[2 * i + 1][1] =
            loc[1] + std::sin(angle) * radius * -v[0] - velocity[1] * 0.4 * std::cos(angle);
        data[2 * i + 1][2] = loc[2] + std::cos(angle) * radius * z - velocity[2] * 0.2;
      }

      idxs[2 * i] = 2 * i;
      idxs[2 * i + 1] = 2 * i + 1;
    }

    glBindBuffer(GL_ARRAY_BUFFER, databufs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  }

  // Handle modifiers
  if (modTimeLeft[MOD_FLOAT]) {
    const GLfloat stripeA[4] = {1.0, 1.0, 1.0, 1.0};
    const GLfloat stripeB[4] = {1.0, 0.2, 0.2, 1.0};

    GLfloat data[60][8];
    ushort idxs[40][3];
    char *pos = (char *)data;
    for (int i = 0; i < 10; i++) {
      GLfloat a0 = (i / 10.0) * 2. * M_PI, a1 = ((i + 1) / 10.0) * 2. * M_PI;
      const GLfloat *ringcolor = i % 2 ? stripeA : stripeB;

      GLfloat ir = radius * 1.0f, mr = radius * 1.2f, lr = radius * 1.4f, h = radius * 0.2f;

      GLfloat inormal0[3] = {-std::sin(a0), -std::cos(a0), 0.f};
      GLfloat inormal1[3] = {-std::sin(a1), -std::cos(a1), 0.f};
      GLfloat onormal0[3] = {std::sin(a0), std::cos(a0), 0.f};
      GLfloat onormal1[3] = {std::sin(a1), std::cos(a1), 0.f};
      GLfloat unormal[3] = {0.f, 0.f, 1.f};

      pos += packObjectVertex(pos, loc[0] + std::sin(a0) * ir, loc[1] + std::cos(a0) * ir,
                              loc[2], 0., 0., ringcolor, inormal0);
      pos += packObjectVertex(pos, loc[0] + std::sin(a1) * ir, loc[1] + std::cos(a1) * ir,
                              loc[2], 0., 0., ringcolor, inormal1);

      pos += packObjectVertex(pos, loc[0] + std::sin(a0) * mr, loc[1] + std::cos(a0) * mr,
                              loc[2] + h, 0., 0., ringcolor, unormal);
      pos += packObjectVertex(pos, loc[0] + std::sin(a1) * mr, loc[1] + std::cos(a1) * mr,
                              loc[2] + h, 0., 0., ringcolor, unormal);

      pos += packObjectVertex(pos, loc[0] + std::sin(a0) * lr, loc[1] + std::cos(a0) * lr,
                              loc[2], 0., 0., ringcolor, onormal0);
      pos += packObjectVertex(pos, loc[0] + std::sin(a1) * lr, loc[1] + std::cos(a1) * lr,
                              loc[2], 0., 0., ringcolor, onormal1);

      ushort pts[4][3] = {{1, 0, 3}, {0, 2, 3}, {2, 5, 3}, {2, 4, 5}};
      for (int k = 0; k < 12; k++) { idxs[4 * i + k / 3][k % 3] = pts[k / 3][k % 3] + 6 * i; }
    }
    glBindBuffer(GL_ARRAY_BUFFER, databufs[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  }

  if (modTimeLeft[MOD_EXTRA_LIFE]) {
    int ntries = 0;
    int nverts = 0;
    int detail = 5;
    countObjectSpherePoints(&ntries, &nverts, detail);
    GLfloat *data = new GLfloat[nverts * 8];
    ushort *idxs = new ushort[ntries * 3];
    GLfloat color[4] = {primaryColor[0], primaryColor[1], primaryColor[2], 0.5f};
    Matrix3d identity = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};
    placeObjectSphere(data, idxs, 0, loc, identity, radius * 1.25, detail, color);

    glBindBuffer(GL_ARRAY_BUFFER, databufs[4]);
    glBufferData(GL_ARRAY_BUFFER, nverts * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[4]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ntries * 3 * sizeof(ushort), idxs, GL_STATIC_DRAW);
    delete[] data;
    delete[] idxs;
  }

  if (modTimeLeft[MOD_JUMP]) {
    GLfloat data[9][8];
    ushort idxs[3][3];
    char *pos = (char *)data;
    GLfloat z = 1.1 + 1.0 * fmod(Game::current->gameTime, 1.0);
    GLfloat frad = (GLfloat)radius;
    GLfloat corners[9][2] = {
        {frad * -.3f, frad * z},         {frad * .3f, frad * (z + .9f)},
        {frad * .3f, frad * z},          {frad * .3f, frad * (z + .9f)},
        {frad * -.3f, frad * z},         {frad * -.3f, frad * (z + .9f)},
        {frad * -.6f, frad * (z + .9f)}, {0.f, frad * (z + 2.1f)},
        {frad * .6f, frad * (z + .9f)},
    };

    GLfloat color[4] = {1.f, 1.f, 1.f, 0.5f};
    GLfloat flat[3] = {0.f, 0.f, 0.f};
    GLfloat spin = Game::current->gameTime * 0.15;
    for (int i = 0; i < 9; i++) {
      pos += packObjectVertex(pos, loc[0] + std::cos(spin) * corners[i][0],
                              loc[1] + std::sin(spin) * corners[i][0], loc[2] + corners[i][1],
                              0., 0., color, flat);
      idxs[i / 3][i % 3] = i;
    }

    glBindBuffer(GL_ARRAY_BUFFER, databufs[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[5]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  }

  if (modTimeLeft[MOD_DIZZY]) {
    GLfloat data[12][8];
    char *pos = (char *)data;
    GLfloat color[4] = {1.f, 1.f, 1.f, 0.5f};
    GLfloat flat[3] = {0.f, 0.f, 0.f};
    for (int i = 0; i < 3; i++) {
      GLfloat angle = 0.5 * M_PI * Game::current->gameTime + 2 * i * M_PI / 3;
      GLfloat frad = radius;
      GLfloat corners[4][3] = {{-0.6f * frad, 1.5f * frad, -0.1f * frad},
                               {+0.6f * frad, 1.5f * frad, -0.1f * frad},
                               {+0.6f * frad, 1.5f * frad, 1.1f * frad},
                               {-0.6f * frad, 1.5f * frad, 1.1f * frad}};
      GLfloat txco[4][2] = {
          {dizzyTexCoords[0], dizzyTexCoords[1] + dizzyTexCoords[3]},
          {dizzyTexCoords[0] + dizzyTexCoords[2], dizzyTexCoords[1] + dizzyTexCoords[3]},
          {dizzyTexCoords[0] + dizzyTexCoords[2], dizzyTexCoords[1]},
          {dizzyTexCoords[0], dizzyTexCoords[1]}};
      for (int k = 0; k < 4; k++) {
        pos += packObjectVertex(
            pos, loc[0] - std::sin(angle) * corners[k][0] + std::cos(angle) * corners[k][1],
            loc[1] + std::cos(angle) * corners[k][0] + std::sin(angle) * corners[k][1],
            loc[2] + corners[k][2], txco[k][0], txco[k][1], color, flat);
      }
    }

    ushort idxs[6][3] = {{0, 1, 2}, {0, 3, 2}, {4, 5, 6}, {4, 7, 6}, {8, 9, 10}, {8, 11, 10}};

    glBindBuffer(GL_ARRAY_BUFFER, databufs[6]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (idxbufs)[6]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);
  }

  return N;
}

void Ball::drawBuffers1(GLuint *idxbufs, GLuint *databufs) {
  if (!alive) return;
  if (dontReflectSelf) return;

  GLfloat specular[4];
  for (int i = 0; i < 3; i++) { specular[i] = specularColor[i]; }
  specular[3] = 1.0;
  double shininess = 20.0;

  // Handle primary ball..
  if (modTimeLeft[MOD_GLASS]) {
    double phase = std::min(modTimePhaseIn[MOD_GLASS] / 2.0, 1.0);
    if (modTimeLeft[MOD_GLASS] > 0)
      phase = std::min(modTimeLeft[MOD_GLASS] / 2.0, phase);
    else
      phase = 1.0;
    double blend = phase;
    specular[0] = 2.5 * blend + specular[0] * (1.0 - blend);
    specular[1] = 2.5 * blend + specular[1] * (1.0 - blend);
    specular[2] = 2.5 * blend + specular[2] * (1.0 - blend);
    shininess = 50.0 * blend + shininess * (1.0 - blend);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
  } else if (modTimeLeft[MOD_FROZEN]) {
    double phase = std::min(modTimePhaseIn[MOD_FROZEN] / 2.0, 1.0);
    if (modTimeLeft[MOD_FROZEN] > 0)
      phase = std::min(modTimeLeft[MOD_FROZEN] / 2.0, phase);
    else
      phase = 1.0;
    double blend = phase;
    specular[0] = 2.0 * blend + specular[0] * (1.0 - blend);
    specular[1] = 2.0 * blend + specular[1] * (1.0 - blend);
    specular[2] = 4.0 * blend + specular[2] * (1.0 - blend);
    shininess = 50.0 * blend + shininess * (1.0 - blend);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
  } else {
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
  }

  {
    int ntries = 0;
    int nverts = 0;
    int detail;
    switch (ballResolution) {
    case BALL_HIRES:
      detail = 8;
      break;
    default:
    case BALL_NORMAL:
      detail = 6;
      break;
    case BALL_LORES:
      detail = 3;
      break;
    }
    countObjectSpherePoints(&ntries, &nverts, detail);

    // Draw the ball...
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), specular[0] * 0.5,
                specular[1] * 0.5, specular[2] * 0.5, specular[3] * 0.5);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), shininess);
    if (modTimeLeft[MOD_EXTRA_LIFE]) {
      glBindTexture(GL_TEXTURE_2D, textures[loadTexture("track.png")]);
    } else if (texture == 0) {
      glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);
    } else {
      glBindTexture(GL_TEXTURE_2D, textures[texture]);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);

    if (Settings::settings->doReflections && reflectivity > 0.0 && environmentTexture) {
      GLfloat c[4];
      if (metallic) {
        c[0] = primaryColor[0];
        c[1] = primaryColor[1];
        c[2] = primaryColor[2];
        c[3] = reflectivity;
      } else {
        c[0] = 1.0;
        c[1] = 1.0;
        c[2] = 1.0;
        c[3] = reflectivity;
      }

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      setActiveProgramAndUniforms(shaderReflection);
      glUniform4f(glGetUniformLocation(shaderReflection, "refl_color"), c[0], c[1], c[2],
                  c[3]);
      glUniform1i(glGetUniformLocation(shaderReflection, "tex"), 0);
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, environmentTexture);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
      glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
      configureObjectAttributes();
      glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);
    }
  }

  if (modTimeLeft[MOD_SPIKE]) {
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    // Transfer
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.1f, 0.1f, 0.1f, 1.f);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 10.f);
    glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[1]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 20 * 3 * 3, GL_UNSIGNED_SHORT, (void *)0);
  }

  if (modTimeLeft[MOD_SPEED] && !activeView.calculating_shadows) {
    glEnable(GL_BLEND);
    glLineWidth(1.0);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    setActiveProgramAndUniforms(shaderLine);
    glUniform4f(glGetUniformLocation(shaderLine, "line_color"), 1.0f, 1.0f, 1.0f, 0.5f);

    const int nlines = 8;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[2]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glDrawElements(GL_LINES, 2 * nlines, GL_UNSIGNED_SHORT, (void *)0);
  }

  // Handle modifiers
  if (modTimeLeft[MOD_FLOAT]) {
    glDisable(GL_BLEND);
    // In case we look from below
    glDisable(GL_CULL_FACE);

    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.1f, 0.1f, 0.1f, 1.f);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 10.f);
    glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[3]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 40 * 3, GL_UNSIGNED_SHORT, (void *)0);
  }
}

void Ball::drawBuffers2(GLuint *idxbufs, GLuint *databufs) {
  if (!alive) return;
  if (dontReflectSelf) return;
  if (activeView.calculating_shadows) return;

  if (modTimeLeft[MOD_EXTRA_LIFE]) {
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    int ntries = 0;
    int nverts = 0;
    int detail = 5;
    countObjectSpherePoints(&ntries, &nverts, detail);
    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.f, 0.f, 0.f, 0.f);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.f);
    glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[4]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[4]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);
  }
  if (modTimeLeft[MOD_JUMP]) {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.f, 0.f, 0.f, 0.f);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.f);
    glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), -1.);
    glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[5]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[5]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, (void *)0);
  }

  if (modTimeLeft[MOD_DIZZY]) {
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    setActiveProgramAndUniforms(shaderObject);
    glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.f, 0.f, 0.f, 0.f);
    glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.f);
    glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), -1.);
    glBindTexture(GL_TEXTURE_2D, textures[loadTexture("dizzy.png")]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[6]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[6]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, (void *)0);
  }
}

void Ball::doExpensiveComputations() {
  if (reflectivity <= 0.0 || !Settings::settings->doReflections) return;

  // Skip if far enough from camera
  double dx = position[0] - MainMode::mainMode->camFocus[0];
  double dy = position[1] - MainMode::mainMode->camFocus[1];
  if (dx * dx + dy * dy > 50 * 50) { return; }

  int vis =
      testBboxClip(position[0] + boundingBox[0][0], position[0] + boundingBox[1][0],
                   position[1] + boundingBox[0][1], position[1] + boundingBox[1][1],
                   position[2] + boundingBox[0][2], position[2] + boundingBox[1][2],
                   MainMode::mainMode->cameraModelView, MainMode::mainMode->cameraProjection);
  if (!vis) return;

  if (environmentTexture == 0) glGenTextures(1, &environmentTexture);
  glBindTexture(GL_TEXTURE_2D, environmentTexture);

  dontReflectSelf = 1;
  MainMode::mainMode->renderEnvironmentTexture(environmentTexture, position);
  dontReflectSelf = 0;
}

void Ball::tick(Real time) {
  double phase;

  Animated::tick(time);

  for (int i = 0; i < NUM_MODS; i++) {
    if (!modTimeLeft[i])
      modTimePhaseIn[i] = 0.0;
    else
      modTimePhaseIn[i] += time;

    if (modTimeLeft[i] > 0.0) {
      modTimeLeft[i] -= time;
      if (modTimeLeft[i] < 0) modTimeLeft[i] = 0.0;
    }
  }

  /* Nitro balls create "debris" clouds constantly */
  if (modTimeLeft[MOD_NITRO]) {
    nitroDebrisCount += time;
    while (nitroDebrisCount > 0.0) {
      nitroDebrisCount -= 0.25;
      Debris *d = new Debris(this, position, velocity, 2.0 + 2.0 * frandom());
      d->position[0] += (frandom() - 0.5) * radius;
      d->position[1] += (frandom() - 0.5) * radius;
      d->position[2] += radius * 1.0;
      d->velocity[2] += 0.2;
      d->gravity = -0.1;
      d->modTimeLeft[MOD_GLASS] = -1.0;
      d->primaryColor[0] = 0.1;
      d->primaryColor[1] = 0.6;
      d->primaryColor[2] = 0.1;
      d->no_physics = 1;
    }
  }

  radius = realRadius;
  if (modTimeLeft[MOD_LARGE]) {
    phase = std::min(modTimePhaseIn[MOD_LARGE] / 5.0, 1.0);
    if (modTimeLeft[MOD_LARGE] > 0)
      phase = std::min(modTimeLeft[MOD_LARGE] / 3.0, phase);
    else
      phase = 1.0;
    radius *= 1.0 + phase;
  }
  if (modTimeLeft[MOD_SMALL]) {
    phase = std::min(modTimePhaseIn[MOD_SMALL] / 5.0, 1.0);
    if (modTimeLeft[MOD_SMALL] > 0)
      phase = std::min(modTimeLeft[MOD_SMALL] / 3.0, phase);
    else
      phase = 1.0;
    radius /= 1.0 + phase;
  }
  boundingBox[0][0] = -radius;
  boundingBox[0][1] = -radius;
  boundingBox[0][2] = -radius;
  boundingBox[1][0] = radius;
  boundingBox[1][1] = radius;
  boundingBox[1][2] = radius;
  physics(time);
}

void Ball::setReflectivity(double reflectivity, int metallic) {
  this->reflectivity = reflectivity;
  this->metallic = metallic;
  if (reflectivity <= 0.0 && environmentTexture != 0) {
    /* Deallocate environment texture */
    glDeleteTextures(1, &environmentTexture);
    environmentTexture = 0;
  }
}

Boolean Ball::physics(Real time) {
  Real x, y;

  if (!Game::current) return true;
  Map *map = Game::current->map;

  if (modTimeLeft[MOD_DIZZY]) {
    /* Explanation. We cast the address of the ball into an integer in order to get
       a unique random seed for every ball. Could possibly cause problems on 64 bit platforms,
       have to
       look into it. */
    rotation[0] += time * 7.0 * (frand((long)Game::current->gameTime + (long)this) - 0.5);
    rotation[1] += time * 7.0 * (frand(47 + (long)Game::current->gameTime + (long)this) - 0.5);
  }

  /*  rotateX(-rotation[1]*time*2.0*M_PI*radius,rotations);
          rotateY(-rotation[0]*time*2.0*M_PI*radius,rotations);*/
  rotateX(-rotation[1] * time * 2.0 * M_PI * 0.3 * 0.3 / radius, rotations);
  rotateY(-rotation[0] * time * 2.0 * M_PI * 0.3 * 0.3 / radius, rotations);

  Cell &c = map->cell((int)position[0], (int)position[1]);
  Coord3d normal;
  c.getNormal(normal, Cell::CENTER);

  Real mapHeight = map->getHeight(position[0], position[1]);

  /* All effects of gravity */
  if (inTheAir)
    velocity[2] = velocity[2] - gravity * time;
  else if (!inPipe) {
    double scale = gravity * time / normal[2];
    velocity[0] += normal[0] * scale;  // gravity * time;
    velocity[1] += normal[1] * scale;  // gravity * time;
  }

  /* Sand - generate debris */
  if (!inPipe && c.flags & CELL_SAND && !inTheAir && radius > 0.2) {
    double speed =
        velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
    if (frandom() < (speed - 0.3) * 0.08) {
      /* lots of friction when we crash into sand */
      velocity[0] *= 0.9;
      velocity[1] *= 0.9;
      velocity[2] *= 0.9;
      generateSandDebris();
    }
  } else if (Settings::settings->difficulty > 0 && !inPipe && modTimeLeft[MOD_SPIKE] &&
             !inTheAir && radius > 0.2) {
    double speed =
        velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
    if (frandom() < (speed - 0.3) * 0.0007 * Settings::settings->difficulty) {
      velocity[0] *= 0.9;
      velocity[1] *= 0.9;
      velocity[2] *= 0.9;
      generateDebris(c.colors[Cell::CENTER]);
    }
  }

  /* All effects of water */
  {
    double waterHeight = map->getWaterHeight(position[0], position[1]);

    /* Floating */
    if (modTimeLeft[MOD_FLOAT] && !inPipe && waterHeight > position[2] &&
        waterHeight > mapHeight + radius + 0.025) {
      velocity[2] += gravity * time * 1.5;
      velocity[2] *= 0.99;
      // avoid sticking to the ground
      double delta = position[2] - radius - map->getHeight(position[0], position[1]);
      if (delta < 0.025) {
        position[2] += 0.025 - delta;
        if (velocity[2] < 0.0) velocity[2] = 0.0;
      }
    }

    if (position[2] - radius < waterHeight) {
      double depth = waterHeight - (position[2] - radius);
      // splashes caused by speed
      double speed = velocity[0] * velocity[0] + velocity[1] * velocity[1] +
                     velocity[2] * velocity[2] * 5.;
      if (frandom() < speed * 0.001 * (depth < 0.5 ? depth : 1.0 - depth) * radius / 0.3) {
        GLfloat waterColor[4] = {0.4, 0.4, 0.8, 0.5};
        Coord3d center;
        assign(position, center);
        center[2] = map->getWaterHeight(center[0], center[1]);
        new Splash(center, velocity, waterColor, 30 * radius / 0.3,
                   radius);  // speed*radius*(depth<0.5?depth:1.0-depth)*2.0,radius);
      }
      // splashes caused by rotation. eg "swimming"
      speed = rotation[0] * rotation[0] + rotation[1] * rotation[1];
      if (frandom() < speed * 0.001) {
        GLfloat waterColor[4] = {0.4, 0.4, 0.8, 0.5};
        Coord3d center;
        assign(position, center);
        center[2] = map->getWaterHeight(center[0], center[1]);
        Coord3d vel;
        vel[0] = -rotation[0] * radius;  // 0.3;
        vel[1] = -rotation[1] * radius;  // 0.3;
        vel[2] = 0.2;
        rotation[0] *= 0.9;
        rotation[1] *= 0.9;
        velocity[0] += 0.01 * rotation[0];
        velocity[1] += 0.01 * rotation[1];
        new Splash(center, vel, waterColor, (int)speed * 0.5, radius);
      }
      double fric = 0.004 * std::min(1.0, depth / (2. * radius));  // an extra water friction
      velocity[0] = velocity[0] * (1. - fric) + c.velocity[0] * fric;
      velocity[1] = velocity[1] * (1. - fric) + c.velocity[1] * fric;
      fric = 0.008;
      velocity[2] = velocity[2] * (1. - fric) + c.velocity[1] * fric;
    }
  }

  if (!inPipe && !inTheAir && c.flags & CELL_KILL) {
    die(DIE_OTHER);
    return false;
  }

  /* Sinking into floor material */
  if (c.flags & CELL_SAND && !inPipe) {
    sink += 0.8 * time;
    if (sink > radius * 0.5) sink = radius * 0.5;
  } else if (c.flags & CELL_ACID && !inPipe && !inTheAir) {
    sink += 0.8 * time;
    double speed =
        velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
    if (frandom() < (speed - 0.2) * 0.05) {
      GLfloat acidColor[4] = {0.1, 0.5, 0.1, 0.5};
      Coord3d center;
      assign(position, center);
      center[2] = map->getHeight(center[0], center[1]);
      new Splash(center, velocity, acidColor, speed * radius, radius);
    }
    if (modTimeLeft[MOD_GLASS]) sink = std::min(sink, 0.3);
    if (sink > radius * 2.0) {
      die(DIE_ACID);
      return false;
    }
  } else
    sink = std::max(0.0, sink - 2.0 * time);

  /*                                      */
  /* Ground "grip" - Also works in pipes! */
  /*                                      */
  {
    double v_fric = 0.08;
    double r_fric = 0.10;

    if (c.flags & CELL_ACID) {
      v_fric = 0.008;
      r_fric = 0.010;
    }

    if (c.flags & CELL_ICE) {
      if (modTimeLeft[MOD_SPIKE]) {
        v_fric = 0.008;
        r_fric = 0.010;
      } else {
        v_fric = 0.0008;
        r_fric = 0.0010;
      }
    }

    if (inTheAir) {
      if (map->getWaterHeight(position[0], position[1]) > position[2] - radius) {
        v_fric = 0.0005;
        r_fric = 0.0005;
      } else
        v_fric = r_fric = 0.0;
    }

    if (inPipe) {
      v_fric = 0.08;
      r_fric = 0.10;
    }

    if (c.flags & CELL_TRACK) {
      velocity[0] = velocity[0] * (1.0 - v_fric) + (rotation[0] + c.velocity[0]) * v_fric;
      velocity[1] = velocity[1] * (1.0 - v_fric) + (rotation[1] + c.velocity[1]) * v_fric;
      rotation[0] = rotation[0] * (1.0 - r_fric) + (velocity[0] - c.velocity[0]) * r_fric;
      rotation[1] = rotation[1] * (1.0 - r_fric) + (velocity[1] - c.velocity[1]) * r_fric;
    } else {
      velocity[0] = velocity[0] * (1.0 - v_fric) + rotation[0] * v_fric;
      velocity[1] = velocity[1] * (1.0 - v_fric) + rotation[1] * v_fric;
      rotation[0] = rotation[0] * (1.0 - r_fric) + velocity[0] * r_fric;
      rotation[1] = rotation[1] * (1.0 - r_fric) + velocity[1] * r_fric;
    }
  }

  /* rotational friction - limits the speed when on ice or in the air, water etc. */
  rotation[0] *= 0.9995;
  rotation[1] *= 0.9995;
  double effective_friction = 0.001 * friction;
  if (inTheAir && !(map->getWaterHeight(position[0], position[1]) > position[2] - radius))
    effective_friction *= 0.1;
  else if (!inTheAir) {
    if (inPipe) {
    } else {
      if (c.flags & CELL_ACID) effective_friction *= 2.0;
      if (c.flags & CELL_ICE) effective_friction *= 0.1;
      if (c.flags & CELL_SAND)
        effective_friction *= (c.flags & CELL_TRACK ? 20.0 : 10.0);
      else if (c.flags & CELL_TRACK)
        effective_friction *= 4.0;  // Note. not both sand and track effect
    }
    if (modTimeLeft[MOD_SPIKE]) effective_friction *= 1.5;
    if (modTimeLeft[MOD_SPEED]) effective_friction *= 0.5;
  }
  for (int i = 0; i < 2; i++)
    velocity[i] = velocity[i] - (velocity[i] - c.velocity[i]) * effective_friction;
  velocity[2] *= 1.0 - effective_friction;
  if (inTheAir && velocity[2] > 5.0) velocity[2] *= 0.995;

  for (int i = 0; i < 3; i++) position[i] += time * velocity[i];

  if (!inPipe) {
    handleEdges();

    /* Ground collisions */
    if (!checkGroundCollisions(map, 0, 0)) return false;
    for (x = -radius + 0.05; x <= radius - 0.05; x += 0.05)
      if (!checkGroundCollisions(map, x, 0.0)) return false;
    for (y = -radius + 0.05; y <= radius - 0.05; y += 0.05)
      if (!checkGroundCollisions(map, 0.0, y)) return false;
  }

  /* Collisions with other balls */
  handleBallCollisions();

  /* Collisions with forcefields */
  handleForcefieldCollisions();

  double dh = position[2] - radius - map->getHeight(position[0], position[1]);
  if (dh > 0.1) inTheAir = true;

  /* Pipes */
  handlePipes(time);
  return true;
}
Boolean Ball::checkGroundCollisions(Map *map, Real x, Real y) {
  Real dh = position[2] - sqrt(radius * radius - x * x - y * y) -
            map->getHeight(position[0] + x, position[1] + y);
  if (dh < 0.02) {
    if (inTheAir) {
      /* We where in the air and have now hit the ground. Calculate
             a bounce */

      /* first, change speed etc. if we are rotating */
      double v_fric = 0.2;
      double r_fric = 0.4;
      velocity[0] = velocity[0] * (1.0 - v_fric) + rotation[0] * v_fric;
      velocity[1] = velocity[1] * (1.0 - v_fric) + rotation[1] * v_fric;
      rotation[0] = rotation[0] * (1.0 - r_fric) + velocity[0] * r_fric;
      rotation[1] = rotation[1] * (1.0 - r_fric) + velocity[1] * r_fric;

      Coord3d normal;
      Cell &c = map->cell((int)(position[0] + x), (int)(position[1] + y));
      /* TODO: Use correct normal */
      c.getNormal(normal, Cell::CENTER);

      Real speed = -dotProduct(velocity, normal);  // or simply -velocity[2];
      if (speed > 0) {
        int tx = (int)(position[0] + x), ty = (int)(position[1] + y);
        Cell &cell = map->cell(tx, ty);

        double crash_speed = speed;
        if (cell.flags & (CELL_TRAMPOLINE | CELL_SAND)) crash_speed *= 0.4;
        if (modTimeLeft[MOD_JUMP]) crash_speed *= 0.8;

        if (!crash(crash_speed)) return false;
        double effective_bounceFactor = bounceFactor;
        if (cell.flags & CELL_ACID) effective_bounceFactor = 0.0;
        if (cell.flags & CELL_SAND) effective_bounceFactor = 0.1;
        if (cell.flags & CELL_TRAMPOLINE) {
          Real dh = 1.0 * speed * radius * radius * radius;
          effective_bounceFactor += 0.6;
          for (int i = 0; i < 5; i++) cell.heights[i] -= dh;
          if (cell.sunken <= 0.0) new Trampoline(tx, ty);
          cell.sunken += dh;
        }
        speed *= 1.0 + effective_bounceFactor;
        velocity[0] += normal[0] * speed;
        velocity[1] += normal[1] * speed;
        velocity[2] += normal[2] * speed;

        if (cell.flags & CELL_ACID) {
          GLfloat acidColor[4] = {0.1, 0.5, 0.1, 0.5};
          Coord3d center;
          assign(position, center);
          center[2] = map->getHeight(center[0], center[1]);
          new Splash(center, velocity, acidColor, speed * radius * 20.0, radius);
        }

        if (cell.flags & CELL_SAND) {
          /* lots of friction when we crash into sand */
          velocity[0] *= 0.5;
          velocity[1] *= 0.5;
          velocity[2] *= 0.5;
          if (radius > 0.2)
            for (int i = 0; i < 10; i++)
              if (frandom() < (speed - 1.0) * 0.2) generateSandDebris();
          if (speed > 4.0) playEffect(SFX_SAND_CRASH);
        }
      }

      if (dh < 0.01) { /* ugly fix to stop being caught on edges */
        Cell &cell = map->cell((int)(position[0] + x), (int)(position[1] + y));
        velocity[0] -= 0.005 * x;
        velocity[1] -= 0.005 * y;
        if (cell.flags & CELL_SAND) {
          velocity[0] -= 0.5 * x;
          velocity[1] -= 0.5 * y;
        }
      }
    }

    if (velocity[2] > 2.0) {
      position[2] = position[2] - dh + 0.02;
    } else {
      position[2] -= dh;
      velocity[2] -= dh;
      inTheAir = false;
    }
  }
  return true;
}

Boolean Ball::crash(Real speed) {
  if (modTimeLeft[MOD_GLASS]) speed *= 1.5;

  if (speed > crashTolerance) {
    die(DIE_CRASH);
    return false;
  }

  // Make ball dizzy if crash is above 60% of what we can
  // tolerate. If we already are very dizzy (>4s or permanent)
  // keep it that way.
  if (speed > crashTolerance * (modTimeLeft[MOD_JUMP] ? 0.9 : 0.6))
    if (modTimeLeft[MOD_DIZZY] >= 0.0 &&
        modTimeLeft[MOD_DIZZY] < 3.0 + 1.0 * Settings::settings->difficulty)
      modTimeLeft[MOD_DIZZY] = 3.0 + 1.0 * Settings::settings->difficulty;

  return true;
}

void Ball::generateSandDebris() {
  Coord3d pos, vel;
  Real a = frandom() * 2.0 * M_PI;
  double speed =
      sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2]);
  pos[0] = position[0] + radius * 1.7 * sin(a);
  pos[1] = position[1] + radius * 1.7 * cos(a);
  pos[2] = position[2] - radius * .5;
  vel[0] = velocity[0] + 2.0 * speed * sin(a);  // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[1] = velocity[1] + 2.0 * speed * cos(a);  // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[2] = velocity[2];                         // + speed * 1/2048.0 * ((rand()%2048)-1024);
  Debris *d = new Debris(NULL, pos, vel, 0.5 + 1.0 * frandom());
  d->initialSize = 0.05;
  d->primaryColor[0] = 0.6 + 0.3 * frandom();
  d->primaryColor[1] = 0.5 + 0.4 * frandom();
  d->primaryColor[2] = 0.1 + 0.3 * frandom();
  d->friction = 0.0;
  d->calcRadius();
}
void Ball::generateDebris(GLfloat color[4]) {
  Coord3d pos, vel;
  Real a = frandom() * 2.0 * M_PI;
  double speed =
      sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2]);
  pos[0] = position[0] + radius * 1.5 * sin(a);
  pos[1] = position[1] + radius * 1.5 * cos(a);
  pos[2] = position[2] - radius * .5;
  vel[0] = velocity[0] + 2.0 * speed * sin(a);  // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[1] = velocity[1] + 2.0 * speed * cos(a);  // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[2] = velocity[2];                         // + speed * 1/2048.0 * ((rand()%2048)-1024);
  Debris *d = new Debris(NULL, pos, vel, 0.5 + 1.0 * frandom());
  d->initialSize = 0.05;
  d->primaryColor[0] = color[0];
  d->primaryColor[1] = color[1];
  d->primaryColor[2] = color[2];
  d->friction = 0.0;
  d->calcRadius();
}

void Ball::handleBallCollisions() {
  std::set<Ball *>::iterator iter = balls->begin();
  std::set<Ball *>::iterator end = balls->end();
  Coord3d v;
  double dist, err, speed;
  Ball *ball;
  for (; iter != end; iter++) {
    ball = *iter;
    if (ball == this) continue;
    if (!ball->alive) continue;
    if (ball->no_physics) continue;
    sub(ball->position, position, v);
    dist = length(v);
    if (dist < radius + ball->radius - 1e-3) {
      err = radius + ball->radius - dist;
      position[0] -= err * v[0];
      position[1] -= err * v[1];
      position[2] -= err * v[2];
      normalize(v);
      speed = dotProduct(v, velocity) - dotProduct(v, ball->velocity);
      if (speed < 1e-3) continue;
      double myWeight = radius * radius * radius,
             hisWeight = ball->radius * ball->radius * ball->radius,
             totWeight = myWeight + hisWeight;
      myWeight /= totWeight;
      hisWeight /= totWeight;
      this->crash(speed * hisWeight * 1.5 * (ball->modTimeLeft[MOD_SPIKE] ? 6.0 : 1.0));
      ball->crash(speed * myWeight * 1.5 * (this->modTimeLeft[MOD_SPIKE] ? 6.0 : 1.0));
      for (int i = 0; i < 3; i++) {
        velocity[i] -= speed * v[i] * 3.0 * hisWeight;
        ball->velocity[i] += speed * v[i] * 3.0 * myWeight;
      }
    }
  }
}
void Ball::handleForcefieldCollisions() {
  std::set<ForceField *>::iterator iter = ForceField::forcefields->begin();
  std::set<ForceField *>::iterator end = ForceField::forcefields->end();
  for (; iter != end; iter++) {
    ForceField *ff = *iter;
    if (!ff->is_on) continue;

    /* Boundingbox test, continue if we cannot possibly be withing FF */
    if (ff->direction[0] >= 0.0) {
      if (this->position[0] + radius < ff->position[0]) continue;
      if (this->position[0] - radius > ff->position[0] + ff->direction[0]) continue;
    } else {
      if (this->position[0] + radius < ff->position[0] + ff->direction[0]) continue;
      if (this->position[0] - radius > ff->position[0]) continue;
    }
    if (ff->direction[1] >= 0.0) {
      if (this->position[1] + radius < ff->position[1]) continue;
      if (this->position[1] - radius > ff->position[1] + ff->direction[1]) continue;
    } else {
      if (this->position[1] + radius < ff->position[1] + ff->direction[1]) continue;
      if (this->position[1] - radius > ff->position[1]) continue;
    }

    /*                                */
    /* Detailed collision computation */
    /*                                */
    Coord3d v, ff_normal;
    sub(this->position, ff->position, v);
    v[2] = 0.0;
    ff_normal[0] = ff->direction[1];
    ff_normal[1] = ff->direction[0];
    ff_normal[2] = 0.0;
    normalize(ff_normal);
    double xy_dist = dotProduct(v, ff_normal);
    // if(xy_dist > this->radius) continue;
    assign(ff->direction, ff_normal);
    normalize(ff_normal);
    double ff_where = dotProduct(v, ff_normal);  // how long along ff->direction the hit is
    double ff_len = length(ff->direction);       // the xy-length of the forcefield
    if (ff_where < 0) {
      xy_dist = sqrt(xy_dist * xy_dist + ff_where * ff_where);
      ff_where = 0.0;
    } else if (ff_where > ff_len) {
      double tmp = ff_where - ff_len;
      xy_dist = sqrt(xy_dist * xy_dist + tmp * tmp);
      ff_where = ff_len;
    }
    double ff_base =
        ff->position[2] +
        ff_where / ff_len * ff->direction[2];  // lower z of forcefield at point of impact
    double ff_where_h = position[2];           // z position where we hit the forcefield
    double z_dist = 0.0;
    if (position[2] < ff_base) {
      z_dist = ff_base - position[2];
      ff_where_h = ff_base;
    } else if (position[2] > ff_base + ff->height) {
      z_dist = position[2] - ff_base - ff->height;
      ff_where_h = ff_base + ff->height;
    }
    double dist = sqrt(xy_dist * xy_dist + z_dist * z_dist);
    if (dist < radius) {
      // normal of forcefield
      ff_normal[0] = ff->direction[1];
      ff_normal[1] = ff->direction[0];
      ff_normal[2] = 0.0;
      normalize(ff_normal);

      // the direction we are hitting it from
      v[0] = ff->position[0] + ff_where / ff_len * ff->direction[0] - position[0];
      v[1] = ff->position[1] + ff_where / ff_len * ff->direction[1] - position[1];
      v[2] = ff_where_h - position[2];
      normalize(v);

      // sign for direction
      double sign = dotProduct(ff_normal, v);
      if (sign < 0)
        sign = -1.0;
      else
        sign = 1.0;

      double bounce = ff->bounceFactor * dotProduct(velocity, v);
      if (bounce < 0.0) continue;  // we are already heading *away* from the forcefield

      if (sign == 1.0) {
        if (ff->allow & FF_KILL1) {
          die(DIE_FF);
          return;
        } else if (!(ff->allow & FF_BOUNCE1))
          continue;
      } else if (sign == -1.0) {
        if (ff->allow & FF_KILL2) {
          die(DIE_FF);
          return;
        } else if (!(ff->allow & FF_BOUNCE2))
          continue;
      }

      if (ff_where <= 0.0 || ff_where >= ff_len) {
        // Hitting a forcefield side edge
        velocity[0] -= bounce * v[0];
        velocity[1] -= bounce * v[1];
      } else if (v[2] < 0.0) {
        // Hitting forcefield from above
        if (velocity[2] < 0.0) velocity[2] -= velocity[2] * ff->bounceFactor;
      } else if (v[2] > 0.0) {
        // Hitting forcefield from below (!)
        if (velocity[2] > 0.0) velocity[2] -= velocity[2] * ff->bounceFactor;
      } else {
        // Hitting forcefield from the side
        velocity[0] -= sign * bounce * ff_normal[0];
        velocity[1] -= sign * bounce * ff_normal[1];
      }
    }
  }
}
void Ball::handleEdges() {
  Map *map = Game::current->map;

  if (map->getHeight(position[0] + radius, position[1]) >= position[2] - radius * 0.0) {
    if (velocity[0] > 0) {
      crash(velocity[0] * 0.0);
      velocity[0] *= -bounceFactor;
    }
    velocity[0] -= 0.1;  // position[0] -= 0.1;
  }
  if (map->getHeight(position[0] - radius, position[1]) >= position[2] - radius * 0.0) {
    if (velocity[0] < 0) {
      crash(-velocity[0] * 0.0);
      velocity[0] *= -bounceFactor;
    }
    velocity[0] += 0.1;  // position[0] += 0.1;
  }
  if (map->getHeight(position[0], position[1] + radius) >= position[2] - radius * 0.0) {
    if (velocity[1] > 0) {
      crash(velocity[1] * 0.0);
      velocity[1] *= -bounceFactor;
    }
    velocity[1] -= 0.1;  // position[1] -= 0.1;
  }
  if (map->getHeight(position[0], position[1] - radius) >= position[2] - radius * 0.0) {
    if (velocity[1] < 0) {
      crash(-velocity[1] * 0.0);
      velocity[1] *= -bounceFactor;
    }
    velocity[1] += 0.1;  // position[1] += 0.1;
  }

  const double sqrt2 = 0.7071067;
  if (map->getHeight(position[0] + radius * sqrt2, position[1] + radius * sqrt2) >=
      position[2] - radius * 0.0) {
    if (velocity[0] > 0) {
      crash(velocity[0] * 0.0);
      velocity[0] *= -bounceFactor;
    }
    velocity[0] -= 0.1;  // position[0] -= 0.1;
    if (velocity[1] > 0) {
      crash(velocity[1] * 0.0);
      velocity[1] *= -bounceFactor;
    }
    velocity[1] -= 0.1;  // position[1] -= 0.1;
  }
  if (map->getHeight(position[0] - radius * sqrt2, position[1] + radius * sqrt2) >=
      position[2] - radius * 0.0) {
    if (velocity[0] < 0) {
      crash(-velocity[0] * 0.0);
      velocity[0] *= -bounceFactor;
    }
    velocity[0] += 0.1;  // position[0] += 0.1;
    if (velocity[1] > 0) {
      crash(velocity[1] * 0.0);
      velocity[1] *= -bounceFactor;
    }
    velocity[1] -= 0.1;  // position[1] -= 0.1;
  }
  if (map->getHeight(position[0] + radius * sqrt2, position[1] - radius * sqrt2) >=
      position[2] - radius * 0.0) {
    if (velocity[0] > 0) {
      crash(velocity[0] * 0.0);
      velocity[0] *= -bounceFactor;
    }
    velocity[0] -= 0.1;  // position[0] -= 0.1;
    if (velocity[1] < 0) {
      crash(-velocity[1] * 0.0);
      velocity[1] *= -bounceFactor;
    }
    velocity[1] += 0.1;  // position[1] += 0.1;
  }
  if (map->getHeight(position[0] - radius * sqrt2, position[1] - radius * sqrt2) >=
      position[2] - radius * 0.0) {
    if (velocity[0] < 0) {
      crash(-velocity[0] * 0.0);
      velocity[0] *= -bounceFactor;
    }
    velocity[0] += 0.1;  // position[0] += 0.1;
    if (velocity[1] < 0) {
      crash(-velocity[1] * 0.0);
      velocity[1] *= -bounceFactor;
    }
    velocity[1] += 0.1;  // position[1] += 0.1;
  }
}
void Ball::handlePipes(Real time) {
  inPipe = false;

  std::set<Pipe *>::iterator iter = Pipe::pipes->begin();
  std::set<Pipe *>::iterator end = Pipe::pipes->end();
  for (; iter != end; iter++) {
    Pipe *pipe = *iter;
    Coord3d direction;  // direction of pipe
    Coord3d dirNorm;    // normalized direction
    sub(pipe->to, pipe->from, direction);
    double pipeLength = length(direction);
    sub(pipe->to, pipe->from, dirNorm);
    normalize(dirNorm);
    Coord3d v0;  // pipe enterance -> ball position
    sub(position, pipe->from, v0);
    double l =
        std::max(0.0, std::min(1.0,
                               dotProduct(v0, dirNorm) /
                                   length(direction)));  // where along pipe ball is projected
    Coord3d proj;  // where (as pos) the ball is projected
    for (int i = 0; i < 3; i++) proj[i] = pipe->from[i] + l * direction[i];
    Coord3d v1;  // projection point -> ball position
    sub(position, proj, v1);
    double distance = length(v1);  // how far away from the pipe the ball is
    if (distance > pipe->radius || l == 0.0 || l == 1.0) {
      /* Ball is on the outside */
      // note. If <inPipe> then we are already inside another pipe. No collision!
      if (pipe->radius > 0.1 && pipe->radius < radius && distance < radius) {
        /* Added code to bounce of too small pipes, except for *realy* thin pipes which
         are meant to be lifts */
        Coord3d normal;
        assign(v1, normal);
        normalize(normal);
        double speed = -dotProduct(velocity, normal);
        if (speed > 0)
          for (int i = 0; i < 3; i++) velocity[i] += speed * normal[i] * 1.5;
        double correction = radius - distance;
        for (int i = 0; i < 3; i++) position[i] += correction * normal[i];
      } else if (distance < pipe->radius + radius && l != 0.0 && l != 1.0 && !inPipe) {
        /* Collision from outside */
        if ((pipe->flags & PIPE_SOFT_ENTER) && l < 0.2 / pipeLength) continue;
        if ((pipe->flags & PIPE_SOFT_EXIT) && l > 1.0 - 0.2 / pipeLength) continue;

        Coord3d normal;
        assign(v1, normal);
        normalize(normal);
        double speed = -dotProduct(velocity, normal);
        if (speed > 0)
          for (int i = 0; i < 3; i++) velocity[i] += speed * normal[i] * 1.5;
        double correction = pipe->radius + radius - distance;
        for (int i = 0; i < 3; i++) position[i] += correction * normal[i];
      }
    } else {
      /* Ball is on the inside */
      inPipe = true;
      if (distance > pipe->radius * 0.97 - radius && l != 0.0 && l != 1.0) {
        /* Collision from inside */
        Coord3d normal;
        assign(v1, normal);
        normalize(normal);
        double speed = dotProduct(velocity, normal);
        if (speed > 0)
          for (int i = 0; i < 3; i++) velocity[i] -= speed * normal[i] * 1.5;
        double correction = distance - (pipe->radius * 0.97 - radius);
        for (int i = 0; i < 3; i++) position[i] -= correction * normal[i];
      }

      double zHere = (1.0 - l) * pipe->from[2] + l * pipe->to[2];
      if (distance > pipe->radius * 0.94 - radius && position[2] < zHere) {
        /* Ball is touching lower part of pipe wall */
        inTheAir = false;
        Coord3d normal;
        assign(v1, normal);
        normalize(normal);
        double scale = gravity * time / (-normal[2] + 1e-3);
        velocity[0] -= normal[0] * scale;
        velocity[1] -= normal[1] * scale;
        // velocity[2] += normal[2] * gravity * time;
      }

      /* Wind */
      if (dotProduct(velocity, dirNorm) > 0.0)
        for (int i = 0; i < 3; i++) velocity[i] += pipe->windForward * time * dirNorm[i];
      else
        for (int i = 0; i < 3; i++) velocity[i] += pipe->windBackward * time * dirNorm[i];
    }
  }

  /* Check for pipeConnectors to support ball, only if we are not inside a pipe already */
  if (!inPipe) {
    std::set<PipeConnector *>::iterator iter2 = PipeConnector::connectors->begin();
    std::set<PipeConnector *>::iterator end2 = PipeConnector::connectors->end();
    for (; iter2 != end2; iter2++) {
      PipeConnector *connector = *iter2;
      Coord3d v0;  // ball -> connector
      sub(connector->position, position, v0);
      double dist = length(v0);  // Distance ball center, connector center
      if (dist > connector->radius) {
        /* Ball is outside connector */
        if (dist < connector->radius + radius) {
          /* Collision from outside */
          normalize(v0);
          double speed = dotProduct(velocity, v0);
          if (speed > 0)
            for (int i = 0; i < 3; i++) velocity[i] -= speed * v0[i] * 1.5;
          double correction = connector->radius + radius - dist;
          for (int i = 0; i < 3; i++) position[i] -= correction * v0[i];
        }
      } else {
        /* Ball is inside connector */
        inPipe = true;

        if (dist > connector->radius * 0.97 - radius) {
          /* Collision from inside */
          normalize(v0);
          double speed = dotProduct(velocity, v0);
          if (speed > 0)
            for (int i = 0; i < 3; i++) velocity[i] += speed * v0[i] * 1.5;
          double correction = dist - (connector->radius * 0.97 - radius);
          for (int i = 0; i < 3; i++) position[i] += correction * v0[i];
        }
        if (dist > connector->radius * 0.94 - radius && position[2] < connector->position[2]) {
          /* Ball is touching lower part of connector */
          inTheAir = false;
          normalize(v0);
          double scale = 0.5 * gravity * time / (-v0[2] + 1e-3);
          velocity[0] -= v0[0] * scale;
          velocity[1] -= v0[1] * scale;
        }
      }
    }
  }
}

void Ball::die(int how) { Animated::die(how); }
