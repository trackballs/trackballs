/* ball.h
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

#ifndef BALL_H
#define BALL_H

#ifndef ANIMATED_H
#include "animated.h"
#endif

#define MOD_SPEED 0
#define MOD_JUMP 1
#define MOD_SPIKE 2
#define MOD_GLASS 3
#define MOD_DIZZY 4
#define MOD_FROZEN 5
#define MOD_FLOAT 6
#define MOD_EXTRA_LIFE 7  // this is not a real mod.
#define MOD_SMALL 8
#define MOD_LARGE 9
#define MOD_NITRO 10

#define NUM_MODS 11

#define DIE_CRASH 0
#define DIE_FF 1
#define DIE_ACID 2
#define DIE_TIMEOUT 3
#define DIE_OTHER 4

class Ball : public Animated {
 public:
  Ball();
  virtual ~Ball();
  Boolean physics(Real time);
  void draw();
  void draw2();
  void tick(Real);
  void doExpensiveComputations();
  void onRemove();
  void setReflectivity(double reflectivity, int metallic);
  virtual Boolean crash(Real speed);  // Generic crash called when we hit ground etc. Returns
                                      // if we are still alive
  virtual Boolean crash(
      Real speed, Ball *);  // When we crash into another ball. Returns if we are still alive
  virtual void die(int how) = 0;

  static void init();
  static void reset();

  Coord3d velocity;
  int inTheAir, inPipe;
  float modTimeLeft[NUM_MODS], modTimePhaseIn[NUM_MODS];
  float rotation[2];

  // radius is the *effective* radius, realRadius the real one
  Real radius, realRadius;
  Real friction, gravity, bounceFactor;
  Real crashTolerance;
  int resolution, no_physics;
  int ballResolution; /* 0 for low resolution balls (eg. debris), 1 for normal balls, 2 for
                         high res balls (eg. player) */

  static const Real physicsResolution;
  static class std::set<Ball *> *balls;

  double sink;  // how far the ball has sunk into acid / sand / etc.
  Matrix4d rotations;

  GLuint environmentTexture;

 protected:
  double nitroDebrisCount;

 private:
  Boolean checkGroundCollisions(class Map *, Real x, Real y);
  double reflectivity;
  int metallic, dontReflectSelf;

  static GLuint dizzyTexture;
  static GLfloat dizzyTexMinX, dizzyTexMinY;
  static GLfloat dizzyTexMaxX, dizzyTexMaxY;

  void generateSandDebris();
  void generateDebris(GLfloat color[4]);
  void handleEdges();
  void handleBallCollisions();
  void handleForcefieldCollisions();
  void handlePipes();
};

#endif
