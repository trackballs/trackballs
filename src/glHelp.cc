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

#include "general.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_opengl.h"
#include "settings.h"
#include "font.h"
#include "SDL2/SDL_image.h"
#include "sparkle2d.h"
#include <map>

using namespace std;

float fps = 50.0;
float realTimeNow = 0.0;
int screenWidth = 640, screenHeight = 480;
const GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
const GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};
GLUquadricObj *qball;

/* Precompiled display lists for spheres of varying resolution (dependent on gfx settings) */
GLuint sphereDisplayLists[3];

GLuint shaderWater = 0;
GLuint shaderTile = 0;
GLuint shaderTileRim = 0;
GLuint shaderUI = 0;
GLuint shaderObject = 0;
GLuint theVao = 0;

TTF_Font *msgFont, *infoFont, *ingameFont, *menuFont, *scrollFont;
extern SDL_Surface *screen;
extern double displayStartTime;

double fake_rand[4711];
double frand(int i) { return fake_rand[i % 4711]; }
double frand() { return (rand() % (1 << 30)) / ((double)(1 << 30)); }

#define GLHELP_MAX_TEXTURES 256
GLuint textures[GLHELP_MAX_TEXTURES] = {0};  // added init. to 0 (no texture)
char *textureNames[GLHELP_MAX_TEXTURES] = {NULL};
int numTextures;

Sparkle2D *sparkle2D = NULL;
/*
SDL_Surface *mousePointer;
GLuint mousePointerTexture;
*/

struct StringInfo {
  TTF_Font *font;
  char string[256];
  int x, y;
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

void draw2DString(TTF_Font *font, const char *string, int x, int y, float red, float green,
                  float blue, float alpha, int outlined) {
  struct StringInfo inf;
  inf.color.r = 255 * red;
  inf.color.g = 255 * green;
  inf.color.b = 255 * blue;
  inf.color.a = 255 * alpha;
  inf.font = font;
  inf.x = x;
  inf.y = y;
  strncpy(inf.string, string, 255);
  inf.string[255] = '\0';
  if (strcache.count(inf) <= 0) {
    struct StringCache newentry;
    newentry.tick = stringTick;
    SDL_Surface *surf = drawStringToSurface(inf, outlined);
    if (!surf) { return; }
    newentry.texture = LoadTexture(surf, newentry.texcoord, 1, NULL);
    newentry.w = surf->w;
    newentry.h = surf->h;
    SDL_FreeSurface(surf);
    strcache[inf] = newentry;
  }

  struct StringCache &cached = strcache[inf];
  cached.tick = stringTick;

  y -= cached.h / 2;

  draw2DRectangle(x, y, cached.w, cached.h, cached.texcoord[0], cached.texcoord[1],
                  cached.texcoord[2], cached.texcoord[3], 1., 1., 1., 1., cached.texture);
}

void update2DStringCache() {
  stringTick++;
  int erased;
  do {
    erased = false;
    for (std::map<StringInfo, StringCache>::iterator i = strcache.begin(); i != strcache.end();
         ++i) {
      if (i->second.tick < stringTick - 3) {
        glDeleteTextures(1, &i->second.texture);
        strcache.erase(i);
        erased = true;
        break;
      }
    }
  } while (erased);
}

TTF_Font *menuFontForSize(int sz) {
  static std::map<int, TTF_Font *> lookup;
  if (!lookup.count(sz)) {
    char str[256];
    snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "FreeSerifBoldItalic.ttf");
    lookup[sz] = TTF_OpenFont(str, 2 * sz);  // barbatri
    if (!lookup[sz]) { error("failed to load font %s", str); }
  }
  return lookup[sz];
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

  static GLuint idxs = (GLuint)-1;
  if (idxs == (GLuint)-1) {
    glGenBuffers(1, &idxs);
    ushort idxdata[6] = {0, 1, 2, 1, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(ushort), idxdata, GL_STATIC_DRAW);
  }

