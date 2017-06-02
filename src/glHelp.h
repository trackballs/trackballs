/* glHelp.h
   Some misc. help functions for GL commands etc...

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

#ifndef GLHELP_H
#define GLHELP_H

#include "font.h"

/* Prototypes */
void glHelpInit();
void regenerateSphereDisplaylists();
GLuint loadProgram(const char *fragment, const char *vertex);
double frand();
double frand(int);

/* displays a 2d text on specific screen coordinates */
void draw2DString(TTF_Font *, const char *, int x, int y, float red, float green, float blue,
                  float alpha, int outlined = 0);
void update2DStringCache();
/* Common interface for drawing 2d things on the screen */
void draw2DRectangle(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat tx, GLfloat ty,
                     GLfloat tw, GLfloat th, GLfloat r, GLfloat g, GLfloat b, GLfloat a,
                     GLuint tex = 0);
void draw2DQuad(const GLfloat corners[4][2], const GLfloat texture[4][2],
                const GLfloat color[4][4], GLuint tex = 0);

void drawTextured2DRectangle(int x, int y, int w, int h);
void tickMouse(Real td);
void drawMousePointer();
void drawMouse(int x, int y, int w, int h, Real td);
void drawSpike(Coord3d a, Coord3d b, Coord3d c, Coord3d d);

size_t packObjectVertex(void *dest, GLfloat x, GLfloat y, GLfloat z, GLfloat tx, GLfloat ty,
                        GLfloat color[3], GLfloat normal[3]);
void configureObjectAttributes();
void setupObjectRenderState();
inline uint32_t packNormal(const GLfloat n[3]) {
  uint32_t d = (512 + n[2] * 511.f);
  uint32_t e = (512 + n[1] * 511.f);
  uint32_t f = (512 + n[0] * 511.f);
  uint32_t x = 0;
  // Alpha (<<30) unused
  x |= d << 20;
  x |= e << 10;
  x |= f << 0;
  return x;
}

// generates a snapshot
int createSnapshot();
// displays a semitransparent centered sign with two text rows */
void message(char *row1, char *row2);
void multiMessage(int nlines, const char *left[], const char *right[]);
void displayFrameRate();

void assign(const float[3], float[3]);
void assign(const double[3], double[3]);
void crossProduct(const double[3], const double[3], double[3]);
double dotProduct(const double[3], const double[3]);
void add(const double[3], const double[3], double[3]);
void sub(const double[3], const double[3], double[3]);
void normalize(double[3]);
double length(double[3]);
void zero(double[3]);

void debugMatrix(Matrix4d);
void useMatrix(Matrix4d, const double[3], double[3]);
void useMatrix(Matrix3d, const double[3], double[3]);
void identityMatrix(Matrix4d);
void assign(const Matrix4d, Matrix4d);
void transpose(const Matrix4d, Matrix4d);
void matrixMult(const Matrix4d, const Matrix4d, Matrix4d);
void rotateX(double, Matrix4d);
void rotateY(double, Matrix4d);
void rotateZ(double, Matrix4d);
void makeProjectionMatrix(const Coord3d p1, const Coord3d p2, const Coord3d p3, Matrix3d m);
int power_of_two(int input);
int testBboxClip(double x1, double x2, double y1, double y2, double z1, double z2,
                 const double *model, const double *proj);

void Require2DMode();
void Enter2DMode();
void Leave2DMode();
GLuint LoadTexture(SDL_Surface *surface, GLfloat *texcoord, int linearFilter = 0,
                   GLuint *texture = NULL);

int loadTexture(
    const char *name);  // preloads a texture from file and returns position in textures array
int bindTexture(
    const char *name);  // binds texture. Also returns index in textures array used for this.
int resetTextures();    // Reloads all textures

extern GLuint textures[256];
extern char *textureNames[256];  // the names of preloaded textures
extern int numTextures;
extern GLuint shaderWater, shaderTile, shaderTileRim, shaderUI, shaderObject;
extern GLuint theVao;

/* Globals */
extern float fps;
extern float realTimeNow;
extern int screenWidth, screenHeight;
extern TTF_Font *msgFont, *infoFont, *ingameFont, *menuFont, *scrollFont;
TTF_Font *menuFontForSize(int sz);

/* A set of predefined displaylists for drawing balls of unit size. */
#define BALL_LORES 0
#define BALL_NORMAL 1
#define BALL_HIRES 2
extern GLuint sphereDisplayLists[3];

extern const GLfloat white[4], black[4];
extern GLUquadricObj *qball;
// extern SDL_Surface *mousePointer;

/***********************************/
/*  Inlined vector operations      */

#define INLINE_VECTOR_OPS
#ifdef INLINE_VECTOR_OPS

/* C <- A + B */
inline void add(const double A[3], const double B[3], double C[3]) {
  for (int i = 0; i < 3; i++) C[i] = A[i] + B[i];
}

/* C <- A - B */
inline void sub(const double A[3], const double B[3], double C[3]) {
  for (int i = 0; i < 3; i++) C[i] = A[i] - B[i];
}
/* C <- C * 1 / |C| */
inline void normalize(double C[3]) {
  double l = sqrt(C[0] * C[0] + C[1] * C[1] + C[2] * C[2]);
  C[0] /= l;
  C[1] /= l;
  C[2] /= l;
}
/* |A| */
inline double length(double A[3]) { return sqrt(A[0] * A[0] + A[1] * A[1] + A[2] * A[2]); }
/* C <- A x B */
inline void crossProduct(const double A[3], const double B[3], double C[3]) {
  C[0] = A[1] * B[2] - A[2] * B[1];
  C[1] = A[2] * B[0] - A[0] * B[2];
  C[2] = A[0] * B[1] - A[1] * B[0];
}

/* <- A . B */
inline double dotProduct(const double A[3], const double B[3]) {
  return A[0] * B[0] + A[1] * B[1] + A[2] * B[2];
}
#endif

#endif
