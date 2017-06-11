/** \file glHelp.cc
   Some OpenGL utility algorithms.
*/
/*
   Copyright (C) 2000-2005  Mathias Broxvall
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

#include "glHelp.h"

#include "font.h"
#include "map.h"
#include "settings.h"
#include "sparkle2d.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <map>

float fps = 50.0;
int screenWidth = 640, screenHeight = 480;
ViewParameters activeView;

GLuint shaderWater = 0;
GLuint shaderTile = 0;
GLuint shaderLine = 0;
GLuint shaderUI = 0;
GLuint shaderObject = 0;
GLuint shaderReflection = 0;
GLuint theVao = 0;

TTF_Font *ingameFont;
extern double displayStartTime;

const GLfloat menuColorSelected[4] = {0.86f, 0.86f, 0.86f, 1.f};
const GLfloat menuColor[4] = {0.86f, 0.86f, 0.25f, 1.f};

double fake_rand[4711];
double frand(int i) { return fake_rand[i % 4711]; }
double frand() { return (rand() % (1 << 30)) / ((double)(1 << 30)); }

#define GLHELP_MAX_TEXTURES 256
GLuint textures[GLHELP_MAX_TEXTURES] = {0};  // added init. to 0 (no texture)
char *textureNames[GLHELP_MAX_TEXTURES] = {NULL};
int numTextures;

static std::map<int, TTF_Font *> menuFontLookup;

Sparkle2D *sparkle2D = NULL;
/*
SDL_Surface *mousePointer;
GLuint mousePointerTexture;
*/

struct StringInfo {
  TTF_Font *font;
  char string[256];
  SDL_Color color;
};
int operator<(const struct StringInfo &a, const struct StringInfo &b) {
  return memcmp(&a, &b, sizeof(struct StringInfo)) < 0;
}

struct StringCache {
  GLuint texture;
  GLfloat texcoord[4];
  int w, h;
  long tick;
};

static long stringTick = 0;
static std::map<StringInfo, StringCache> strcache;

static SDL_Surface *drawStringToSurface(struct StringInfo &inf, int outlined) {
  SDL_Surface *outline;
  if (outlined) {
    TTF_SetFontOutline(inf.font, 0);
    SDL_Surface *inner = TTF_RenderUTF8_Blended(inf.font, inf.string, inf.color);
    if (!inner) {
      warning("Failed to render string outline '%s'", inf.string);
      return NULL;
    }
    TTF_SetFontOutline(inf.font, 2);
    SDL_Color blkColor = {0, 0, 0, 0};
    outline = TTF_RenderUTF8_Blended(inf.font, inf.string, blkColor);
    if (!outline) {
      warning("Failed to render string inside '%s'", inf.string);
      return NULL;
    }
    SDL_Rect rect = {2, 2, inner->w, inner->h};
    SDL_SetSurfaceBlendMode(inner, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(inner, NULL, outline, &rect);
    SDL_FreeSurface(inner);
  } else {
    outline = TTF_RenderUTF8_Blended(inf.font, inf.string, inf.color);
    if (!outline) {
      warning("Failed to render string '%s'", inf.string);
      return NULL;
    }
  }
  return outline;
}

int draw2DString(TTF_Font *font, const char *string, int x, int y, float red, float green,
                 float blue, float alpha, int outlined, int align) {
  struct StringInfo inf;
  inf.color.r = 255 * red;
  inf.color.g = 255 * green;
  inf.color.b = 255 * blue;
  inf.color.a = 255 * alpha;
  inf.font = font;
  memset(inf.string, 0, 256);
  strncpy(inf.string, string, 255);
  inf.string[255] = '\0';
  if (strcache.count(inf) <= 0) {
    struct StringCache newentry;
    newentry.tick = stringTick;
    SDL_Surface *surf = drawStringToSurface(inf, outlined);
    if (!surf) { return 0; }
    newentry.texture = LoadTexture(surf, newentry.texcoord, 1, NULL);
    newentry.w = surf->w;
    newentry.h = surf->h;
    SDL_FreeSurface(surf);
    strcache[inf] = newentry;
  }

  struct StringCache &cached = strcache[inf];
  cached.tick = stringTick;

  y -= cached.h / 2;

  draw2DRectangle(x - align * cached.w / 2, y, cached.w, cached.h, cached.texcoord[0],
                  cached.texcoord[1], cached.texcoord[2], cached.texcoord[3], 1., 1., 1., 1.,
                  cached.texture);
  return cached.w;
}

void update2DStringCache() {
  stringTick++;
  int erased;
  do {
    erased = false;
    for (std::map<StringInfo, StringCache>::iterator i = strcache.begin(); i != strcache.end();
         ++i) {
      if (i->second.tick < stringTick - 10) {
        glDeleteTextures(1, &i->second.texture);
        strcache.erase(i);
        erased = true;
        break;
      }
    }
  } while (erased);
}

TTF_Font *menuFontForSize(int sz) {
  if (!menuFontLookup.count(sz)) {
    char str[256];
    snprintf(str, sizeof(str), "%s/fonts/%s", effectiveShareDir, "FreeSerifBoldItalic.ttf");
    menuFontLookup[sz] = TTF_OpenFont(str, 2 * sz);  // barbatri
    if (!menuFontLookup[sz]) { error("failed to load font %s", str); }
  }
  return menuFontLookup[sz];
}

double mousePointerPhase = 0.0;

void tickMouse(Real td) {
  int mouseX, mouseY;
  static int oldMouseX = 0, oldMouseY = 0;
  SDL_GetMouseState(&mouseX, &mouseY);
  double mouseSpeedX = (mouseX - oldMouseX) / td;
  double mouseSpeedY = (mouseY - oldMouseY) / td;
  static Real last_sparkle = 0.0;
  if (mouseSpeedX > 20.) mouseSpeedX = 20.;
  if (mouseSpeedY > 20.) mouseSpeedY = 20.;
  if (mouseSpeedX < -20.) mouseSpeedX = -20.;
  if (mouseSpeedY < -20.) mouseSpeedY = -20.;

  oldMouseX = mouseX;
  oldMouseY = mouseY;

  mousePointerPhase += td;
  sparkle2D->tick(td);
  last_sparkle += td * (1.0 + (fabs(mouseSpeedX) + fabs(mouseSpeedY)) * 0.1);
  while (last_sparkle > 0.0) {
    last_sparkle -= 0.05;
    sparkle2D->add(
        (float)mouseX + 10.0 * (frandom() - 0.5), (float)mouseY + 10.0 * (frandom() - 0.5),
        (float)mouseSpeedX / 2. + 100.0 * (frandom() - 0.5),
        (float)mouseSpeedY / 2. - 120.0 * frandom(), (float)(1.5 + 0.5 * (frandom() - 0.5)),
        (float)(11 + 3. * (frandom() - 0.5)), 0.8 + 0.2 * frandom(), 0.8 + 0.2 * frandom(),
        0.5 + 0.2 * frandom(), 0.9 - 0.35 * frandom());
  }
}

void draw2DRectangle(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat tx, GLfloat ty,
                     GLfloat tw, GLfloat th, GLfloat r, GLfloat g, GLfloat b, GLfloat a,
                     GLuint tex) {
  GLfloat corners[4][2] = {x, y, x, y + h, x + w, y, x + w, y + h};
  GLfloat texture[4][2] = {tx, ty, tx, ty + th, tx + tw, ty, tx + tw, ty + th};
  GLfloat colors[4][4] = {r, g, b, a, r, g, b, a, r, g, b, a, r, g, b, a};
  draw2DQuad(corners, texture, colors, tex);
}

void draw2DQuad(const GLfloat ver[4][2], const GLfloat txc[4][2], const GLfloat col[4][4],
                GLuint tex) {
  Require2DMode();
  if (tex == 0) { tex = textures[loadTexture("blank.png")]; }

  const GLfloat data[32] = {
      ver[0][0], ver[0][1], col[0][0], col[0][1], col[0][2], col[0][3], txc[0][0], txc[0][1],
      ver[1][0], ver[1][1], col[1][0], col[1][1], col[1][2], col[1][3], txc[1][0], txc[1][1],
      ver[2][0], ver[2][1], col[2][0], col[2][1], col[2][2], col[2][3], txc[2][0], txc[2][1],
      ver[3][0], ver[3][1], col[3][0], col[3][1], col[3][2], col[3][3], txc[3][0], txc[3][1],
  };

  static GLuint idxs = (GLuint)-1;
  static GLuint buf = (GLuint)-1;
  if (idxs == (GLuint)-1) {
    glGenBuffers(1, &idxs);
    ushort idxdata[6] = {0, 1, 2, 1, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(ushort), idxdata, GL_STATIC_DRAW);

    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);
  } else {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 32 * sizeof(GLfloat), data);
  }

  glBindTexture(GL_TEXTURE_2D, tex);

  // Input structure: 2x Position; 4x color; 2x texture coord
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(2 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(6 * sizeof(GLfloat)));

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0);
}

