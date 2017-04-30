/** \file bird.cc
   Travels along their path and kill player if touched.
*/
/*
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
#include "bird.h"
#include "ball.h"
#include "player.h"
#include "sign.h"
#include "scoreSign.h"
#include "sound.h"

using namespace std;

Bird::Bird(int x, int y, int dx, int dy, Real size, Real speed) {
  this->x = x;
  this->y = y;
  this->dx = dx - x;
  this->dy = dy - y;
  this->size = size;
  this->speed = speed;
  this->lng = sqrt((double)(this->dx * this->dx + this->dy * this->dy));
  position[0] = (Real)x + 0.5;
  position[1] = (Real)y + 0.5;
  position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;
  primaryColor[0] = 1.;
  primaryColor[1] = 1.;
  primaryColor[2] = 1.;
  secondaryColor[0] = .6;
  secondaryColor[1] = 0.8;
  secondaryColor[2] = 0.9;
  animation = 0.0;
  rotation = M_PI - atan2(this->dx / lng, this->dy / lng);
}

void Bird::draw2() {
  int i;

  if (hide > 0.) return;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0], position[1], position[2]);
  glRotatef(rotation / (2.0 * M_PI) * 360.0, 0.0, 0.0, 1.0);

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
  /*
  glNormal3f(0., 0., 1.);
  glBegin(GL_TRIANGLES);
  glVertex3f(size, 0., 0);
  glVertex3f(0., -size/2., 0);
  glVertex3f(0.,  size/2., 0);
  glVertex3f(size, 0., 0);
  glVertex3f(0.,  size/2., 0);
  glVertex3f(0., -size/2., 0);
  glEnd();
  */
  glDisable(GL_CULL_FACE);
  double angle =
      (0.8 - (animation < 0.5 ? animation * 2.0 : 2.0 - animation * 2.0)) * M_PI / 2.0;

  glEnable(GL_TEXTURE_2D);
  bindTexture("wings.png");

  glEnable(GL_BLEND);

  glBegin(GL_TRIANGLES);
  glNormal3f(-sin(angle), 0.0, cos(angle));
  glTexCoord2f(1.0, 1.0);
  glVertex3f(0.0, size, 0.0);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(size / 2. * cos(angle), size, size / 2. * sin(angle));

  glNormal3f(sin(angle), 0.0, cos(angle));
  glTexCoord2f(1.0, 1.0);
  glVertex3f(0.0, size, 0.0);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-size / 2. * cos(angle), size, size / 2. * sin(angle));
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

void Bird::tick(Real t) {
  animation = fmod(animation + t / 0.8, 1.0);

  // if hiden, just count down the time
  if (hide > 0.) {
    hide -= t;
    return;
  }

  // update the position on the line
  position[0] += speed * t * dx / lng;
  position[1] += speed * t * dy / lng;
  if (!(flags & BIRD_CONSTANT_HEIGHT))
    position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;

  // check for collisions with balls
  Ball *ball;
  Coord3d diff;
  set<Ball *>::iterator iter = Ball::balls->begin();
  set<Ball *>::iterator end = Ball::balls->end();

  for (; iter != end; iter++) {
    ball = *iter;
    if (!ball->alive) continue;
    if (ball->no_physics) continue;

    sub(position, ball->position, diff);
    // TODO: improve the collision detection
    if (length(diff) < ball->radius + size * .75) {
      if (ball->modTimeLeft[MOD_SPIKE]) {
        // the ball kills the bird !!!
        position[2] += 1.2;
        new ScoreSign(25, position);
        playEffect(SFX_BIRD_DIE);
        // restart the bird
        position[0] = x;
        position[1] = y;
        position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;
        hide = 1.;
        return;
      }
      // kill the ball
      ball->die(DIE_CRASH);
    }
  }

  // check if the bird is outside its path
  Real cdx, cdy;

  cdx = position[0] - x;
  cdy = position[1] - y;
  if (((dx < 0.) && (cdx <= dx)) || ((dx > 0.) && (cdx >= dx)) || ((dy < 0.) && (cdy <= dy)) ||
      ((dy > 0.) && (cdy >= dy))) {
    // restart
    position[0] = x;
    position[1] = y;
    position[2] = Game::current->map->getHeight(position[0], position[1]) + .5;
    hide = .5;  // wait .5 sec before restarting
  }
}
