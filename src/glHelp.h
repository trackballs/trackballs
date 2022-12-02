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

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "general.h"

typedef struct _TTF_Font TTF_Font;
typedef struct SDL_Surface SDL_Surface;
class Map;
class Game;

/* Prototypes */
void glHelpInit();
void glHelpCleanup();
void warnForGLerrors(const char *where_am_i);

/* renders and caches a drawing of a 2d string; returns the recommended width
 * to draw it at. */
int prepare2DString(TTF_Font *font, const char *string, bool outlined);

/* displays a 2d text on specific screen coordinates, returning width */
int draw2DString(TTF_Font *, const char *, int x, int y, Color color, bool outlined, int align,
                 int maxwidth = 0);
void update2DStringCache(bool force_wipe);
/* Common interface for drawing 2d things on the screen */
void draw2DRectangle(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat tx, GLfloat ty,
                     GLfloat tw, GLfloat th, Color color, GLuint tex = 0);
void draw2DQuad(const GLfloat corners[4][2], const GLfloat texture[4][2], const Color color[4],
                GLuint tex = 0);

void tickMouse(Real td);
void drawMousePointer();
void drawMouse(int x, int y, int w, int h);
size_t packObjectVertex(void *dest, GLfloat x, GLfloat y, GLfloat z, GLfloat tx, GLfloat ty,
                        const Color &color, const GLfloat normal[3]);
void configureObjectAttributes();
void configureUIAttributes();
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
void countObjectSpherePoints(int *ntriangles, int *nvertices, int detail);
void placeObjectSphere(void *data, ushort *idxs, ushort first_index, GLfloat const position[3],
                       Matrix3d rotation, GLfloat radius, int detail, const Color &color);

typedef enum {
  Shader_Tile,
  Shader_Object,
  Shader_Water,
  Shader_Line,
  Shader_Reflection,
} Shader_Type;
/* call markViewChanged when activeView is changed */
void markViewChanged();
/* setActiveProgramAndUniforms switches program, sets view uniforms, and updates common uniform
 * locations */
void setActiveProgramAndUniforms(Shader_Type shader);
/* update object shader uniforms */
typedef enum {
  Lighting_None,      /* No shadows, orientation independent */
  Lighting_NoShadows, /* Orientation dependent colors, no shadows */
  Lighting_Regular    /* Shadows and orientation dependent colors */
} Object_Lighting;
void setObjectUniforms(Color specular, float sharpness, Object_Lighting lighting);
/* if the currently active shader has a uniform with matching name, then this struct provides
 * the location */
struct UniformLocations {
  GLuint line_color;
  GLuint refl_color;
  GLuint tex;
  GLuint render_stage;
  GLuint gameTime;
  GLuint arrtex;
  GLuint wtex;
  GLuint specular_color;
  GLuint sharpness;
  GLuint ignore_shadow;
  GLuint use_lighting;
  // For UI shader
  GLuint ui_screen_width;
  GLuint ui_screen_height;
  GLuint ui_tex;
};
extern UniformLocations uniformLocations;

typedef struct _viewpa {
  Matrix4d modelview;
  Matrix4d projection;

  int fog_enabled;
  GLfloat fog_color[3];
  GLfloat fog_start;
  GLfloat fog_end;

  GLfloat light_ambient[3];
  GLfloat light_diffuse[3];
  GLfloat light_specular[3];

  GLfloat light_position[3];
  GLfloat sun_direction[3];

  bool use_shadows;
  bool day_mode;
  GLuint cascadeTexsize;
  GLuint cascadeTexture[3];
  Matrix4d cascade_proj[3];
  Matrix4d cascade_model[3];

  GLuint shadowMapTexture;
  GLuint shadowMapTexsize;
  bool calculating_shadows;

  bool show_flag_state;
} ViewParameters;

extern ViewParameters activeView;

void perspectiveMatrix(GLdouble fovy_deg, GLdouble aspect, GLdouble zNear, GLdouble zFar,
                       Matrix4d out);
void lookAtMatrix(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX,
                  GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ,
                  Matrix4d out);

void renderShadowMap(const Coord3d &focus, Map *mp, Game *gm);
void renderShadowCascade(const Coord3d &focus, Map *mp, Game *gm);

// generates a snapshot
int createSnapshot();
// displays a semitransparent centered sign with two text rows */
void message(char *row1, char *row2);
void multiMessage(int nlines, const char *left[], const char *right[]);
void displayFrameRate();

Coord3d crossProduct(const Coord3d &, const Coord3d &);
double dotProduct(const Coord3d &, const Coord3d &);
double length(const Coord3d &);

void debugMatrix(Matrix4d);
Coord3d useMatrix(Matrix4d, const Coord3d &);
Coord3d useMatrix(Matrix3d, const Coord3d &);
void identityMatrix(Matrix4d);
void assign(const Matrix4d, Matrix4d);
void matrixMult(const Matrix4d, const Matrix4d, Matrix4d);
void rotateX(double, Matrix4d);
void rotateY(double, Matrix4d);
void rotateZ(double, Matrix4d);
inline void assign(const GLfloat A[3], GLfloat B[3]) {
  for (int k = 0; k < 3; k++) B[k] = A[k];
}
int powerOfTwo(int input);
bool testBboxClip(double x1, double x2, double y1, double y2, double z1, double z2,
                  const Matrix4d model_times_proj);

/* Cache small polygon coordinates */
extern const double sin6[6];
extern const double sin10[10];
extern const double sin12[12];
extern const double sin14[14];
extern const double cos6[6];
extern const double cos10[10];
extern const double cos12[12];
extern const double cos14[14];

void Require2DMode();
void Enter2DMode();
void Leave2DMode();
GLuint LoadTexture(SDL_Surface *surface, GLfloat *texcoord);

// preloads texture from file (if not already loaded); returns texture array position
int loadTexture(const char *name);
/* Loads image from SHARE_DIR/image/ */
SDL_Surface *loadImage(const char *imagename);

extern GLuint textures[256];
extern char *textureNames[256];  // the names of preloaded textures
extern int numTextures;
extern GLuint textureBlank, textureGlitter, textureMousePointer, textureDizzy, textureWings,
    textureTrack;  // immediate texture lookup

/* Globals */
extern float fps;
extern int screenWidth, screenHeight;
extern TTF_Font *ingameFont;
TTF_Font *menuFontForSize(int sz);
extern const Color menuColorSelected, menuColor;

/***********************************/
/*  Inlined vector operations      */

/* |A| */
inline double length(const Coord3d &A) {
  return std::sqrt(A[0] * A[0] + A[1] * A[1] + A[2] * A[2]);
}
/* C <- A x B */
inline Coord3d crossProduct(const Coord3d &A, const Coord3d &B) {
  Coord3d C;
  C[0] = A[1] * B[2] - A[2] * B[1];
  C[1] = A[2] * B[0] - A[0] * B[2];
  C[2] = A[0] * B[1] - A[1] * B[0];
  return C;
}

/* <- A . B */
inline double dotProduct(const Coord3d &A, const Coord3d &B) {
  return A[0] * B[0] + A[1] * B[1] + A[2] * B[2];
}

/* Inlined helper function */
inline void glUniformC(GLint l, const Color &c) {
  glUniform4f(l, c.f0(), c.f1(), c.f2(), c.f3());
}
#endif
