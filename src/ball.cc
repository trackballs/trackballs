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

#include "animatedCollection.h"
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

#define MAX_CONTACT_POINTS 24

GLfloat Ball::dizzyTexCoords[4] = {0.f, 0.f, 1.f, 1.f};
extern GLuint hiresSphere;

void Ball::init() {
  loadTexture("dizzy.png");
  dizzyTexCoords[0] = 0.f;
  dizzyTexCoords[1] = 0.f;
  dizzyTexCoords[2] = 1.f;
  dizzyTexCoords[3] = 1.f;
}

Ball::Ball(int role) : Animated(role, 7) {
  sink = 0.0;

  ballResolution = BALL_LORES;
  friction = 1.0;
  rotation[0] = rotation[1] = 0.0;
  rotoacc[0] = rotoacc[1] = 0.0;

  primaryColor = Color(0.8, 0.8, 0.8, 1.);
  secondaryColor = Color(0.1, 0.1, 0.1, 1.);

  gravity = 8.0;
  bounceFactor = 0.8;
  crashTolerance = 7;
  no_physics = false;
  inTheAir = false;
  inPipe = false;
  nextJumpStrength = 0.0;
  acceleration = 4.0;
  radius = 0.49;
  realRadius = 0.49;

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

void Ball::generateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                           bool /*mustUpdate*/) const {
  if (!is_on) return;

  Color color = primaryColor.toOpaque();
  if (0) {
    /* useful debug code */
    if (inTheAir) {
      for (int i = 0; i < 3; i++) { color.v[i] = 65535 - color.v[i]; }
    }
  }
  GLfloat loc[3] = {(GLfloat)position[0], (GLfloat)position[1], (GLfloat)(position[2] - sink)};
  if (modTimeLeft[MOD_GLASS]) {
    double phase = std::min(modTimePhaseIn[MOD_GLASS] / 2.0, 1.0);
    if (modTimeLeft[MOD_GLASS] > 0)
      phase = std::min(modTimeLeft[MOD_GLASS] / 2.0, phase);
    else
      phase = 1.0;
    color = Color::mix(phase, color, Color(0.8, 0.8, 0.8, 0.5));
  } else if (modTimeLeft[MOD_FROZEN]) {
    double phase = std::min(modTimePhaseIn[MOD_FROZEN] / 2.0, 1.0);
    if (modTimeLeft[MOD_FROZEN] > 0)
      phase = std::min(modTimeLeft[MOD_FROZEN] / 2.0, phase);
    else
      phase = 1.0;
    color = Color::mix(phase, color, Color(0.4, 0.4, 0.9, 0.6));
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
    double icoverts[12][3] = {
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

    Color sco = secondaryColor.toOpaque();
    GLfloat flat[3] = {0.f, 0.f, 0.f};
    Matrix3d trot;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++) { trot[i][j] = rotations[j][i]; }

    GLfloat data[20 * 4][8];
    ushort idxs[20 * 3][3];
    char *pos = (char *)data;
    for (int i = 0; i < 20; i++) {
      Coord3d centroid;
      for (int j = 0; j < 3; j++) { centroid = centroid + Coord3d(icoverts[icofaces[i][j]]); }
      centroid = centroid / 3.0;

      Coord3d spike = centroid;
      spike = spike * 0.87 * scale;
      Coord3d sub = useMatrix(trot, spike);
      pos += packObjectVertex(pos, loc[0] + sub[0], loc[1] + sub[1], loc[2] + sub[2], 0., 0.,
                              sco, flat);
      for (int j = 2; j >= 0; j--) {
        Coord3d edge(icoverts[icofaces[i][j]]);
        edge = 0.3 * scale * (0.1 * centroid + 0.9 * edge);
        sub = useMatrix(trot, edge);
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

      Coord3d v = velocity;
      v[2] = 0.0;
      if (length(v) > 0.8) {
        v = v / length(v);
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
    Color stripeA(1., 1., 1., 1.);
    Color stripeB(1., 0.2, 0.2, 1.);

    GLfloat data[60][8];
    ushort idxs[40][3];
    char *pos = (char *)data;
    for (int i = 0; i < 10; i++) {
      GLfloat a0 = (i / 10.0) * 2. * M_PI, a1 = ((i + 1) / 10.0) * 2. * M_PI;
      const Color &ringcolor = i % 2 ? stripeA : stripeB;

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
    Color color = primaryColor;
    color.v[3] = 32768;
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

    Color color(1., 1., 1., 0.5);
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
    Color color(1., 1., 1., 0.5);
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
}

void Ball::drawBuffers1(const GLuint *idxbufs, const GLuint *databufs) const {
  if (!is_on) return;
  if (dontReflectSelf) return;

  Color specular = specularColor.toOpaque();
  double shininess = 20.0;

  // Handle primary ball..
  if (modTimeLeft[MOD_GLASS]) {
    double phase = std::min(modTimePhaseIn[MOD_GLASS] / 2.0, 1.0);
    if (modTimeLeft[MOD_GLASS] > 0)
      phase = std::min(modTimeLeft[MOD_GLASS] / 2.0, phase);
    else
      phase = 1.0;
    specular = Color::mix(phase, specular, Color(1., 1., 1., 1.));
    shininess = 50.0 * phase + shininess * (1.0 - phase);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
  } else if (modTimeLeft[MOD_FROZEN]) {
    double phase = std::min(modTimePhaseIn[MOD_FROZEN] / 2.0, 1.0);
    if (modTimeLeft[MOD_FROZEN] > 0)
      phase = std::min(modTimeLeft[MOD_FROZEN] / 2.0, phase);
    else
      phase = 1.0;
    double blend = phase;
    specular = Color::mix(phase, specular, Color(0.8, 0.8, 1., 1.));
    shininess = 50.0 * phase + shininess * (1.0 - phase);
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
    if (activeView.calculating_shadows) {
      setActiveProgramAndUniforms(shaderObjectShadow);
    } else {
      setActiveProgramAndUniforms(shaderObject);
      glUniformC(glGetUniformLocation(shaderObject, "specular"), specular);
      glUniform1f(glGetUniformLocation(shaderObject, "shininess"), shininess);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);

    if (modTimeLeft[MOD_EXTRA_LIFE]) {
      glBindTexture(GL_TEXTURE_2D, textures[loadTexture("track.png")]);
    } else if (texture == 0) {
      glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);
    } else {
      glBindTexture(GL_TEXTURE_2D, textures[texture]);
    }
    configureObjectAttributes();

    glDrawElements(GL_TRIANGLES, 3 * ntries, GL_UNSIGNED_SHORT, (void *)0);

    if (Settings::settings->doReflections && reflectivity > 0.0 && environmentTexture &&
        !activeView.calculating_shadows) {
      GLfloat c[4];
      if (metallic) {
        c[0] = primaryColor.f0();
        c[1] = primaryColor.f1();
        c[2] = primaryColor.f2();
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
    if (activeView.calculating_shadows) {
      setActiveProgramAndUniforms(shaderObjectShadow);
    } else {
      setActiveProgramAndUniforms(shaderObject);
      glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.1f, 0.1f, 0.1f, 1.f);
      glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 10.f);
    }
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

    if (activeView.calculating_shadows) {
      setActiveProgramAndUniforms(shaderObjectShadow);
    } else {
      setActiveProgramAndUniforms(shaderObject);
      glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0.1f, 0.1f, 0.1f, 1.f);
      glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 10.f);
    }
    glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[3]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[3]);
    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 40 * 3, GL_UNSIGNED_SHORT, (void *)0);
  }
}