void drawMousePointer() {
  int mouseX, mouseY;
  sparkle2D->draw();
  SDL_GetMouseState(&mouseX, &mouseY);
  drawMouse(mouseX, mouseY, 64, 64);
}

void drawMouse(int x, int y, int w, int h) {
  GLfloat r1 = 1.0 + 0.1 * cos(mousePointerPhase * 1.8);
  GLfloat r2 = 1.0 + 0.1 * cos(mousePointerPhase * 1.9);
  GLfloat dx = 0.707f * w * r1 * std::sin(mousePointerPhase * 0.35);
  GLfloat dy = 0.707f * h * r2 * std::cos(mousePointerPhase * 0.35);

  GLfloat colors[4][4] = {1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1.};
  GLfloat texco[4][2] = {{0., 0.}, {0., 1.}, {1., 0.}, {1., 1.}};

  GLfloat vco[4][2] = {{x - dx, y - dy}, {x - dy, y + dx}, {x + dy, y - dx}, {x + dx, y + dy}};
  draw2DQuad(vco, texco, colors, textures[loadTexture("mousePointer.png")]);
}

size_t packObjectVertex(void *dest, GLfloat x, GLfloat y, GLfloat z, GLfloat tx, GLfloat ty,
                        const GLfloat color[4], const GLfloat normal[3]) {
  uint32_t *aout = (uint32_t *)dest;
  GLfloat *fout = (GLfloat *)dest;
  fout[0] = x;
  fout[1] = y;
  fout[2] = z;
  aout[3] = (((uint32_t)(65535.f * color[1])) << 16) + (uint32_t)(65535.f * color[0]);
  aout[4] = (((uint32_t)(65535.f * color[3])) << 16) + (uint32_t)(65535.f * color[2]);
  aout[5] = (((uint32_t)(65535.f * ty)) << 16) + (uint32_t)(65535.f * tx);
  aout[6] = packNormal(normal);
  return 8 * sizeof(GLfloat);
}
void configureObjectAttributes() {
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
  glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                        (void *)(3 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                        (void *)(5 * sizeof(GLfloat)));
  glVertexAttribPointer(4, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 8 * sizeof(GLfloat),
                        (void *)(6 * sizeof(GLfloat)));
}

