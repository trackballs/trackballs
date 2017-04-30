/* cactus.cc
   Hurts when touched

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

#include "general.h"
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "cactus.h"
#include "ball.h"
#include "player.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"

using namespace std;

Cactus::Cactus(int x, int y, Real radius) {
  this->x = x;
  this->y = y;
  this->radius = radius;
  position[0] = x + 0.5;
  position[1] = y + 0.5;
  position[2] = Game::current->map->getHeight(position[0], position[1]);
  primaryColor[0] = .3;
  primaryColor[1] = 1.;
  primaryColor[2] = .4;
  secondaryColor[0] = 0.2;
  secondaryColor[1] = 0.8;
  secondaryColor[2] = 0.3;
  this->killed = 0;
  this->killed_time = 1.;
  this->base_radius = radius;

  scoreOnDeath = Game::defaultScores[SCORE_CACTUS][0];
  timeOnDeath = Game::defaultScores[SCORE_CACTUS][1];
}

void Cactus::draw() {
  int i;

  if (killed == 2) return;

  double d1 = Game::current->gameTime * 1.0, d2 = 3.0, d3 = 0.5;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0], position[1], position[2]);

  GLfloat color[4];
  for (i = 0; i < 3; i++) color[i] = primaryColor[i];
  color[3] = 1.0;

  GLfloat specular[4];
  for (i = 0; i < 3; i++) specular[i] = specularColor[i];
  specular[3] = 1.0;

  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
  glShadeModel(GL_SMOOTH);

  glColor3f(1.0, 1.0, 1.0);

#define STEP (2. * 3.14159 / 6.)
  float t, p1[3], p2[3], p3[3], p4[3];

  glBegin(GL_QUADS);
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    p1[0] = cos(t);
    p1[1] = sin(t);
    p1[2] = 0.;
    p2[0] = cos(t + STEP);
    p2[1] = sin(t + STEP);
    p2[2] = 0.;
    p3[0] = cos(t + STEP);
    p3[1] = sin(t + STEP);
    p3[2] = .35;
    p4[0] = cos(t);
    p4[1] = sin(t);
    p4[2] = .35;

    glNormal3f(p1[0], p1[1], -.2);
    glVertex3f(.5 * radius * p1[0], .5 * radius * p1[1], p1[2]);
    glNormal3f(p2[0], p2[1], -.2);
    glVertex3f(.5 * radius * p2[0], .5 * radius * p2[1], p2[2]);
    glNormal3f(p3[0], p3[1], -.1);
    glVertex3f(1 * radius * p3[0], 1 * radius * p3[1], p3[2]);
    glNormal3f(p4[0], p4[1], -.1);
    glVertex3f(1 * radius * p4[0], 1 * radius * p4[1], p4[2]);
  }
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    p1[0] = cos(t);
    p1[1] = sin(t);
    p1[2] = .35;
    p2[0] = cos(t + STEP);
    p2[1] = sin(t + STEP);
    p2[2] = .35;
    p3[0] = cos(t + STEP);
    p3[1] = sin(t + STEP);
    p3[2] = .6;
    p4[0] = cos(t);
    p4[1] = sin(t);
    p4[2] = .6;

    glNormal3f(p1[0], p1[1], -.1);
    glVertex3f(1 * radius * p1[0], 1 * radius * p1[1], p1[2]);
    glNormal3f(p2[0], p2[1], -.1);
    glVertex3f(1 * radius * p2[0], 1 * radius * p2[1], p2[2]);
    glNormal3f(p3[0], p3[1], .2);
    glVertex3f(.9 * radius * p3[0], .9 * radius * p3[1], p3[2]);
    glNormal3f(p4[0], p4[1], .2);
    glVertex3f(.9 * radius * p4[0], .9 * radius * p4[1], p4[2]);
  }
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    p1[0] = cos(t);
    p1[1] = sin(t);
    p1[2] = .6;
    p2[0] = cos(t + STEP);
    p2[1] = sin(t + STEP);
    p2[2] = .6;
    p3[0] = cos(t + STEP);
    p3[1] = sin(t + STEP);
    p3[2] = .8;
    p4[0] = cos(t);
    p4[1] = sin(t);
    p4[2] = .8;

    glNormal3f(p1[0], p1[1], 0.2);
    glVertex3f(.9 * radius * p1[0], .9 * radius * p1[1], p1[2]);
    glNormal3f(p2[0], p2[1], 0.2);
    glVertex3f(.9 * radius * p2[0], .9 * radius * p2[1], p2[2]);
    glNormal3f(p3[0], p3[1], 0.35);
    glVertex3f(.6 * radius * p3[0], .6 * radius * p3[1], p3[2]);
    glNormal3f(p4[0], p4[1], 0.35);
    glVertex3f(.6 * radius * p4[0], .6 * radius * p4[1], p4[2]);
  }
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    p1[0] = cos(t);
    p1[1] = sin(t);
    p1[2] = .8;
    p2[0] = cos(t + STEP);
    p2[1] = sin(t + STEP);
    p2[2] = .8;

    glNormal3f(p1[0], p1[1], 0.35);
    glVertex3f(.6 * radius * p1[0], .6 * radius * p1[1], p1[2]);
    glNormal3f(p2[0], p2[1], 0.35);
    glVertex3f(.6 * radius * p2[0], .6 * radius * p2[1], p2[2]);
    glNormal3f(0., 0., 1.);
    glVertex3f(0., 0., 1.);
    glNormal3f(0., 0., 1.);
    glVertex3f(0., 0., 1.);
  }
  glEnd();

  // drawing spikes
  Coord3d a, b, c, d;
  glPushMatrix();
  glColor3f(.85 * color[0], .85 * color[1], .85 * color[2]);
  glTranslatef(0., 0., .17);
  a[0] = .89 * radius;
  a[1] = 0.;
  a[2] = -0.05;
  b[0] = 0.;
  b[1] = 0.;
  b[2] = .05;
  c[0] = 0.;
  c[1] = .05;
  c[2] = -.05;
  d[0] = 0.;
  d[1] = -.05;
  d[2] = -.05;
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    glPushMatrix();
    glRotatef((t + STEP / 2.) * 360. / (2. * 3.14159), 0., 0., 1.);
    drawSpike(a, b, c, d);
    glPopMatrix();
  }
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0., 0., .39);
  a[0] = 1.2 * radius;
  a[1] = 0.;
  a[2] = 0.;
  b[0] = 0.;
  b[1] = 0.;
  b[2] = .05;
  c[0] = 0.;
  c[1] = .05;
  c[2] = -.05;
  d[0] = 0.;
  d[1] = -.05;
  d[2] = -.05;
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    glPushMatrix();
    glRotatef((t + STEP / 2.) * 360. / (2. * 3.14159), 0., 0., 1.);
    drawSpike(a, b, c, d);
    glPopMatrix();
  }
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0., 0., .68);
  a[0] = .99 * radius;
  a[1] = 0.;
  a[2] = 0.003;
  b[0] = 0.;
  b[1] = 0.;
  b[2] = .05;
  c[0] = 0.;
  c[1] = .05;
  c[2] = -.05;
  d[0] = 0.;
  d[1] = -.05;
  d[2] = -.05;
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    glPushMatrix();
    glRotatef((t + STEP / 2.) * 360. / (2. * 3.14159), 0., 0., 1.);
    drawSpike(a, b, c, d);
    glPopMatrix();
  }
  glPopMatrix();
  glPushMatrix();
  glTranslatef(0., 0., .89);
  a[0] = .44 * radius;
  a[1] = 0.;
  a[2] = 0.02;
  b[0] = 0.;
  b[1] = 0.;
  b[2] = .05;
  c[0] = 0.;
  c[1] = .05;
  c[2] = -.05;
  d[0] = 0.;
  d[1] = -.05;
  d[2] = -.05;
  for (t = 0.; t < 2. * 3.14159; t += STEP) {
    glPushMatrix();
    glRotatef((t + STEP / 2.) * 360. / (2. * 3.14159), 0., 0., 1.);
    drawSpike(a, b, c, d);
    glPopMatrix();
  }
  glPopMatrix();

#undef STEP

  glPopMatrix();
  glPopAttrib();
}

void Cactus::tick(Real t) {
  position[2] = Game::current->map->getHeight(position[0], position[1]);
  Player *p = Game::current->player1;
  Coord3d diff;

  // no more used (can be deleted ?)
  if (killed == 2) return;

  if (killed) {
    killed_time -= t;
    radius = killed_time * base_radius;

    if (radius <= 0.) {
      remove();
      // killed = 2;
      return;
    }
  }

  // do I parse all the balls (incl. Mr Black) or just the player ?
  Ball *ball;
  set<Ball *>::iterator iter = Ball::balls->begin();
  set<Ball *>::iterator end = Ball::balls->end();
  for (; iter != end; iter++) {
    ball = *iter;
    if (!ball->alive) continue;
    if (ball->no_physics) continue;

    sub(position, ball->position, diff);
    if (length(diff) < ball->radius + radius) {
      if (ball->modTimeLeft[MOD_SPIKE]) {
        Animated::die(DIE_OTHER);

        // the ball kills the cactus !!!
        if (killed == 1) return;  // still killed

        killed = 1;
        /*position[2] += 1.2;
        new ScoreSign(100,position);
        position[2] -= 1.2;*/
        playEffect(SFX_CACTUS_DIE);
        return;
      }

      ball->die(DIE_CRASH);
    }
  }
}