  const GLfloat data[32] = {
      ver[0][0], ver[0][1], col[0][0], col[0][1], col[0][2], col[0][3], txc[0][0], txc[0][1],
      ver[1][0], ver[1][1], col[1][0], col[1][1], col[1][2], col[1][3], txc[1][0], txc[1][1],
      ver[2][0], ver[2][1], col[2][0], col[2][1], col[2][2], col[2][3], txc[2][0], txc[2][1],
      ver[3][0], ver[3][1], col[3][0], col[3][1], col[3][2], col[3][3], txc[3][0], txc[3][1],
  };

  // May want to convert to streaming by creating oversize buffer and rotating through
  GLuint buf;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(GLfloat), data, GL_STATIC_DRAW);

  glBindTexture(GL_TEXTURE_2D, tex);

  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);

  // Input structure: 2x Position; 4x color; 2x texture coord
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(2 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(6 * sizeof(GLfloat)));

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0);

  glDeleteBuffers(1, &buf);
}

void drawMousePointer() {
  int mouseX, mouseY;
  sparkle2D->draw();
  SDL_GetMouseState(&mouseX, &mouseY);
  drawMouse(mouseX, mouseY, 64, 64, 0.01);
}

void drawMouse(int x, int y, int w, int h, Real td) {
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
                        GLfloat color[3], GLfloat normal[3]) {
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

  GLfloat proj[16];
  GLfloat model[16];

  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  glGetFloatv(GL_MODELVIEW_MATRIX, model);

  glUniformMatrix4fv(glGetUniformLocation(shaderObject, "proj_matrix"), 1, GL_FALSE,
                     (GLfloat *)&proj[0]);
  glUniformMatrix4fv(glGetUniformLocation(shaderObject, "model_matrix"), 1, GL_FALSE,
                     (GLfloat *)&model[0]);
  glUniform1f(glGetUniformLocation(shaderObject, "use_lighting"), 1.);

  glUniform1i(glGetUniformLocation(shaderObject, "tex"), 0);
  glActiveTexture(GL_TEXTURE0 + 0);
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

  w = max(w1, w2) + 20;

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
  int i;
  int size = 16;

  total_height = 0;
  for (i = 0; i < nlines; i++) { total_height += size * 2; }
  width = 600;

  int x1 = screenWidth / 2 - width / 2 - 5, x2 = screenWidth / 2 + width / 2 + 5;
  int y1 = screenHeight / 2 - total_height / 2 - 5,
      y2 = screenHeight / 2 + total_height / 2 + 30;

  Enter2DMode();
  draw2DRectangle(x1, y1, x2 - x1, y2 - y1, 0., 0., 1., 1., 0.2, 0.5, 0.2, 0.5);

  h_now = -size;
  for (i = 0; i < nlines; i++) {
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
  int i;

  /* Check in cache if texture already loaded */
  for (i = 0; i < numTextures; i++)
    if (strcmp(textureNames[i], name) == 0) return i;

  if (numTextures >= GLHELP_MAX_TEXTURES) {
    warning("Warning: max. number of textures reached (%d). Texture '%s' not loaded.",
            GLHELP_MAX_TEXTURES, name);
    return 0;
  }

  snprintf(str, sizeof(str), "%s/images/%s", SHARE_DIR, name);
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
  char str[256];

  for (int i = 0; i < 4711; i++) fake_rand[i] = frand();

  TTF_Init();
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  msgFont = TTF_OpenFont(str, 30);  // 30 astron
  if (!msgFont) { error("failed to load font %s", str); }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  infoFont = TTF_OpenFont(str, 18);
  if (!infoFont) { error("failed to load font %s", str); }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  ingameFont = TTF_OpenFont(str, 30);  // barbatri
  if (!ingameFont) { error("failed to load font %s", str); }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "FreeSerifBoldItalic.ttf");
  menuFont = TTF_OpenFont(str, 40);  // barbatri
  if (!menuFont) { error("failed to load font %s", str); }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  scrollFont = TTF_OpenFont(str, 18);
  if (!(msgFont && infoFont && ingameFont)) { error("Error: failed to load fonts"); }
  TTF_SetFontStyle(msgFont, TTF_STYLE_NORMAL);

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

  qball = gluNewQuadric();
  gluQuadricDrawStyle(qball, GLU_FILL);
  gluQuadricNormals(qball, GLU_SMOOTH);
  gluQuadricTexture(qball, GL_TRUE);

  sphereDisplayLists[0] = glGenLists(1);
  sphereDisplayLists[1] = glGenLists(1);
  sphereDisplayLists[2] = glGenLists(1);
  regenerateSphereDisplaylists();

  // The VAO need only be there :-)
  glGenVertexArrays(1, &theVao);

  // Errors handled within loadProgram
  shaderTile = loadProgram("basic.vert", "basic.frag");
  shaderTileRim = loadProgram("line.vert", "line.frag");
  shaderWater = loadProgram("water.vert", "water.frag");
  shaderUI = loadProgram("ui.vert", "ui.frag");
  shaderObject = loadProgram("object.vert", "object.frag");
}