void setupObjectRenderState() {
  glUseProgram(shaderObject);

  glBindVertexArray(theVao);

  // Pos, Color, Tex, ~Vel~, Norm
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(4);

  setViewUniforms(shaderObject);

  glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), 1.);

  glUniform1i(glGetUniformLocation(shaderObject, "tex"), 0);
  glActiveTexture(GL_TEXTURE0 + 0);
}

void countObjectSpherePoints(int *ntriangles, int *nvertices, int detail) {
  if (detail < 1) {
    warning("Sphere detail level must be > 1");
    *ntriangles = 0.;
    *nvertices = 0.;
    return;
  }

  int radial_count = 4 * detail;
  int nrows = 2 * detail + 1;
  *nvertices = (radial_count + 1) * (nrows - 2) + 2;
  *ntriangles = 2 * radial_count * (nrows - 2);
}

void placeObjectSphere(void *data, ushort *idxs, ushort first_index, GLfloat position[3],
                       Matrix3d rotation, GLfloat radius, int detail, GLfloat color[4]) {
  if (detail < 1) {
    warning("Sphere detail level must be > 1. Drawing nothing.");
    return;
  }
  int radial_count = 4 * detail;
  int nrows = 2 * detail + 1;

  // Construct vertices
  char *pos = (char *)data;
  for (int z = 1; z < nrows - 1; z++) {
    GLfloat theta = z * M_PI / (nrows - 1);
    for (int t = 0; t <= radial_count; t++) {
      GLfloat phi = 2 * t * M_PI / radial_count;
      // Texture handling with staggered rows reqs. passing txcoords > 1
      // which packObjectVertex doesn't account for as of writing
      // if (z % 2 == 1) phi += M_PI / radial_count;

      Coord3d local = {std::sin(theta) * std::cos(phi), std::sin(theta) * std::sin(phi),
                       std::cos(theta)};
      Coord3d off;
      useMatrix(rotation, local, off);
      GLfloat foff[3] = {(GLfloat)off[0], (GLfloat)off[1], (GLfloat)off[2]};
      pos += packObjectVertex(pos, position[0] + radius * off[0],
                              position[1] + radius * off[1], position[2] + radius * off[2],
                              phi / (2 * M_PI), theta / M_PI, color, foff);
    }
  }
  Coord3d pnorm = {0., 0., 1.};
  Coord3d vnorm = {0., 0., -1};
  Coord3d poff, voff;
  useMatrix(rotation, pnorm, poff);
  useMatrix(rotation, vnorm, voff);
  GLfloat fpoff[3] = {(GLfloat)poff[0], (GLfloat)poff[1], (GLfloat)poff[2]};
  GLfloat fvoff[3] = {(GLfloat)voff[0], (GLfloat)voff[1], (GLfloat)voff[2]};
  pos += packObjectVertex(pos, position[0] + radius * poff[0], position[1] + radius * poff[1],
                          position[2] + radius * poff[2], 0.5, 0., color, fpoff);
  pos += packObjectVertex(pos, position[0] + radius * voff[0], position[1] + radius * voff[1],
                          position[2] + radius * voff[2], 0.5, 1., color, fvoff);

  // Triangulate end caps
  for (int i = 0; i < radial_count; i++) {
    idxs[6 * i + 0] = first_index + i;
    idxs[6 * i + 1] = first_index + i + 1;
    idxs[6 * i + 2] = first_index + (radial_count + 1) * (nrows - 2);
    idxs[6 * i + 3] = first_index + (radial_count + 1) * (nrows - 3) + i + 1;
    idxs[6 * i + 4] = first_index + (radial_count + 1) * (nrows - 3) + i;
    idxs[6 * i + 5] = first_index + (radial_count + 1) * (nrows - 2) + 1;
  }
  // Triangulate body
  ushort *base = &idxs[2 * 3 * radial_count];
  for (int z = 1; z < nrows - 2; z++) {
    for (int i = 0; i < radial_count; i++) {
      base[6 * i + 0] = first_index + (z - 1) * (radial_count + 1) + i + 1;
      base[6 * i + 1] = first_index + (z - 1) * (radial_count + 1) + i;
      base[6 * i + 2] = first_index + z * (radial_count + 1) + i + 1;
      base[6 * i + 3] = first_index + (z - 1) * (radial_count + 1) + i;
      base[6 * i + 4] = first_index + z * (radial_count + 1) + i;
      base[6 * i + 5] = first_index + z * (radial_count + 1) + i + 1;
    }
    base = &base[6 * radial_count];
  }
}

void perspectiveMatrix(GLdouble fovy_deg, GLdouble aspect, GLdouble zNear, GLdouble zFar,
                       Matrix4d out) {
  GLdouble f = 1. / std::tan(fovy_deg * M_PI / 360.0);
  Matrix4d mat = {{f / aspect, 0., 0., 0.},
                  {0., f, 0., 0.},
                  {0., 0., (zFar + zNear) / (zNear - zFar), -1.},
                  {0., 0., 2 * zFar * zNear / (zNear - zFar), 0.}};
  assign(mat, out);
}