void Ball::drawBuffers2(const GLuint *idxbufs, const GLuint *databufs) const {
  if (!is_on) return;
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

  Matrix4d mvp;
  matrixMult(MainMode::mainMode->cameraModelView, MainMode::mainMode->cameraProjection, mvp);
  int vis =
      testBboxClip(position[0] + boundingBox[0][0], position[0] + boundingBox[1][0],
                   position[1] + boundingBox[0][1], position[1] + boundingBox[1][1],
                   position[2] + boundingBox[0][2], position[2] + boundingBox[1][2], mvp);
  if (!vis) return;

  if (environmentTexture == 0) glGenTextures(1, &environmentTexture);
  glBindTexture(GL_TEXTURE_2D, environmentTexture);

  dontReflectSelf = 1;
  MainMode::mainMode->renderEnvironmentTexture(environmentTexture, position);
  dontReflectSelf = 0;
}

void Ball::tick(Real time) {
  if (!is_on) return;

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
  if (modTimeLeft[MOD_NITRO]) generateNitroDebris(time);

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

bool Ball::physics(Real time) {
  if (!Game::current) return true;
  Map *map = Game::current->map;

  if (modTimeLeft[MOD_DIZZY]) {
    /* We cast the address of the ball into an integer in order to get a unique random seed for
     * every ball. */
    rotation[0] += time * 7.0 * (frand((long)Game::current->gameTime + (long)this) - 0.5);
    rotation[1] += time * 7.0 * (frand(47 + (long)Game::current->gameTime + (long)this) - 0.5);
  }

  /* Ball self-drive */
  double effective_acceleration = acceleration;
  if (modTimeLeft[MOD_SPEED]) effective_acceleration *= 1.5;
  if (modTimeLeft[MOD_DIZZY]) effective_acceleration /= 2.0;
  if (modTimeLeft[MOD_NITRO]) {
    /* Mod-nitro induces maximal acceleration in a direction of interest */
    effective_acceleration *= 3.0;
    double dlen = std::sqrt(rotoacc[0] * rotoacc[0] + rotoacc[1] * rotoacc[1]);
    if (dlen <= 0.) {
      double rlen = std::sqrt(rotation[0] * rotation[0] + rotation[1] * rotation[1]);
      if (rlen <= 0.) {
        double rnd = M_PI2 * frand();
        rotoacc[0] = std::sin(rnd);
        rotoacc[1] = std::cos(rnd);
      } else {
        rotoacc[0] = rotation[0] / rlen;
        rotoacc[1] = rotation[1] / rlen;
      }
    } else {
      rotoacc[0] = rotoacc[0] / dlen;
      rotoacc[1] = rotoacc[1] / dlen;
    }
  }
  if (modTimeLeft[MOD_FROZEN]) effective_acceleration = 0.;
  rotation[0] += effective_acceleration * time * rotoacc[0];
  rotation[1] += effective_acceleration * time * rotoacc[1];

  rotateX(-rotation[1] * time * 2.0 * M_PI * 0.3 * 0.3 / radius, rotations);
  rotateY(-rotation[0] * time * 2.0 * M_PI * 0.3 * 0.3 / radius, rotations);

  /* Interact with terrain */
  Cell *cells[MAX_CONTACT_POINTS];
  Coord3d hitpts[MAX_CONTACT_POINTS];
  Coord3d normals[MAX_CONTACT_POINTS];
  ICoord2d cellco[MAX_CONTACT_POINTS];
  double dhs[MAX_CONTACT_POINTS];
  double min_height_above_ground = 1.0;
  int nhits =
      locateContactPoints(map, cells, hitpts, normals, cellco, dhs, &min_height_above_ground);
  Coord3d wall_normals[MAX_CONTACT_POINTS];
  int nwalls = locateWallBounces(map, wall_normals);

  const double wall_thresh = 0.35;

  double max_dx = 0., min_dx = 0., max_dy = 0., min_dy = 0.;
  for (int i = 0; i < nhits; i++) {
    if (normals[i][2] < wall_thresh) {
      if (dhs[i] < 0.) { /* Steep wall, bounce the normal */
        if (nwalls < MAX_CONTACT_POINTS) {
          wall_normals[nwalls] = normals[i];
          nwalls++;
        }
      }
    }
    /* Greatest axis-aligned slopes contribute */
    double dx = normals[i][0] / std::max(normals[i][2], wall_thresh);
    double dy = normals[i][1] / std::max(normals[i][2], wall_thresh);
    max_dx = std::max(dx, max_dx);
    min_dx = std::min(dx, min_dx);
    max_dy = std::max(dy, max_dy);
    min_dy = std::min(dy, min_dy);
  }
  /* All effects of gravity */
  if (inTheAir)
    velocity[2] = velocity[2] - gravity * time;
  else if (!inPipe) {
    velocity[0] += gravity * time * (max_dx + min_dx);
    velocity[1] += gravity * time * (max_dy + min_dy);
  }

  /* Compute terrain interaction fractions */
  int sand_count = 0, acid_count = 0, ice_count = 0, track_count = 0;
  for (int i = 0; i < nhits; i++) {
    if (cells[i]->flags & CELL_SAND) sand_count++;
    if (cells[i]->flags & CELL_ACID) acid_count++;
    if (cells[i]->flags & CELL_ICE) ice_count++;
    if (cells[i]->flags & CELL_TRACK) track_count++;
  }
  double inh = nhits > 0 ? 1. / nhits : 0.;
  double sand_frac = sand_count * inh;
  double acid_frac = acid_count * inh;
  double ice_frac = ice_count * inh;
  double track_frac = track_count * inh;

  /* Execute possible planned jump */
  if (nextJumpStrength > 0.) {
    if (!inTheAir && acid_frac < 0.99) {
      velocity[2] += nextJumpStrength * (1.0 - acid_frac);
      position[2] += 0.10 * radius;
      /* correct contact height estimates; see handleGround */
      for (int i = 0; i < nhits; i++) { dhs[i] += 0.10 * radius; }
      inTheAir = true;
    }
  }
  nextJumpStrength = 0.;

  /* Sand - generate debris */
  if (!inTheAir && radius > 0.2 && !inPipe) {
    double speed2 =
        velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
    if (sand_frac > 0.) {
      if (frandom() < (speed2 - 0.3) * 0.08) {
        /* lots of friction when we crash into sand */
        double slowdown = std::pow(0.9, sand_frac);
        velocity[0] *= slowdown;
        velocity[1] *= slowdown;
        velocity[2] *= slowdown;
        generateSandDebris();
      }
    } else if (Settings::settings->difficulty > 0 && modTimeLeft[MOD_SPIKE]) {
      if (frandom() < (speed2 - 0.3) * 0.0007 * Settings::settings->difficulty) {
        velocity[0] *= 0.9;
        velocity[1] *= 0.9;
        velocity[2] *= 0.9;
        Cell &c = map->cell((int)position[0], (int)position[1]);
        generateDebris(c.colors[Cell::CENTER]);
      }
    }
  }

  /* All effects of water */
  Real mapHeight = map->getHeight(position[0], position[1]);
  double waterHeight = map->getWaterHeight(position[0], position[1]);
  {
    /* Floating */
    if (modTimeLeft[MOD_FLOAT] && !inPipe && waterHeight > position[2] &&
        waterHeight > mapHeight + radius + 0.025) {
      velocity[2] += gravity * time * 1.5;
      velocity[2] *= 0.99;
      // avoid sticking to the ground
      double delta = position[2] - radius - mapHeight;
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
        Color waterColor(0.4, 0.4, 0.8, 0.5);
        Coord3d center(position[0], position[1], waterHeight);
        Game::current->add(
            new Splash(center, velocity, waterColor, 30 * radius / 0.3,
                       radius));  // speed*radius*(depth<0.5?depth:1.0-depth)*2.0,radius);
      }
      // splashes caused by rotation. eg "swimming"
      speed = rotation[0] * rotation[0] + rotation[1] * rotation[1];
      if (frandom() < speed * 0.001) {
        Color waterColor(0.4, 0.4, 0.8, 0.5);
        Coord3d center(position[0], position[1], waterHeight);
        Coord3d vel;
        vel[0] = -rotation[0] * radius;  // 0.3;
        vel[1] = -rotation[1] * radius;  // 0.3;
        vel[2] = 0.2;
        rotation[0] *= 0.9;
        rotation[1] *= 0.9;
        velocity[0] += 0.01 * rotation[0];
        velocity[1] += 0.01 * rotation[1];
        Game::current->add(new Splash(center, vel, waterColor, (int)speed * 0.5, radius));
      }
      /* cell velocity field extends to the water within the cell */
      Cell &c = map->cell((int)position[0], (int)position[1]);
      double fric = 0.004 * std::min(1.0, depth / (2. * radius));  // an extra water friction
      velocity[0] = velocity[0] * (1. - fric) + c.velocity[0] * fric;
      velocity[1] = velocity[1] * (1. - fric) + c.velocity[1] * fric;
      fric = 0.008;
      velocity[2] = velocity[2] * (1. - fric) + c.velocity[1] * fric;
    }
  }

  /* Sinking into floor material */
  if (acid_frac > 0. && !inPipe && !inTheAir) {
    sink += 0.8 * time * (acid_frac + sand_frac);
    double speed2 =
        velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
    if (frandom() < (speed2 - 0.2) * 0.05) {
      Color acidColor(0.1, 0.5, 0.1, 0.5);
      Coord3d center(position[0], position[1], mapHeight);
      Game::current->add(new Splash(center, velocity, acidColor, speed2 * radius, radius));
    }
    if (modTimeLeft[MOD_GLASS]) sink = std::min(sink, 0.3);
    if (sink > radius * 2.0) {
      die(DIE_ACID);
      return false;
    }
  } else if (sand_frac > 0. && !inPipe && !inTheAir) {
    sink += 0.8 * time * (acid_frac + sand_frac);
    if (sink > 0.5 * sand_frac * radius) sink = 0.5 * sand_frac * radius;
  } else
    sink = std::max(0.0, sink - 2.0 * time);

  /*                                      */
  /* Ground "grip" - Also works in pipes! */
  /*                                      */
  {
    const double v_base = 0.08, r_base = 0.10;

    double v_fric = v_base;
    double r_fric = r_base;

    v_fric += (0.008 - v_base) * acid_frac;
    r_fric += (0.010 - r_base) * acid_frac;

    if (modTimeLeft[MOD_SPIKE]) {
      v_fric += (0.008 - v_base) * ice_frac;
      r_fric += (0.010 - r_base) * ice_frac;
    } else {
      v_fric += (0.0008 - v_base) * ice_frac;
      r_fric += (0.0010 - r_base) * ice_frac;
    }

    if (inTheAir) {
      if (waterHeight > position[2] - radius) {
        v_fric = 0.0005;
        r_fric = 0.0005;
      } else
        v_fric = r_fric = 0.0;
    }

    if (inPipe) {
      v_fric = 0.08;
      r_fric = 0.10;
    }

    if (track_frac > 0.) {
      double trackv[2] = {0., 0.};
      for (int i = 0; i < nhits; i++) {
        if (cells[i]->flags & CELL_TRACK) {
          trackv[0] += cells[i]->velocity[0] / (double)nhits;
          trackv[1] += cells[i]->velocity[1] / (double)nhits;
        }
      }
      velocity[0] = velocity[0] * (1.0 - v_fric) + (rotation[0] + trackv[0]) * v_fric;
      velocity[1] = velocity[1] * (1.0 - v_fric) + (rotation[1] + trackv[1]) * v_fric;
      rotation[0] = rotation[0] * (1.0 - r_fric) + (velocity[0] - trackv[0]) * r_fric;
      rotation[1] = rotation[1] * (1.0 - r_fric) + (velocity[1] - trackv[1]) * r_fric;
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
      double slowdown = 0.;
      slowdown += std::log(2.0) * acid_frac;
      slowdown += std::log(0.1) * ice_frac;
      // sand+track is 20x boost, not 10x
      slowdown += std::log(10.0) * sand_frac;
      slowdown += std::log(4.0) * track_frac;
      slowdown += std::log(0.5) * sand_frac * track_frac;

      effective_friction *= std::exp(slowdown);
    }
    if (modTimeLeft[MOD_SPIKE]) effective_friction *= 1.5;
    if (modTimeLeft[MOD_SPEED]) effective_friction *= 0.5;
  }
  double cvel[3] = {0., 0., 0.};
  for (int i = 0; i < nhits; i++) {
    cvel[0] += normals[i][2] * cells[i]->velocity[0] / (double)nhits;
    cvel[1] += normals[i][2] * cells[i]->velocity[1] / (double)nhits;
    cvel[2] +=
        -1 * (normals[i][0] * cells[i]->velocity[0] + normals[i][1] * cells[i]->velocity[1]) /
        (double)nhits;
  }
  for (int i = 0; i < 3; i++)
    velocity[i] = velocity[i] - (velocity[i] - cvel[i]) * effective_friction;
  if (inTheAir && velocity[2] > 5.0) velocity[2] *= 0.995;

  for (int i = 0; i < 3; i++) position[i] += time * velocity[i];

  if (!inPipe) {
    /* automatically force ball to map height if it is far enough down */
    if (mapHeight > position[2] + radius) { position[2] = mapHeight + 0.75 * radius; }

    if (!handleGround(map, cells, hitpts, normals, cellco, dhs, nhits, time)) return false;

    if (!handleWalls(wall_normals, nwalls)) return false;
  }

  /* Collisions with other balls */
  handleBallCollisions();

  /* Collisions with forcefields */
  handleForcefieldCollisions();

  if (inPipe || nhits <= 0) inTheAir = true;

  /* Pipes */
  handlePipes(time);

  return true;
}
static int closestPointOnTriangle(const Coord3d &tricor0, const Coord3d &tricor1,
                                  const Coord3d &tricor2, const Coord3d &point,
                                  Coord3d *closest, Coord3d *normal) {
  Coord3d dv0 = tricor2 - tricor0;
  Coord3d dv1 = tricor2 - tricor1;
  Coord3d baseoff = point - tricor2;
  Coord3d nor = crossProduct(dv0, dv1);
  nor = nor / length(nor);
  double dist = dotProduct(baseoff, nor);
  if (dist > 0) {
    *normal = nor;
  } else {
    *normal = -nor;
  }
  Coord3d nearoff = baseoff - dist * nor;
  double s = -dotProduct(nearoff, dv0);
  double t = -dotProduct(nearoff, dv1);
  Coord3d m1 = dv0 - dv1;
  Coord3d nearoffm1 = nearoff + dv1;
  double r = -dotProduct(nearoffm1, m1);
  double mm = dotProduct(m1, m1);
  double uu = dotProduct(dv0, dv0);
  double vv = dotProduct(dv1, dv1);
  double uv = dotProduct(dv0, dv1);
  double idet = 1. / (uu * vv - uv * uv);
  double a = idet * (vv * s - uv * t);
  double b = idet * (-uv * s + uu * t);
  double c = 1 - a - b;
  if (0. <= a && a <= 1. && 0. <= b && b <= 1. && 0. <= c && c <= 1.) {
    *closest = tricor0 * a + tricor1 * b + tricor2 * c;
    return 0;
  } else if (0. <= s && s <= uu && b <= 0.) {
    double q = s / uu;
    *closest = tricor2 * (1. - q) + tricor0 * q;
    return 1;
  } else if (0. <= t && t <= vv && a <= 0.) {
    double q = t / vv;
    *closest = tricor2 * (1. - q) + tricor1 * q;
    return 2;
  } else if (0. <= r && r <= mm && c <= 0.) {
    double q = r / mm;
    *closest = tricor0 * q + tricor1 * (1. - q);
    return 3;
  } else if (s <= 0. && t <= 0.) {
    *closest = tricor2;
    return 4;
  } else if (t >= 0. && r <= 0.) {
    *closest = tricor1;
    return 5;
  } else if (s >= 0 && r >= mm) {
    *closest = tricor0;
    return 6;
  }
  warning("cPoT impossible case happened");
  return -1;
}
int Ball::locateContactPoints(class Map *map, class Cell **cells, Coord3d *hitpts,
                              Coord3d *normals, ICoord2d *cellco, double *dhs,
                              double *min_height_above_ground) {
  int nhits = 0;
  /* Construct a list of triangular facets the ball could interact with,
   * and locate their closest interaction points. */
  int xmin = std::floor(position[0] - radius), xmax = std::floor(position[0] + radius);
  int ymin = std::floor(position[1] - radius), ymax = std::floor(position[1] + radius);
  for (int x = xmin; x <= xmax; x++) {
    for (int y = ymin; y <= ymax; y++) {
      Cell &c = map->cell(x, y);

      double ch = c.heights[Cell::CENTER];
      Coord3d centerpoint(x + 0.5, y + 0.5, ch);
      Coord3d offset = position - centerpoint;

      Coord3d local_corners[4] = {
          Coord3d(-0.5, -0.5, c.heights[Cell::SOUTH + Cell::WEST] - ch),
          Coord3d(0.5, -0.5, c.heights[Cell::SOUTH + Cell::EAST] - ch),
          Coord3d(0.5, 0.5, c.heights[Cell::NORTH + Cell::EAST] - ch),
          Coord3d(-0.5, 0.5, c.heights[Cell::NORTH + Cell::WEST] - ch),
      };

      for (int i0 = 0; i0 < 4; i0++) {
        int i1 = (i0 + 1) % 4;

        Coord3d closest, normal;
        int ret = closestPointOnTriangle(local_corners[i1], local_corners[i0], Coord3d(),
                                         offset, &closest, &normal);
        closest = closest + centerpoint;
        if (ret < 0) continue;

        /* Only top sides of facets matter; and no getting pulled up */
        if (normal[2] < 0 || position[2] < closest[2]) continue;

        /* Ensure that ball is over the closest point */
        double dx = (closest[0] - position[0]), dy = (closest[1] - position[1]);
        double rad2 = dx * dx + dy * dy;
        if (rad2 > radius * radius) continue;

        /* Ball must be closer than 0.07*radius to closest point */
        Real dh = position[2] - std::sqrt(std::max(radius * radius - rad2, 0.)) - closest[2];
        *min_height_above_ground = std::min(*min_height_above_ground, dh);
        if (dh >= 0.07 * radius) continue;

        if (nhits >= MAX_CONTACT_POINTS) continue;
        cells[nhits] = &c;
        hitpts[nhits] = closest;
        normals[nhits] = normal;
        cellco[nhits][0] = x;
        cellco[nhits][1] = y;
        dhs[nhits] = dh;
        nhits++;
      }
    }
  }
  return nhits;
}
bool Ball::handleGround(class Map *map, Cell **cells, Coord3d *hitpts, Coord3d *normals,
                        ICoord2d *cellco, double *dhs, int nhits, Real time) {
  /* If there are no points of contact, done */
  if (nhits == 0) return true;

  /* For each point of contact, compute interaction */
  double weight = 1. / nhits;

  if (inTheAir) {
    /* General contact friction */
    double v_fric = 0.2;
    double r_fric = 0.4;
    velocity[0] = velocity[0] * (1.0 - v_fric) + rotation[0] * v_fric;
    velocity[1] = velocity[1] * (1.0 - v_fric) + rotation[1] * v_fric;
    rotation[0] = rotation[0] * (1.0 - r_fric) + velocity[0] * r_fric;
    rotation[1] = rotation[1] * (1.0 - r_fric) + velocity[1] * r_fric;

    /* Crash handling */
    int trampcell[MAX_CONTACT_POINTS][2];
    double trampspeed[MAX_CONTACT_POINTS];
    int nspeeds[MAX_CONTACT_POINTS];
    int ntramp = 0;

    int nacidsplash = 0;
    int acidSpeed = 0.;

    int nsandcells = 0;
    double sandSpeed = 0;

    Coord3d velbounce;
    double max_crash_speed = 0.;
    for (int i = 0; i < nhits; i++) {
      Real speed = -dotProduct(velocity, normals[i]);
      if (speed > 0) {
        Cell &cell = *cells[i];

        double crash_speed = speed;
        if (cell.flags & (CELL_TRAMPOLINE | CELL_SAND)) crash_speed *= 0.4;
        if (modTimeLeft[MOD_JUMP]) crash_speed *= 0.8;
        max_crash_speed = std::max(crash_speed, max_crash_speed);

        double effective_bounceFactor = bounceFactor;
        if (cell.flags & CELL_ACID) effective_bounceFactor = 0.0;
        if (cell.flags & CELL_SAND) effective_bounceFactor = 0.1;
        if (cell.flags & CELL_TRAMPOLINE) {
          effective_bounceFactor += 0.6;

          /* Boost existing trampoline cell if possible */
          int ptramp = ntramp;
          for (int j = 0; j < ntramp; j++) {
            if (trampcell[j][0] == cellco[i][0] && trampcell[j][1] == cellco[i][1]) {
              ptramp = j;
              break;
            }
          }
          if (ptramp == ntramp) {
            ntramp++;
            trampspeed[ptramp] = 0.;
            nspeeds[ptramp] = 0;
          }
          trampcell[ptramp][0] = cellco[i][0];
          trampcell[ptramp][1] = cellco[i][1];
          trampspeed[ptramp] += speed;
          nspeeds[ptramp]++;
        }
        speed *= 1.0 + effective_bounceFactor;
        velbounce = velbounce + weight * normals[i] * speed;

        if (cell.flags & CELL_ACID) {
          nacidsplash++;
          acidSpeed += speed;
        }
        if (cell.flags & CELL_SAND) {
          nsandcells++;
          sandSpeed += speed;
        }
      }
    }

    if (!crash(max_crash_speed)) return false;

    /* Acid splash */
    if (nacidsplash) {
      Color acidColor(0.1, 0.5, 0.1, 0.5);
      Coord3d center(position[0], position[1], map->getHeight(position[0], position[1]));
      Game::current->add(new Splash(center, velocity, acidColor,
                                    (acidSpeed / nacidsplash) * radius * 20.0, radius));
    }

    /* Apply bounce */
    velocity = velocity + velbounce;

    /* Sand handling */
    if (nsandcells > 0) {
      /* lots of friction when we crash into sand */
      double slowdown = std::pow(0.5, nsandcells / (double)nhits);
      velocity[0] *= slowdown;
      velocity[1] *= slowdown;
      velocity[2] *= slowdown;
      sandSpeed /= nsandcells;
      if (radius > 0.2)
        for (int i = 0; i < 10; i++)
          if (frandom() < (sandSpeed - 1.0) * 0.2) generateSandDebris();
      if (sandSpeed > 4.0) playEffect(SFX_SAND_CRASH);
    }

    /* Activate trampolines */
    for (int i = 0; i < ntramp; i++) {
      Real speed = trampspeed[i] / nspeeds[i];
      Cell &cell = map->cell(trampcell[i][0], trampcell[i][1]);
      Real dh = 1.0 * speed * radius * radius * radius;
      for (int j = 0; j < 5; j++) cell.heights[j] -= dh;
      if (cell.sunken <= 0.0)
        Game::current->add(new Trampoline(trampcell[i][0], trampcell[i][1]));
      cell.sunken += dh;
    }
  }

  /* Surface attachment & kill cell*/
  {
    double meandh = 0.;
    int ndh = 0;
    Coord3d meanNormal;
    for (int i = 0; i < nhits; i++) {
      Real dh = dhs[i];
      if (dh > 0.07 * radius) { continue; }
      /* contact with kill cells is death */
      if (dh < 0.001 && cells[i]->flags & CELL_KILL) {
        die(DIE_OTHER);
        return false;
      }
      meandh += dh;
      meanNormal = meanNormal + normals[i];
      ndh++;
    }
    double dhw = ndh > 0 ? 1. / ndh : 0.;
    meandh *= dhw;
    meanNormal = meanNormal * dhw;
    /* can't be pulled down faster than gravity + slope */
    double slopedv = -1 * (meanNormal[0] * velocity[0] + meanNormal[1] * velocity[1]);

    double vmin = velocity[2] - time * gravity;
    double pmin = position[2] + vmin * time;

    if (velocity[2] - slopedv > 2.0) {
      /* Escape */
      position[2] -= meandh;
      position[2] += 0.07 * radius;
      position[2] = std::max(position[2], pmin);
    } else {
      /* Last bounce not strong enough to pull away; stick to the surface */
      position[2] -= meandh;
      if (inTheAir) {
        position[2] = std::max(position[2], pmin);
        velocity[2] = slopedv; /* adopt surface vel */
      } else {
        double k = 1.0;
        velocity[2] -= k * meandh;
        position[2] = std::max(position[2], pmin);
        velocity[2] = std::min(velocity[2], slopedv);
        velocity[2] = std::max(velocity[2], vmin);
      }
      inTheAir = false;
    }
  }

  return true;
}
static double sign(double v) {
  if (v > 0.) return 1.;
  if (v < 0.) return -1.;
  return 0.;
}
int Ball::locateWallBounces(class Map *map, Coord3d *wall_normals) {
  /* individual bounce back for each wall segment intersecting the rim of
   * the ball */
  int xmin = std::floor(position[0] - radius), xmax = std::floor(position[0] + radius);
  int ymin = std::floor(position[1] - radius), ymax = std::floor(position[1] + radius);
  int nwalls = 0;
  double Z = position[2];
  for (int x = xmin; x <= xmax; x++) {
    for (int y = ymin; y <= ymax; y++) {
      Cell &c = map->cell(x, y);
      /* handle X walls */
      for (int ly = 0; ly < 2; ly++) {
        double yp = (y + ly);
        double h1 = c.heights[(ly ? Cell::NORTH : Cell::SOUTH) + Cell::WEST];
        double h2 = c.heights[(ly ? Cell::NORTH : Cell::SOUTH) + Cell::EAST];
        double s = 0., t = 1.;
        bool linethere = true;
        if (h1 > Z && h2 > Z) {
        } else if (h1 > Z) {
          t = (Z - h1) / (h2 - h1);
        } else if (h2 > Z) {
          s = (Z - h1) / (h2 - h1);
        } else {
          linethere = false;
        }
        double as = 0., at = 1.;
        Cell &op = map->cell(x, y + (ly * 2 - 1));
        double ah1 = op.heights[(ly ? Cell::SOUTH : Cell::NORTH) + Cell::WEST];
        double ah2 = op.heights[(ly ? Cell::SOUTH : Cell::NORTH) + Cell::EAST];
        if (ah1 < Z && ah2 < Z) {
        } else if (ah1 < Z) {
          at = (Z - ah1) / (ah2 - ah1);
        } else if (ah2 < Z) {
          as = (Z - ah1) / (ah2 - ah1);
        } else {
          linethere = false;
        }
        s = std::max(s, as);
        t = std::min(t, at);
        if (s > t) linethere = false;

        if (position[1] > yp && ly == 0) linethere = false;
        if (position[1] < yp && ly == 1) linethere = false;
        if (position[1] == yp) linethere = false;

        if (linethere) {
          int hit = 0;
          if (x + s <= position[0] && position[0] <= x + t) {
            if (position[1] > yp && position[1] - radius < yp) hit = 1;
            if (position[1] < yp && position[1] + radius > yp) hit = 2;
          }
          double dsx = (x + s - position[0]), dsy = (yp - position[1]);
          double dtx = (x + t - position[0]), dty = (yp - position[1]);
          if (dsx * dsx + dsy * dsy < radius * radius) hit = 3;
          if (dtx * dtx + dty * dty < radius * radius) hit = 4;
          if (hit) {
            wall_normals[nwalls][0] = 0.;
            wall_normals[nwalls][1] = sign(position[1] - yp);
            wall_normals[nwalls][2] = 0.;
            nwalls++;
          }
        }
      }
      /* handle Y walls */
      for (int lx = 0; lx < 2; lx++) {
        double xp = (x + lx);
        double h1 = c.heights[(lx ? Cell::EAST : Cell::WEST) + Cell::SOUTH];
        double h2 = c.heights[(lx ? Cell::EAST : Cell::WEST) + Cell::NORTH];
        double s = 0., t = 1.;
        bool linethere = true;
        if (h1 > Z && h2 > Z) {
        } else if (h1 > Z) {
          t = (Z - h1) / (h2 - h1);
        } else if (h2 > Z) {
          s = (Z - h1) / (h2 - h1);
        } else {
          linethere = false;
        }
        double as = 0., at = 1.;
        Cell &op = map->cell(x + (lx * 2 - 1), y);
        double ah1 = op.heights[(lx ? Cell::WEST : Cell::EAST) + Cell::SOUTH];
        double ah2 = op.heights[(lx ? Cell::WEST : Cell::EAST) + Cell::NORTH];
        if (ah1 < Z && ah2 < Z) {
        } else if (ah1 < Z) {
          at = (Z - ah1) / (ah2 - ah1);
        } else if (ah2 < Z) {
          as = (Z - ah1) / (ah2 - ah1);
        } else {
          linethere = false;
        }
        s = std::max(s, as);
        t = std::min(t, at);
        if (s > t) linethere = false;

        if (position[0] > xp && lx == 0) linethere = false;
        if (position[0] < xp && lx == 1) linethere = false;
        if (position[0] == xp) linethere = false;

        if (linethere) {
          int hit = 0;
          if (y + s <= position[1] && position[1] <= y + t) {
            if (position[0] > xp && position[0] - radius < xp) hit = 1;
            if (position[0] < xp && position[0] + radius > xp) hit = 2;
          }
          double dsx = (xp - position[0]), dsy = (y + s - position[1]);
          double dtx = (xp - position[0]), dty = (y + t - position[1]);
          if (dsx * dsx + dsy * dsy < radius * radius) hit = 3;
          if (dtx * dtx + dty * dty < radius * radius) hit = 4;
          if (hit) {
            wall_normals[nwalls][0] = sign(position[0] - xp);
            wall_normals[nwalls][1] = 0.;
            wall_normals[nwalls][2] = 0.;
            nwalls++;
          }
        }
      }
    }
  }
  return nwalls;
}
bool Ball::handleWalls(Coord3d *wall_normals, int nwalls) {
  Coord3d bounce_normal;
  Coord3d mean_normal;
  int nbounce = 0;
  for (int i = 0; i < nwalls; i++) {
    double crash_speed = -dotProduct(velocity, wall_normals[i]);
    if (modTimeLeft[MOD_SPEED]) crash_speed *= 0.5;
    if (modTimeLeft[MOD_NITRO]) crash_speed *= 0.5;
    if (modTimeLeft[MOD_JUMP]) crash_speed *= 0.8;
    crash_speed *= 0.5;

    if (crash_speed > 0) {
      if (!crash(crash_speed)) return false;
      nbounce++;
      bounce_normal = bounce_normal + wall_normals[i];
    }
    mean_normal = mean_normal + wall_normals[i];
  }

  if (nbounce) {
    for (int k = 0; k < 3; k++)
      velocity[k] -= (1 + bounceFactor) * velocity[k] * std::abs(bounce_normal[k]) / nbounce;
  }

  if (nwalls) velocity = velocity + 0.1 * mean_normal / nwalls;
  return true;
}

