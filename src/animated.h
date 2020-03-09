/** \file animated.h
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

#ifndef ANIMATED_H
#define ANIMATED_H

#include "gameHook.h"
#include "general.h"
#include "glHelp.h"

#define DIE_CRASH 0
#define DIE_FF 1
#define DIE_ACID 2
#define DIE_TIMEOUT 3
#define DIE_OTHER 4

class Animated : public GameHook {
 public:
  explicit Animated(Game& g, int role, int maxVBOpairsOrVAOs);

  virtual ~Animated();
  /** Setup drawing pass of object.*/

  /** Generate all buffers possibly used in this tick */
  virtual void generateBuffers(const GLuint* idxbufs, const GLuint* databufs,
                               const GLuint* vaolist, bool mustUpdate) const = 0;
  /** First drawing pass of object. Render opaque buffers. */
  virtual void drawBuffers1(const GLuint* vaolist) const = 0;
  /** Draws the second pass of object. Render alpha buffers if needed. */
  virtual void drawBuffers2(const GLuint* vaolist) const = 0;

  /* Object drawing passes */
  void draw();
  void draw2();
  void drawBoundingBox() const;

  virtual void tick(Real dt);
  virtual void die(int how);

  /** General purpose field. Semantics defined by children */
  int flags;

  /** Center position of object */
  Coord3d position;
  /** Lower/higher coordinate of boundingbox relative to position */
  Coord3d boundingBox[2];

  Color primaryColor, secondaryColor, specularColor;
  GLuint texture;

  /** Number of points the player is awarded when this object dies */
  double scoreOnDeath;
  /** Time modification player is awarded when this object dies */
  double timeOnDeath;

  /** Computed by game::draw if the object appears roughly on the screen or not. */
  bool onScreen;
  /** Set by guile interface if object parameters have been altered; used to
   *  update otherwise static objects */
  bool drawChanged;

 private:
  void setupVBOs();

  int lastFrameNumber;
  GLuint* glidxs;
  const GLuint nVOs;
};

#endif