void lookAtMatrix(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX,
                  GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ,
                  Matrix4d out) {
  Coord3d eye = {eyeX, eyeY, eyeZ};
  Coord3d center = {centerX, centerY, centerZ};
  Coord3d up = {upX, upY, upZ};
  Coord3d f, s, u;
  sub(center, eye, f);
  normalize(f);
  normalize(up);
  crossProduct(f, up, s);
  normalize(s);
  crossProduct(s, f, u);

  Matrix4d mat = {{s[0], u[0], -f[0], 0.},
                  {s[1], u[1], -f[1], 0.},
                  {s[2], u[2], -f[2], 0.},
                  {0., 0., 0., 1.}};
  Matrix4d trans = {
      {1., 0., 0., 0.}, {0., 1., 0., 0.}, {0., 0., 1., 0.}, {-eye[0], -eye[1], -eye[2], 1.}};
  matrixMult(trans, mat, out);
}

void setViewUniforms(GLuint shader) {
  GLfloat lproj[16], lmodel[16];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      lproj[4 * i + j] = activeView.projection[i][j];
      lmodel[4 * i + j] = activeView.modelview[i][j];
    }
  }

  glUniformMatrix4fv(glGetUniformLocation(shader, "proj_matrix"), 1, GL_FALSE,
                     (GLfloat *)lproj);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model_matrix"), 1, GL_FALSE,
                     (GLfloat *)lmodel);
  glUniform1i(glGetUniformLocation(shader, "fog_active"), activeView.fog_enabled);
  glUniform3f(glGetUniformLocation(shader, "fog_color"), activeView.fog_color[0],
              activeView.fog_color[1], activeView.fog_color[2]);
  glUniform1f(glGetUniformLocation(shader, "fog_start"), activeView.fog_start);
  glUniform1f(glGetUniformLocation(shader, "fog_end"), activeView.fog_end);

  glUniform3f(glGetUniformLocation(shader, "light_position"),
              (GLfloat)activeView.light_position[0], (GLfloat)activeView.light_position[1],
              (GLfloat)activeView.light_position[2]);
  glUniform3f(glGetUniformLocation(shader, "light_ambient"), activeView.light_ambient[0],
              activeView.light_ambient[1], activeView.light_ambient[2]);
  glUniform3f(glGetUniformLocation(shader, "light_diffuse"), activeView.light_diffuse[0],
              activeView.light_diffuse[1], activeView.light_diffuse[2]);
  glUniform3f(glGetUniformLocation(shader, "light_specular"), activeView.light_specular[0],
              activeView.light_specular[1], activeView.light_specular[2]);
  glUniform3f(glGetUniformLocation(shader, "global_ambient"), activeView.global_ambient[0],
              activeView.global_ambient[1], activeView.global_ambient[2]);
  glUniform1f(glGetUniformLocation(shader, "quadratic_attenuation"),
              activeView.quadratic_attenuation);
}

/* generates a snapshot of the screen */
int createSnapshot() {
  static int snap_number = 0;
  unsigned char *buffer = NULL;
  char name[1024];
  int again = 1, i, j;
  FILE *f;

  /* allocate buffer */
  if ((buffer = (unsigned char *)malloc(sizeof(unsigned char) * screenWidth * screenHeight *
                                        3)) == NULL) {
    warning("cannot allocate %lu bytes for snapshot. Aborting.",
            sizeof(unsigned char) * screenWidth * screenHeight * 3);
    return (0);
  }

  /* find the name for the image */
  do {
    snprintf(name, 1023, "./snapshot_%04d.ppm", snap_number++);
    if ((f = fopen(name, "r")) == NULL) {
      again = 0;
    } else {
      fclose(f);
    }
  } while (again);

  /* Check against symlinks */
  if (pathIsLink(name)) {
    warning("file %s is a symlink.", name);
    return 0;
  }

  /* get the screen */
  glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)buffer);

  /* save the image */
  if ((f = fopen(name, "w")) == NULL) {
    warning("cannot create file '%s'. Abort", name);
    free(buffer);
    return (0);
  }

  fprintf(f, "P6\n");
  fprintf(f, "%d %d\n255\n", screenWidth, screenHeight);

  /* write data */
  for (j = screenHeight - 1; j >= 0; j--) {
    for (i = 0; i < screenWidth; i++) {
      fprintf(f, "%c%c%c", buffer[(i + j * screenWidth) * 3 + 0],
              buffer[(i + j * screenWidth) * 3 + 1], buffer[(i + j * screenWidth) * 3 + 2]);
    }
  }

  /* freed everything */
  fclose(f);
  free(buffer);
  return 1;
}

/* Displays a centered semi-transparent sign with two text rows */
void message(char *A, char *B) {
  int size = 16;
  int w1, w2, h1, h2, w;
  w1 = Font::getTextWidth(A, size);
  w2 = Font::getTextWidth(B, size);
  h1 = 32;
  h2 = 32;

  w = std::max(w1, w2) + 20;

  int x1 = screenWidth / 2 - w / 2, x2 = screenWidth / 2 + w / 2;
  int y1 = screenHeight / 2 - h1 - 5, y2 = screenHeight / 2 + h2 + 5;
  Enter2DMode();
  draw2DRectangle(x1, y1, x2 - x1, y2 - y1, 0., 0., 1., 1., 0.2, 0.5, 0.2, 0.5);

  Font::drawCenterSimpleText(A, screenWidth / 2 + size, screenHeight / 2 - size, size, 0.5,
                             1.0, 0.2, 1.0);
  Font::drawCenterSimpleText(B, screenWidth / 2 + size, screenHeight / 2 + 14, size, 0.5, 1.0,
                             0.2, 1.0);

  Leave2DMode();
}