void regenerateSphereDisplaylists() {
  int resolution = 6;

  glNewList(sphereDisplayLists[0], GL_COMPILE);
  switch (Settings::settings->gfx_details) {
  case GFX_DETAILS_NONE:
    resolution = 6;
    break;
  case GFX_DETAILS_MINIMALISTIC:
    resolution = 8;
    break;
  case GFX_DETAILS_SIMPLE:
    resolution = 8;
    break;
  case GFX_DETAILS_NORMAL:
    resolution = 8;
    break;
  case GFX_DETAILS_EXTRA:
    resolution = 10;
    break;
  case GFX_DETAILS_EVERYTHING:
    resolution = 12;
    break;
  }
  gluSphere(qball, 1.0, resolution, resolution / 2);
  glEndList();

  glNewList(sphereDisplayLists[1], GL_COMPILE);
  switch (Settings::settings->gfx_details) {
  case GFX_DETAILS_NONE:
    resolution = 6;
    break;
  case GFX_DETAILS_MINIMALISTIC:
    resolution = 8;
    break;
  case GFX_DETAILS_SIMPLE:
    resolution = 10;
    break;
  case GFX_DETAILS_NORMAL:
    resolution = 12;
    break;
  case GFX_DETAILS_EXTRA:
    resolution = 16;
    break;
  case GFX_DETAILS_EVERYTHING:
    resolution = 20;
    break;
  }
  gluSphere(qball, 1.0, resolution, resolution / 2);
  glEndList();

  glNewList(sphereDisplayLists[2], GL_COMPILE);
  switch (Settings::settings->gfx_details) {
  case GFX_DETAILS_NONE:
    resolution = 8;
    break;
  case GFX_DETAILS_MINIMALISTIC:
    resolution = 10;
    break;
  case GFX_DETAILS_SIMPLE:
    resolution = 12;
    break;
  case GFX_DETAILS_NORMAL:
    resolution = 16;
    break;
  case GFX_DETAILS_EXTRA:
    resolution = 20;
    break;
  case GFX_DETAILS_EVERYTHING:
    resolution = 24;
    break;
  }
  gluSphere(qball, 1.0, resolution, resolution / 2);
  glEndList();
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

#define FRAME 50

int bindTexture(const char *name) {
  int i;
  for (i = 0; i < numTextures; i++)
    if (strcmp(name, textureNames[i]) == 0) {
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      return i;
    }
  // YP: if the texture is not found, try to load it
  //     and add it in the list
  i = loadTexture(name);
  if (i >= 0) glBindTexture(GL_TEXTURE_2D, textures[i]);
  return i;
}

int resetTextures() {
  GLfloat texCoord[4];
  char str[256];
  SDL_Surface *surface;
  int i;
  int linear = 0;

  for (i = 0; i < numTextures; i++) {
    if (textureNames[i] == NULL) continue;
    snprintf(str, sizeof(str), "%s/images/%s", SHARE_DIR, textureNames[i]);
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
  static double oldTime = 0.0;
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
  realTimeNow = t;

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
  int i, j, k;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) {
      C[i][j] = 0.0;
      for (k = 0; k < 4; k++) C[i][j] += A[i][k] * B[k][j];
    }
}

/* C <- A(B) */
void useMatrix(Matrix4d A, const double B[3], double C[3]) {
  int i, k;
  for (i = 0; i < 3; i++) {
    C[i] = A[i][3];
    for (k = 0; k < 3; k++) C[i] += A[i][k] * B[k];
  }
  double h = A[3][3];
  for (k = 0; k < 3; k++) h += A[3][k];
  for (k = 0; k < 3; k++) C[k] /= h;
}

/* C <- A(B) */
void useMatrix(Matrix3d A, const double B[3], double C[3]) {
  int i, k;
  for (i = 0; i < 3; i++) {
    C[i] = 0.;
    for (k = 0; k < 3; k++) C[i] += A[i][k] * B[k];
  }
}

/* C <- A */
void assign(const Matrix4d A, Matrix4d C) {
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) C[i][j] = A[i][j];
}