bool Ball::crash(Real speed) {
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

void Ball::queueJump(Real strength) {
  if (strength <= 0.) return;
  nextJumpStrength = strength;
}
void Ball::drive(Real x, Real y) {
  rotoacc[0] = x;
  rotoacc[1] = y;
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
  Game::current->add(d);
  d->initialSize = 0.05;
  d->primaryColor =
      Color(0.6 + 0.3 * frandom(), 0.5 + 0.4 * frandom(), 0.1 + 0.3 * frandom(), 1.f);
  d->friction = 0.0;
  d->calcRadius();
}
void Ball::generateNitroDebris(Real time) {
  nitroDebrisCount += time;
  while (nitroDebrisCount > 0.0) {
    nitroDebrisCount -= 0.25;
    Debris *d = new Debris(this, position, velocity, 1.0 + frandom() * 2.0);
    Game::current->add(d);
    d->position[0] += (frandom() - 0.5) * radius;
    d->position[1] += (frandom() - 0.5) * radius;
    d->position[2] += frandom() * radius;
    d->velocity[2] += 0.2;
    d->gravity = -0.1;
    d->modTimeLeft[MOD_GLASS] = -1.0;
    d->primaryColor = Color(0.1, 0.6, 0.1, 1.0);
    d->no_physics = true;
  }
}
void Ball::generateDebris(const Color &color) {
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
  Game::current->add(d);
  d->initialSize = 0.05;
  d->primaryColor = color.toOpaque();
  d->friction = 0.0;
  d->calcRadius();
}

void Ball::handleBallCollisions() {
  if (no_physics) return;

  Animated **balls;
  int nballs = Game::current->balls->bboxOverlapsWith(this, &balls);
  for (int i = 0; i < nballs; i++) {
    Ball *ball = (Ball *)balls[i];
    if (ball == this) continue;
    if (ball->no_physics) continue;

    Coord3d v = ball->position - position;
    double dist = length(v);
    if (dist < radius + ball->radius - 1e-3) {
      double err = radius + ball->radius - dist;
      position[0] -= err * v[0];
      position[1] -= err * v[1];
      position[2] -= err * v[2];
      v = v / length(v);
      double speed = dotProduct(v, velocity) - dotProduct(v, ball->velocity);
      if (speed < 1e-3) continue;
      double myWeight = radius * radius * radius,
             hisWeight = ball->radius * ball->radius * ball->radius,
             totWeight = myWeight + hisWeight;
      myWeight /= totWeight;
      hisWeight /= totWeight;
      this->crash(speed * hisWeight * 1.5 * (ball->modTimeLeft[MOD_SPIKE] ? 6.0 : 1.0));
      ball->crash(speed * myWeight * 1.5 * (this->modTimeLeft[MOD_SPIKE] ? 6.0 : 1.0));
      velocity = velocity - speed * v * 3.0 * hisWeight;
      ball->velocity = ball->velocity + speed * v * 3.0 * myWeight;
    }
  }
}
void Ball::handleForcefieldCollisions() {
  int n = Game::current->hooks[Role_Forcefield].size();
  for (int i = 0; i < n; i++) {
    ForceField *ff = (ForceField *)Game::current->hooks[Role_Forcefield][i];
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
    Coord3d v = this->position - ff->position;
    v[2] = 0.0;
    Coord3d ff_normal;
    ff_normal[0] = ff->direction[1];
    ff_normal[1] = ff->direction[0];
    ff_normal[2] = 0.0;
    ff_normal = ff_normal / length(ff_normal);
    double xy_dist = dotProduct(v, ff_normal);
    // if(xy_dist > this->radius) continue;
    ff_normal = ff->direction;
    ff_normal = ff_normal / length(ff_normal);
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
      ff_normal = ff_normal / length(ff_normal);

      // the direction we are hitting it from
      Coord3d v;
      v[0] = ff->position[0] + ff_where / ff_len * ff->direction[0] - position[0];
      v[1] = ff->position[1] + ff_where / ff_len * ff->direction[1] - position[1];
      v[2] = ff_where_h - position[2];
      v = v / length(v);

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
static void computePipeCoordinates(const Pipe *pipe, const Coord3d &position, double *radial,
                                   double *axial, double *pipeLength,
                                   Coord3d *direction_to_axis, Coord3d *axis_direction) {
  Coord3d direction = pipe->to - pipe->from;
  double pipelen = length(direction);
  Coord3d dirNorm = direction / length(direction);  // normalized direction
  Coord3d v0 = position - pipe->from;               // pipe enterance -> ball position
  double l = std::max(0.0, std::min(1.0, dotProduct(v0, dirNorm) /
                                             pipelen));  // where along pipe ball is projected
  Coord3d proj = pipe->from + l * direction;  // where (as pos) the ball is projected
  Coord3d v1 = position - proj;               // projection point -> ball position
  double distance = length(v1);               // how far away from the pipe the ball is
  *radial = distance;
  *axial = l;
  *pipeLength = pipelen;
  if (length(v1) > 0) v1 = v1 / length(v1);
  *direction_to_axis = v1;
  *axis_direction = dirNorm;
}
void Ball::handlePipes(Real time) {
  inPipe = false;

  int n = Game::current->hooks[Role_Pipe].size();
  for (int i = 0; i < n; i++) {
    Pipe *pipe = (Pipe *)Game::current->hooks[Role_Pipe][i];

    double distance, l, pipeLength;
    Coord3d dirNorm, offset;
    computePipeCoordinates(pipe, position, &distance, &l, &pipeLength, &offset, &dirNorm);
    if (distance < pipe->radius && l > 0.0 && l < 1.0) inPipe = true;
  }

  /* Then we compute the interaction with all the pipes */
  for (int i = 0; i < n; i++) {
    Pipe *pipe = (Pipe *)Game::current->hooks[Role_Pipe][i];

    double distance, l, pipeLength;
    Coord3d dirNorm, normal;
    computePipeCoordinates(pipe, position, &distance, &l, &pipeLength, &normal, &dirNorm);
    if (distance > pipe->radius || l == 0.0 || l == 1.0) {
      /* Ball is on the outside */
      // note. If <inPipe> then we are already inside another pipe. No collision!
      if (pipe->radius > 0.1 && pipe->radius < radius && distance < radius) {
        /* Added code to bounce of too small pipes, except for *realy* thin pipes which
         are meant to be lifts */
        double speed = -dotProduct(velocity, normal);
        if (speed > 0)
          for (int i = 0; i < 3; i++) velocity[i] += speed * normal[i] * 1.5;
        double correction = radius - distance;
        for (int i = 0; i < 3; i++) position[i] += correction * normal[i];
      } else if (distance < pipe->radius + radius && l != 0.0 && l != 1.0 && !inPipe) {
        /* Collision from outside */
        if ((pipe->flags & PIPE_SOFT_ENTER) && l < 0.2 / pipeLength) continue;
        if ((pipe->flags & PIPE_SOFT_EXIT) && l > 1.0 - 0.2 / pipeLength) continue;

        double speed = -dotProduct(velocity, normal);
        if (speed > 0)
          for (int i = 0; i < 3; i++) velocity[i] += speed * normal[i] * 1.5;
        double correction = pipe->radius + radius - distance;
        for (int i = 0; i < 3; i++) position[i] += correction * normal[i];
      }
    } else {
      if (distance > pipe->radius * 0.97 - radius && l != 0.0 && l != 1.0) {
        /* Collision from inside */
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
        normal[2] = std::min(0., normal[2]);
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
    int n = Game::current->hooks[Role_PipeConnector].size();
    for (int i = 0; i < n; i++) {
      PipeConnector *connector = (PipeConnector *)Game::current->hooks[Role_PipeConnector][i];

      Coord3d v0 = connector->position - position;  // ball -> connector
      double dist = length(v0);                     // Distance ball center, connector center
      if (dist > connector->radius) {
        /* Ball is outside connector */
        if (dist < connector->radius + radius) {
          /* Collision from outside */
          v0 = v0 / length(v0);
          double speed = dotProduct(velocity, v0);
          if (speed > 0) velocity = velocity - speed * v0 * 1.5;
          double correction = connector->radius + radius - dist;
          position = position - correction * v0;
        }
      } else {
        /* Ball is inside connector */
        inPipe = true;

        if (dist > connector->radius * 0.97 - radius) {
          /* Collision from inside */
          v0 = v0 / length(v0);
          double speed = dotProduct(velocity, v0);
          if (speed > 0) velocity = velocity + speed * v0 * 1.5;
          double correction = dist - (connector->radius * 0.97 - radius);
          position = position + correction * v0;
        }
        if (dist > connector->radius * 0.94 - radius && position[2] < connector->position[2]) {
          /* Ball is touching lower part of connector */
          inTheAir = false;
          v0 = v0 / length(v0);
          double scale = 0.5 * gravity * time / (-v0[2] + 1e-3);
          velocity[0] -= v0[0] * scale;
          velocity[1] -= v0[1] * scale;
        }
      }
    }
  }
}

void Ball::die(int how) { Animated::die(how); }