void multiMessage(int nlines, const char *left[], const char *right[]) {
  int total_height, width, h_now;
  int size = 16;

  total_height = 0;
  for (int i = 0; i < nlines; i++) { total_height += size * 2; }
  width = 600;

  int x1 = screenWidth / 2 - width / 2 - 5, x2 = screenWidth / 2 + width / 2 + 5;
  int y1 = screenHeight / 2 - total_height / 2 - 5,
      y2 = screenHeight / 2 + total_height / 2 + 30;

  Enter2DMode();
  draw2DRectangle(x1, y1, x2 - x1, y2 - y1, 0., 0., 1., 1., 0.2, 0.5, 0.2, 0.5);

  h_now = -size;
  for (int i = 0; i < nlines; i++) {
    h_now += 2 * size;
    if (left[i]) {
      Font::drawSimpleText(left[i], screenWidth / 2 - width / 2 + size,
                           screenHeight / 2 - total_height / 2 + h_now, size, 0.5, 1.0, 0.2,
                           1.0);
    }
    if (right[i]) {
      Font::drawRightSimpleText(right[i], screenWidth / 2 + width / 2 + size,
                                screenHeight / 2 - total_height / 2 + h_now, size, 0.5, 1.0,
                                0.2, 1.0);
    }
  }
  Leave2DMode();
}

/** Loads a texture from file and returns a reference to it.
    It is safe to load the same texture multiple times since the results are cached
*/
int loadTexture(const char *name) {
  GLfloat texCoord[4];
  char str[256];
  SDL_Surface *surface;

  /* Check in cache if texture already loaded */
  for (int i = 0; i < numTextures; i++)
    if (strcmp(textureNames[i], name) == 0) return i;

  if (numTextures >= GLHELP_MAX_TEXTURES) {
    warning("Warning: max. number of textures reached (%d). Texture '%s' not loaded.",
            GLHELP_MAX_TEXTURES, name);
    return 0;
  }

  snprintf(str, sizeof(str), "%s/images/%s", effectiveShareDir, name);
  surface = IMG_Load(str);
  if (!surface) {
    warning("Failed to load texture %s", str);
    // Override texture name... (to avoid carrying on outdated tex names)
    textureNames[numTextures] = strdup(textureNames[0]);
    textures[numTextures++] =
        textures[0];  // just assume we managed to load this, better than nothing
    return -1;
  } else {
    textureNames[numTextures] = strdup(name);
    textures[numTextures] =
        LoadTexture(surface, texCoord, 1, NULL);  // linear filter was: font != NULL
    /*printf("loaded texture[%d]=%d\n",numTextures,textures[numTextures]);*/
    numTextures++;
    SDL_FreeSurface(surface);
  }
  return (numTextures - 1);  // ok
}

void glHelpInit() {
  for (int i = 0; i < 4711; i++) fake_rand[i] = frand();

  TTF_Init();
  char str[256];
  snprintf(str, sizeof(str), "%s/fonts/%s", effectiveShareDir, "menuFont.ttf");
  ingameFont = TTF_OpenFont(str, 30);
  if (!ingameFont) { error("failed to load font %s", str); }

  /* Note: all textures must be powers of 2 since we ignore texcoords */
  loadTexture("ice.png");
  loadTexture("acid.png");
  loadTexture("sand.png");
  loadTexture("track.png");
  loadTexture("texture.png");
  loadTexture("texture2.png");
  loadTexture("texture3.png");
  loadTexture("texture4.png");
  loadTexture("wings.png");
  loadTexture("mousePointer.png");

  sparkle2D = new Sparkle2D();

  // The VAO need only be there :-)
  glGenVertexArrays(1, &theVao);

  // Errors handled within loadProgram
  shaderTile = loadProgram("basic.vert", "basic.frag");
  shaderLine = loadProgram("line.vert", "line.frag");
  shaderWater = loadProgram("water.vert", "water.frag");
  shaderUI = loadProgram("ui.vert", "ui.frag");
  shaderObject = loadProgram("object.vert", "object.frag");
  shaderReflection = loadProgram("reflection.vert", "reflection.frag");

  // Setup view state
  activeView.fog_enabled = 0;
  activeView.fog_color[0] = 1.f;
  activeView.fog_color[1] = 1.f;
  activeView.fog_color[2] = 1.f;
  activeView.fog_start = 10.f;
  activeView.fog_end = 20.f;

  activeView.light_position[0] = 0.;
  activeView.light_position[1] = 0.;
  activeView.light_position[2] = 10.;
  activeView.light_ambient[0] = 0.2f;
  activeView.light_ambient[1] = 0.2f;
  activeView.light_ambient[2] = 0.2f;
  activeView.light_diffuse[0] = 1.f;
  activeView.light_diffuse[1] = 1.f;
  activeView.light_diffuse[2] = 1.f;
  activeView.light_specular[0] = 0.5f;
  activeView.light_specular[1] = 0.5f;
  activeView.light_specular[2] = 0.5f;
  activeView.global_ambient[0] = 0.f;
  activeView.global_ambient[1] = 0.f;
  activeView.global_ambient[2] = 0.f;
  activeView.quadratic_attenuation = 0.f;

  glEnable(GL_TEXTURE_2D);
}
void glHelpCleanup() {
  if (shaderTile) glDeleteProgram(shaderTile);
  if (shaderLine) glDeleteProgram(shaderLine);
  if (shaderWater) glDeleteProgram(shaderWater);
  if (shaderUI) glDeleteProgram(shaderUI);
  if (shaderObject) glDeleteProgram(shaderObject);
  if (shaderReflection) glDeleteProgram(shaderReflection);
  if (theVao) glDeleteVertexArrays(1, &theVao);
  shaderLine = 0;
  shaderTile = 0;
  shaderWater = 0;
  shaderUI = 0;
  shaderObject = 0;
  theVao = 0;

  if (sparkle2D) delete sparkle2D;

  /* Invalidate all strings so they get cleaned up */
  stringTick += 100000;
  update2DStringCache();

  for (std::map<int, TTF_Font *>::iterator i = menuFontLookup.begin();
       i != menuFontLookup.end(); ++i) {
    TTF_CloseFont(i->second);
  }
  menuFontLookup.clear();
  TTF_CloseFont(ingameFont);
  ingameFont = 0;

  for (int i = 0; i < numTextures; i++) { glDeleteTextures(1, &textures[i]); }
}