/* C <- A */
void transpose(const Matrix4d A, Matrix4d C) {
  Matrix4d B;
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) B[j][i] = A[i][j];
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
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) m[i][j] = i == j ? 1.0 : 0.0;
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
                 const double *model, const double *proj) {
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

    vxl = min(vxl, w[0]);
    vyl = min(vyl, w[1]);
    vzl = min(vzl, w[2]);

    vxh = max(vxh, w[0]);
    vyh = max(vyh, w[1]);
    vzh = max(vzh, w[2]);
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

  glUseProgram(0);

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
  //   Uint32 saved_flags;
  //   Uint8 saved_alpha;
  int useMipmaps = 0;

  /* Use the surface width and height expanded to powers of 2 */
  w = powerOfTwo(surface->w);
  h = powerOfTwo(surface->h);

  /* Rescale image if needed? */
  GLint maxSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
  double scale = 1.0;
  if (w > maxSize || h > maxSize) scale = min(maxSize / (double)w, maxSize / (double)h);

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

  /* Save the alpha blending attributes */
  //   saved_flags = surface->flags & (SDL_SRCALPHA | SDL_RLEACCELOK);
  //   int alpha_valid = !SDL_GetSurfaceAlphaMod(surface, &saved_alpha);
  //   if (alpha_valid) SDL_SetSurfaceAlphaMod(surface, 0);
  //   if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {

  //   }

  /* Copy the surface into the GL texture image */
  area.x = 0;
  area.y = 0;
  area.w = (int)(surface->w * scale);
  area.h = (int)(surface->h * scale);

  if (useMipmaps) scale = 1.0;  // this looks like a bug!

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

  /* Restore the alpha blending attributes */
  //   if ((saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA) {
  //   if (alpha_valid) SDL_SetSurfaceAlphaMod(surface, /*saved_flags,*/ saved_alpha);
  //   }

  /* Create an OpenGL texture for the image */
  GLuint freshTexture = 0;
  if (texture == NULL) texture = &freshTexture;
  if (*texture == 0) {
    glGenTextures(1, texture);
    /*printf("texture: %d created\n",*texture); */
  }

  glBindTexture(GL_TEXTURE_2D, *texture);
  if (linearFilter) {
    if (useMipmaps) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

  } else {
    if (useMipmaps) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
  }

  if (useMipmaps)
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, (int)(w * scale), (int)(h * scale), GL_RGBA,
                      GL_UNSIGNED_BYTE, image->pixels);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)(w * scale), (int)(h * scale), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image->pixels);

  SDL_FreeSurface(image); /* No longer needed */
  return *texture;
}

// YP: draw a spike with TRIANGLE_FAN. less time used by C->GL
//     copies, and reduction of glBegin/glEnd number.
void drawSpike(Coord3d a, Coord3d b, Coord3d c, Coord3d d) {
  Coord3d ab, ac, ad;
  Coord3d normal1, normal2, normal3;

  sub(b, a, ab);
  sub(c, a, ac);
  sub(d, a, ac);
  crossProduct(ac, ab, normal1);
  normalize(normal1);
  crossProduct(ab, ad, normal2);
  normalize(normal2);
  crossProduct(ad, ab, normal3);
  normalize(normal3);

  glBegin(GL_TRIANGLE_FAN);
  glNormal3dv(normal1);
  glVertex3dv(a);
  glVertex3dv(c);
  glVertex3dv(b);
  glNormal3dv(normal2);
  glVertex3dv(d);
  glNormal3dv(normal3);
  glVertex3dv(c);
  glEnd();
}
