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

#define BALL_LORES 0
#define BALL_NORMAL 1
#define BALL_HIRES 2

#define PHYSICS_RESOLUTION 0.002

#include <set>
#include "animated.h"

/** This class is an ABSTRACT class from which specific balls
    (player/opponents/babies) are inherited */
class Ball : public Animated {
 public:
  Ball();
  virtual ~Ball();
  bool physics(Real time);

  virtual int generateBuffers(GLuint*& idxbufs, GLuint*& databufs);
  virtual void drawBuffers1(GLuint* idxbufs, GLuint* databufs);
  virtual void drawBuffers2(GLuint* idxbufs, GLuint* databufs);

  void tick(Real);
  void doExpensiveComputations();
  void onRemove();
  void setReflectivity(double reflectivity, int metallic);
  /* When we crash into something. Returns if we are still alive */
  virtual bool crash(Real speed);
  virtual void die(int how);

  static void init();

  Coord3d velocity;
  int inTheAir, inPipe;
  float modTimeLeft[NUM_MODS], modTimePhaseIn[NUM_MODS];
  float rotation[2];

  /* radius is the *effective* radius, realRadius the real one */
  Real radius, realRadius;
  Real friction, gravity, bounceFactor;
  Real crashTolerance;
  int no_physics;
  /** 0 for low resolution balls (eg. debris), 1 for normal balls, 2
       for high res balls (eg. player) */
  int ballResolution;

  /** how far the ball has sunk into acid / sand / etc. */
  double sink;
  Matrix4d rotations;

  GLuint environmentTexture;

 protected:
  double nitroDebrisCount;

 private:
  double reflectivity;
  int metallic, dontReflectSelf;

  static GLfloat dizzyTexCoords[4];

  void generateSandDebris();
  void generateDebris(GLfloat color[4]);
  void handleBallCollisions();
  void handleForcefieldCollisions();
  void handlePipes(Real time);
  bool handleMapCollisions(class Map*);
  bool handleEdges(class Map*);
};

#endif