char *filetobuf(const char *filename) {
  FILE *fptr = fopen(filename, "rb");
  if (!fptr) return NULL;
  fseek(fptr, 0, SEEK_END);
  long length = ftell(fptr);
  char *buf = (char *)malloc(length + 1);
  fseek(fptr, 0, SEEK_SET);
  fread(buf, length, 1, fptr);
  fclose(fptr);
  buf[length] = 0;
  return buf;
}

GLuint loadProgram(const char *vertname, const char *fragname) {
  /* Read our shaders into the appropriate buffers */
  char path[256];
  snprintf(path, 256, "%s/shaders/%s", effectiveShareDir, vertname);
  GLchar *vertexsource = filetobuf(path);
  if (vertexsource == NULL) { error("Vertex shader %s could not be read", path); }
  snprintf(path, 256, "%s/shaders/%s", effectiveShareDir, fragname);
  GLchar *fragmentsource = filetobuf(path);
  if (fragmentsource == NULL) { error("Fragment shader %s could not be read", path); }
  GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
  int maxLength;
  glShaderSource(vertexshader, 1, (const GLchar **)&vertexsource, 0);
  glCompileShader(vertexshader);
  int IsCompiled_VS, IsCompiled_FS, IsLinked;
  glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
  if (IsCompiled_VS == 0) {
    glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);
    char *vertexInfoLog = (char *)malloc(maxLength);
    glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);
    warning("Vertex shader %s error: %s", vertname, vertexInfoLog);
    glDeleteShader(vertexshader);
    free(vertexInfoLog);
    free(vertexsource);
    free(fragmentsource);
    return -1;
  }
  GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentshader, 1, (const GLchar **)&fragmentsource, 0);
  glCompileShader(fragmentshader);
  glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
  if (IsCompiled_FS == 0) {
    glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);
    char *fragmentInfoLog = (char *)malloc(maxLength);
    glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);
    warning("Fragment shader %s error: %s", fragname, fragmentInfoLog);
    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    free(fragmentInfoLog);
    free(vertexsource);
    free(fragmentsource);
    return -1;
  }
  free(vertexsource);
  free(fragmentsource);
  GLuint shaderprogram = glCreateProgram();
  glAttachShader(shaderprogram, vertexshader);
  glAttachShader(shaderprogram, fragmentshader);
  glBindAttribLocation(shaderprogram, 0, "in_Position");
  glBindAttribLocation(shaderprogram, 1, "in_Color");
  glBindAttribLocation(shaderprogram, 2, "in_Texcoord");
  glBindAttribLocation(shaderprogram, 3, "in_Velocity");
  glBindAttribLocation(shaderprogram, 4, "in_Normal");
  glBindAttribLocation(shaderprogram, 5, "in_Specular");
  glLinkProgram(shaderprogram);
  glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
  glDeleteShader(vertexshader);
  glDeleteShader(fragmentshader);
  if (IsLinked == 0) {
    glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);
    char *shaderProgramInfoLog = (char *)malloc(maxLength);
    glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);
    warning("Program (%s %s) link error: %s", vertname, fragname, shaderProgramInfoLog);
    free(shaderProgramInfoLog);
    return -1;
  }
  return shaderprogram;
}

void warnForGLerrors(const char *where_am_i) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    warning("GL error %x at location: %s", err, where_am_i);
  }
}

#define FRAME 50

int resetTextures() {
  GLfloat texCoord[4];
  char str[256];
  SDL_Surface *surface;
  int linear = 0;

  for (int i = 0; i < numTextures; i++) {
    if (textureNames[i] == NULL) continue;
    snprintf(str, sizeof(str), "%s/images/%s", effectiveShareDir, textureNames[i]);
    surface = IMG_Load(str);
    if (!surface) {
      warning("Failed to load texture %s", str);
      return 0;  // error (a valid texture entry)
    } else {
      glDeleteTextures(1, &textures[i]);
      textures[i] = LoadTexture(surface, texCoord, linear, NULL);
      SDL_FreeSurface(surface);
    }
  }
  return 1;
}

/* Calculates and displays current framerate */
void displayFrameRate() {
  static double oldTime = -0.1;
  double t, td;
  char str[256];

  t = ((double)SDL_GetTicks()) / 1000.0;
  td = t - oldTime;
  if (td > 1.0)
    fps = 1.0;
  else if (td <= 1e-4) {
    static int hasWarned = 0;
    if (!hasWarned) {
      warning("Warning: too fast framerate (%f)", td);
      hasWarned = 1;
    }
  } else
    fps = fps * 0.95 + 0.05 / td;
  oldTime = t;

  if (Settings::settings->showFPS > 0) {
    if (Settings::settings->showFPS == 1) {
      if (fps > 0)
        snprintf(str, sizeof(str), _("Framerate: %.1f"), fps);
      else
        snprintf(str, sizeof(str), _("Framerate unknown"));
    } else {
      snprintf(str, sizeof(str), _("%.1f ms/Frame"),
               1e3 * (getSystemTime() - displayStartTime));
    }
    Font::drawSimpleText(str, 15, screenHeight - 15, 10., 1., 1., 0.25, 0.8);
  }
}

/********************/
/* Vector operations */
/********************/

#ifndef INLINE_VECTOR_OPS
/* C <- A + B */
void add(const double A[3], const double B[3], double C[3]) {
  for (int i = 0; i < 3; i++) C[i] = A[i] + B[i];
}

/* C <- A - B */
void sub(const double A[3], const double B[3], double C[3]) {
  for (int i = 0; i < 3; i++) C[i] = A[i] - B[i];
}
/* C <- C * 1 / |C| */
void normalize(double C[3]) {
  double l = sqrt(C[0] * C[0] + C[1] * C[1] + C[2] * C[2]);
  C[0] /= l;
  C[1] /= l;
  C[2] /= l;
}
/* |A| */
double length(double A[3]) { return sqrt(A[0] * A[0] + A[1] * A[1] + A[2] * A[2]); }
/* C <- A x B */
void crossProduct(const double A[3], const double B[3], double C[3]) {
  C[0] = A[1] * B[2] - A[2] * B[1];
  C[1] = A[2] * B[0] - A[0] * B[2];
  C[2] = A[0] * B[1] - A[1] * B[0];
}

/* <- A . B */
double dotProduct(const double A[3], const double B[3]) {
  return A[0] * B[0] + A[1] * B[1] + A[2] * B[2];
}
#endif

/*********************/
/* Matrix operations */
/*********************/

void debugMatrix(Matrix4d m) {
  printf("%f \t%f \t%f \t%f\n", m[0][0], m[0][1], m[0][2], m[0][3]);
  printf("%f \t%f \t%f \t%f\n", m[1][0], m[1][1], m[1][2], m[1][3]);
  printf("%f \t%f \t%f \t%f\n", m[2][0], m[2][1], m[2][2], m[2][3]);
  printf("%f \t%f \t%f \t%f\n", m[3][0], m[3][1], m[3][2], m[3][3]);
}

/* C <- A * B */
void matrixMult(const Matrix4d A, const Matrix4d B, Matrix4d C) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      C[i][j] = 0.0;
      for (int k = 0; k < 4; k++) C[i][j] += A[i][k] * B[k][j];
    }
}

/* C <- A(B) */
void useMatrix(Matrix4d A, const double B[3], double C[3]) {
  for (int i = 0; i < 3; i++) {
    C[i] = A[i][3];
    for (int k = 0; k < 3; k++) C[i] += A[i][k] * B[k];
  }
  double h = A[3][3];
  for (int k = 0; k < 3; k++) h += A[3][k];
  for (int k = 0; k < 3; k++) C[k] /= h;
}

/* C <- A(B) */
void useMatrix(Matrix3d A, const double B[3], double C[3]) {
  for (int i = 0; i < 3; i++) {
    C[i] = 0.;
    for (int k = 0; k < 3; k++) C[i] += A[i][k] * B[k];
  }
}

/* C <- A */
void assign(const Matrix4d A, Matrix4d C) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) C[i][j] = A[i][j];
}

/* C <- A */
void transpose(const Matrix4d A, Matrix4d C) {
  Matrix4d B;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) B[j][i] = A[i][j];
  assign(C, B);
}

/* C <- A */
void assign(const double A[3], double C[3]) {
  C[0] = A[0];
  C[1] = A[1];
  C[2] = A[2];
}

/* C <- A */
void assign(const float A[3], float C[3]) {
  C[0] = A[0];
  C[1] = A[1];
  C[2] = A[2];
}

void identityMatrix(Matrix4d m) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) m[i][j] = i == j ? 1.0 : 0.0;
}

void rotateX(double v, Matrix4d m) {
  Matrix4d mr = {{1.0, 0.0, 0.0, 0.0},
                 {0.0, cos(v), sin(v), 0.0},
                 {0.0, -sin(v), cos(v), 0.0},
                 {0.0, 0.0, 0.0, 1.0}};
  Matrix4d morig;
  assign(m, morig);
  matrixMult(morig, mr, m);
}
void rotateY(double v, Matrix4d m) {
  Matrix4d mr = {{cos(v), 0.0, sin(v), 0.0},
                 {0.0, 1.0, 0.0, 0.0},
                 {-sin(v), 0.0, cos(v), 0.0},
                 {0.0, 0.0, 0.0, 1.0}};
  Matrix4d morig;
  assign(m, morig);
  matrixMult(morig, mr, m);
}
void rotateZ(double v, Matrix4d m) {
  Matrix4d mr = {{cos(v), sin(v), 0.0, 0.0},
                 {-sin(v), cos(v), 0.0, 0.0},
                 {0.0, 0.0, 1.0, 0.0},
                 {0.0, 0.0, 0.0, 1.0}};
  Matrix4d morig;
  assign(m, morig);
  matrixMult(morig, mr, m);
}
void zero(double v[3]) { v[0] = v[1] = v[2] = 0.0; }

int testBboxClip(double x1, double x2, double y1, double y2, double z1, double z2,
                 const Matrix4d model, const Matrix4d proj) {
  // Construct axis-aligned bounding box in window space
  double vxl = 1e99, vyl = 1e99, vzl = 1e99;
  double vxh = -1e99, vyh = -1e99, vzh = -1e99;
  Matrix4d mvp;
  Matrix4d mmod, mproj;
  memcpy(&mmod[0][0], model, sizeof(Matrix4d));
  memcpy(&mproj[0][0], proj, sizeof(Matrix4d));
  matrixMult(mmod, mproj, mvp);
  for (int i = 0; i < 8; i++) {
    double w[3] = {0., 0., 0.};
    double p[3] = {i & 4 ? x1 : x2, i & 2 ? y1 : y2, i & 1 ? z1 : z2};
    // Apply perspective transform with MVP matrix
    for (int k = 0; k < 3; k++)
      w[k] += mvp[0][k] * p[0] + mvp[1][k] * p[1] + mvp[2][k] * p[2] + mvp[3][k];
    double h = mvp[0][3] * p[0] + mvp[1][3] * p[1] + mvp[2][3] * p[2] + mvp[3][3];
    for (int k = 0; k < 3; k++) w[k] /= h;

    vxl = std::min(vxl, w[0]);
    vyl = std::min(vyl, w[1]);
    vzl = std::min(vzl, w[2]);

    vxh = std::max(vxh, w[0]);
    vyh = std::max(vyh, w[1]);
    vzh = std::max(vzh, w[2]);
  }
  // Window frustum is [-1,1]x[-1,1]x[0,1]
  if (vxl > 1 || vxh < -1) return 0;
  if (vyl > 1 || vyh < -1) return 0;
  if (vzl > 1 || vzh < 0) return 0;
  return 1;
}

static int is_2d_mode = 0;

void Require2DMode() {
  if (!is_2d_mode) { warning("Function should only be called in 2D mode"); }
}

void Enter2DMode() {
  if (is_2d_mode) { return; }

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Load program, bind VAO, and set up uniforms
  glUseProgram(shaderUI);

  glBindVertexArray(theVao);

  glUniform1f(glGetUniformLocation(shaderUI, "screen_width"), (GLfloat)screenWidth);
  glUniform1f(glGetUniformLocation(shaderUI, "screen_height"), (GLfloat)screenHeight);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glUniform1i(glGetUniformLocation(shaderUI, "tex"), 0);
  glActiveTexture(GL_TEXTURE0 + 0);

  is_2d_mode = 1;
}

void Leave2DMode() {
  if (!is_2d_mode) { return; }

  is_2d_mode = 0;
}

int powerOfTwo(int input) {
  int value = 1;

  while (value < input) { value <<= 1; }
  return value;
}

GLuint LoadTexture(SDL_Surface *surface, GLfloat *texcoord, int linearFilter,
                   GLuint *texture) {
  int w, h;
  SDL_Surface *image;
  SDL_Rect area;

  /* Use the surface width and height expanded to powers of 2 */
  w = powerOfTwo(surface->w);
  h = powerOfTwo(surface->h);

  /* Rescale image if needed? */
  GLint maxSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
  double scale = 1.0;
  if (w > maxSize || h > maxSize) scale = std::min(maxSize / (double)w, maxSize / (double)h);

  texcoord[0] = 0.0f;                    /* Min X */
  texcoord[1] = 0.0f;                    /* Min Y */
  texcoord[2] = (GLfloat)surface->w / w; /* Max X */
  texcoord[3] = (GLfloat)surface->h / h; /* Max Y */

  Uint32 mask[4];
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
  mask[0] = 0x000000FF;
  mask[1] = 0x0000FF00;
  mask[2] = 0x00FF0000;
  mask[3] = 0xFF000000;
#else
  mask[0] = 0xFF000000;
  mask[1] = 0x00FF0000;
  mask[2] = 0x0000FF00;
  mask[3] = 0x000000FF;
#endif

  image = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)(w * scale), (int)(h * scale), 32, mask[0],
                               mask[1], mask[2], mask[3]);

  if (image == NULL) { return 0; }

  /* Copy the surface into the GL texture image */
  area.x = 0;
  area.y = 0;
  area.w = (int)(surface->w * scale);
  area.h = (int)(surface->h * scale);

  if (scale == 1.0)
    // Easy, no scaling needed
    SDL_BlitSurface(surface, NULL, image, &area);
  else {
    // Scaling needed
    // This is a realy inefficient and unoptimised way to do it. Sorry!
    static int hasWarned = 0;
    if (!hasWarned) {
      warning("Rescaling images before loading them as textures.");
      hasWarned = 1;
    }
    SDL_Surface *source = SDL_ConvertSurface(surface, image->format, SDL_SWSURFACE);
    SDL_LockSurface(source);
    SDL_LockSurface(image);
    for (int x = 0; x < area.w; x++)
      for (int y = 0; y < area.h; y++)
        *((Uint32 *)(((char *)image->pixels) + x * image->format->BytesPerPixel +
                     y * image->pitch)) =
            *((Uint32 *)(((char *)source->pixels) +
                         ((int)(x / scale)) * source->format->BytesPerPixel +
                         ((int)(y / scale)) * source->pitch));
    SDL_UnlockSurface(source);
    SDL_UnlockSurface(image);
    SDL_FreeSurface(source);
  }

  /* Create an OpenGL texture for the image */
  GLuint freshTexture = 0;
  if (texture == NULL) texture = &freshTexture;
  if (*texture == 0) {
    glGenTextures(1, texture);
    /*printf("texture: %d created\n",*texture); */
  }

  glBindTexture(GL_TEXTURE_2D, *texture);
  if (linearFilter) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)(w * scale), (int)(h * scale), 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image->pixels);

  SDL_FreeSurface(image); /* No longer needed */
  return *texture;
}

SDL_Surface *loadImage(const char *imagename) {
  char path[512];
  snprintf(path, 511, "%s/images/%s", effectiveShareDir, imagename);
  path[511] = '\0';

  SDL_Surface *img = IMG_Load(path);
  if (!img) error("Failed to load image '%s' because: %s", path, IMG_GetError());
  return img;
}
