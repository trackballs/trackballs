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
#include "image.h"

using namespace std;

float fps = 50.0;
float realTimeNow = 0.0;
int screenWidth = 640, screenHeight = 480;
const GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
const GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};
GLUquadricObj *qobj, *qball, *qdiamond;

/* Precompiled display lists for spheres of varying resolution (dependent on gfx settings) */
GLuint sphereDisplayLists[3];

TTF_Font *msgFont, *infoFont, *ingameFont, *menuFont, *scrollFont;
extern SDL_Surface *screen;

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

void draw2DString(TTF_Font *font, char *string, int x, int y, Uint8 red, Uint8 green,
                  Uint8 blue) {
  int w, h;
  SDL_Color fgColor = {255, 255, 255};
  SDL_Surface *text;
  GLuint texture;
  GLfloat texcoord[4];
  GLfloat texMinX, texMinY;
  GLfloat texMaxX, texMaxY;

  fgColor.r = red;
  fgColor.g = green;
  fgColor.b = blue;
  text = TTF_RenderText_Blended(font, string, fgColor);
  texture = LoadTexture(text, texcoord, 1, NULL);
  w = text->w;
  h = text->h;
  SDL_FreeSurface(text);

  texMinX = texcoord[0];
  texMinY = texcoord[1];
  texMaxX = texcoord[2];
  texMaxY = texcoord[3];

  y -= h;

  Enter2DMode();
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x, y);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w, y);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x, y + h);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w, y + h);
  glEnd();
  Leave2DMode();

  glDeleteTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, textures[0]);  // This is needed for mga_dri cards
}
/* Creates a texture from an SDL surface and draws it on screen.
   Inefficient since the texture is created + deleted.  Avoid using this function.
*/
void drawSurface(SDL_Surface *surface, int x, int y, int w, int h) {
  printf(
      "WARNING: Using depracated function 'drawSurface' - dont do this or bad things may "
      "happen!!!\n");

  static GLuint texture = 0;
  GLfloat texcoord[4];
  GLfloat texMinX, texMinY;
  GLfloat texMaxX, texMaxY;

  texture = LoadTexture(surface, texcoord, 0, NULL);

  texMinX = texcoord[0];
  texMinY = texcoord[1];
  texMaxX = texcoord[2];
  texMaxY = texcoord[3];

  Enter2DMode();
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x, y);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w, y);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x, y + h);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w, y + h);
  glEnd();
  Leave2DMode();
  glBindTexture(GL_TEXTURE_2D, textures[0]);  // This is needed for
                                              // mga_dri cards

  // why is this removed? should it not be done???
  glDeleteTextures(1, &texture);
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
/** Draws a textured 2D rectangle with min texcoord at x, y, max texcoord at x+w,y+h.
   Uses the texture which is already bound.
   Suitable for drawing 2D graphics like the panels on the screen.
*/
void drawTextured2DRectangle(int x, int y, int w, int h) {
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(0.0, 0.0);
  glVertex2i(x, y);
  glTexCoord2f(1.0, 0.0);
  glVertex2i(x + w, y);
  glTexCoord2f(0.0, 1.0);
  glVertex2i(x, y + h);
  glTexCoord2f(1.0, 1.0);
  glVertex2i(x + w, y + h);
  glEnd();
}

void drawMousePointer() {
  int mouseX, mouseY;
  glColor4f(1.0, 1.0, 1.0, 1.0);
  sparkle2D->draw();
  SDL_GetMouseState(&mouseX, &mouseY);
  drawMouse(mouseX, mouseY, 64, 64, 0.01);
}

void drawMouse(int x, int y, int w, int h, Real td) {
  GLfloat texMinX, texMinY;
  GLfloat texMaxX, texMaxY;

  bindTexture("mousePointer.png");
  texMinX = 0.0;
  texMinY = 0.0;
  texMaxX = 1.0;
  texMaxY = 1.0;

  Enter2DMode();
  glEnable(GL_BLEND);
  glColor4f(1.0, 1.0, 1.0, 1.0);  // 0.8 + 0.2*cos(3.23*mousePointerPhase));
  // glBindTexture(GL_TEXTURE_2D, mousePointerTexture);
  glPushMatrix();
  glTranslatef(x, y, 0.0);
  glRotatef(mousePointerPhase * 20.0, 0.0, 0.0, 1.0);
  glScalef(1.0 + 0.1 * cos(mousePointerPhase * 1.8), 1.0 + 0.1 * cos(mousePointerPhase * 1.9),
           1.0);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(-w / 2, -h / 2);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(w / 2, -h / 2);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(-w / 2, h / 2);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(w / 2, h / 2);
  glEnd();
  glPopMatrix();
  glDisable(GL_BLEND);
  Leave2DMode();
  // glDeleteTextures(1,&texture);
  glBindTexture(GL_TEXTURE_2D, textures[0]);  // This is needed for mga_dri cards
}
void drawFuzzySurface(SDL_Surface *surface, int x, int y, int w, int h) {
  GLuint texture;
  GLfloat texcoord[4];
  GLfloat texMinX, texMinY;
  GLfloat texMaxX, texMaxY;

  texture = LoadTexture(surface, texcoord, 1, NULL);
  texMinX = texcoord[0];
  texMinY = texcoord[1];
  texMaxX = texcoord[2];
  texMaxY = texcoord[3];

  Enter2DMode();
  glBindTexture(GL_TEXTURE_2D, texture);
  glEnable(GL_BLEND);
  glColor4f(1.0, 1.0, 1.0, 0.5);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x - 4, y);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w - 4, y);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x - 4, y + h);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w - 4, y + h);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x + 4, y);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w + 4, y);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x + 4, y + h);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w + 4, y + h);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x, y - 4);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w, y - 4);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x, y + h - 4);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w, y + h - 4);
  glEnd();
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x, y + 4);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w, y + 4);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x, y + h + 4);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w, y + h + 4);
  glEnd();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glBegin(GL_TRIANGLE_STRIP);
  glTexCoord2f(texMinX, texMinY);
  glVertex2i(x, y);
  glTexCoord2f(texMaxX, texMinY);
  glVertex2i(x + w, y);
  glTexCoord2f(texMinX, texMaxY);
  glVertex2i(x, y + h);
  glTexCoord2f(texMaxX, texMaxY);
  glVertex2i(x + w, y + h);
  glEnd();

  Leave2DMode();
  glDeleteTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, textures[0]);  // This is needed for mga_dri cards
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
    fprintf(stderr, _("Warning: cannot allocate %lu bytes for snapshot. Aborting.\n"),
            sizeof(unsigned char) * screenWidth * screenHeight * 3);
    return (0);
  }

  /* find the name for the image */
  do {
    sprintf(name, "./snapshot_%04d.ppm", snap_number++);
    if ((f = fopen(name, "r")) == NULL) {
      again = 0;
    } else {
      fclose(f);
    }
  } while (again);

  /* Check against symlinks */
  if (pathIsLink(name)) {
    fprintf(stderr, "Error: file %s is a symlink.\n", name);
    return 0;
  }

  /* get the screen */
  glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)buffer);

  /* save the image */
  if ((f = fopen(name, "w")) == NULL) {
    fprintf(stderr, _("Warning: cannot create file '%s'. Abort\n"), name);
    free(buffer);
    return (0);
  }

  fprintf(f, "P6\n# CREATOR: Hexasoft Corp. / trackballs Snapshot\n");
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
  w1 = Font::getTextWidth(0, A, size);
  w2 = Font::getTextWidth(0, B, size);
  h1 = 32;
  h2 = 32;

  w = max(w1, w2) + 20;

  Enter2DMode();
  int x1 = screenWidth / 2 - w / 2, x2 = screenWidth / 2 + w / 2;
  int y1 = screenHeight / 2 - h1 - 5, y2 = screenHeight / 2 + h2 + 5;

  glColor4f(0.2, 0.5, 0.2, 0.5);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex2i(x1, y1);
  glVertex2i(x2, y1);
  glVertex2i(x1, y2);
  glVertex2i(x2, y2);
  glEnd();
  Leave2DMode();

  glColor4f(0.5, 1.0, 0.2, 1.0);
  // draw2DString(msgFont,A,screenWidth/2-w1/2,screenHeight/2-1,255,255,255);
  Font::drawCenterSimpleText(0, A, screenWidth / 2 + size, screenHeight / 2 - size, size, size,
                             0.5, 1.0, 0.2, 1.0);
  // draw2DString(msgFont,B,screenWidth/2-w2/2,screenHeight/2+h2,255,255,255);
  Font::drawCenterSimpleText(0, B, screenWidth / 2 + size, screenHeight / 2 + 14, size, size,
                             0.5, 1.0, 0.2, 1.0);
}

void multiMessage(int nlines, const char *left[], const char *right[]) {
  int w, h, total_height, width, h_now;
  int i;
  int size = 16;

  total_height = 0;
  for (i = 0; i < nlines; i++) {
    // TTF_SizeText(msgFont,left[i],&w,&h);
    w = Font::getTextWidth(0, left[i], size);
    h = size * 2;
    total_height += h;
  }
  width = 600;

  Enter2DMode();
  int x1 = screenWidth / 2 - width / 2 - 5, x2 = screenWidth / 2 + width / 2 + 5;
  int y1 = screenHeight / 2 - total_height / 2 - 5,
      y2 = screenHeight / 2 + total_height / 2 + 30;

  glColor4f(0.2, 0.5, 0.2, 0.5);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLE_STRIP);
  glVertex2i(x1, y1);
  glVertex2i(x2, y1);
  glVertex2i(x1, y2);
  glVertex2i(x2, y2);
  glEnd();

  Leave2DMode();

  glColor4f(0.5, 1.0, 0.2, 1.0);
  h_now = -size;
  TTF_SizeText(msgFont, left[0], &w, &h);
  for (i = 0; i < nlines; i++) {
    h_now += h;
    if (left[i]) {
      // TTF_SizeText(msgFont,left[i],&w,&h);
      // draw2DString(msgFont,left[i],screenWidth/2-width/2,screenHeight/2-total_height/2+h_now,255,255,255);
      Font::drawSimpleText(0, left[i], screenWidth / 2 - width / 2 + size,
                           screenHeight / 2 - total_height / 2 + h_now, size, size, 0.5, 1.0,
                           0.2, 1.0);
    }
    if (right[i]) {
      Font::drawRightSimpleText(0, right[i], screenWidth / 2 + width / 2 + size,
                                screenHeight / 2 - total_height / 2 + h_now, size, size, 0.5,
                                1.0, 0.2, 1.0);
      // TTF_SizeText(msgFont,right[i],&w,&h);
      // draw2DString(msgFont,right[i],screenWidth/2+width/2-w,screenHeight/2-total_height/2+h_now,255,255,255);
    }
  }
}

void drawTriangle(Coord3d a, Coord3d b, Coord3d c) {
  Coord3d ab, ac;
  sub(b, a, ab);
  sub(c, a, ac);
  Coord3d normal;
  crossProduct(ab, ac, normal);
  normalize(normal);

  glBegin(GL_TRIANGLES);
  glNormal3d(normal[0], normal[1], normal[2]);
  glVertex3d(a[0], a[1], a[2]);
  glVertex3d(b[0], b[1], b[2]);
  glVertex3d(c[0], c[1], c[2]);
  glEnd();
}

SDL_Surface *getSurfaceFromRGB(char *fullPath) {
  SDL_Surface *surface;
  int tx, ty;
  unsigned char *font_data = NULL;

  read_2d_image_rgb(fullPath, &font_data, &tx, &ty);
  surface = SDL_CreateRGBSurfaceFrom((void *)font_data, tx, ty, 32, tx * 4, 0xFF000000,
                                     0x00FF0000, 0x0000FF00, 0x000000FF);
  free(font_data);

  if (!surface) printf(_("Error creating surface from RGB file: %s\n"), SDL_GetError());
  return surface;
}

/** Loads a texture from file and returns a reference to it.
    It is safe to load the same texture multiple times since the results are cached
*/
int loadTexture(const char *name, Font *font) {
  GLfloat texCoord[4];
  char str[256];
  SDL_Surface *surface;
  const char *tempStr;
  int i;

  /* Check in cache if texture already loaded */
  for (i = 0; i < numTextures; i++)
    if (strcmp(textureNames[i], name) == 0) return i;

  if (numTextures >= GLHELP_MAX_TEXTURES) {
    fprintf(stderr,
            _("Warning: max. number of textures reached (%d). Texture '%s' not loaded.\n"),
            GLHELP_MAX_TEXTURES, name);
    return 0;
  }

  snprintf(str, sizeof(str), "%s/images/%s", SHARE_DIR, name);
  tempStr = strstr(name, ".rgb");
  if (tempStr != NULL && strcmp(tempStr, ".rgb") == 0) {
    surface = getSurfaceFromRGB(str);
  } else {
    surface = IMG_Load(str);
  }
  if (!surface) {
    printf(_("Warning: Failed to load texture %s\n"), str);
    textureNames[numTextures] = strdup(name);
    textures[numTextures++] =
        textures[0];  // just assume we managed to load this, better than nothing
    return -1;
  } else {
    textureNames[numTextures] = strdup(name);
    textures[numTextures] =
        LoadTexture(surface, texCoord, 1, NULL);  // linear filter was: font != NULL
    /*printf("loaded texture[%d]=%d\n",numTextures,textures[numTextures]);*/
    numTextures++;

    /* Strange special case handling when we are called from the FONT routines */
    if (font != NULL) {
      if (!(font->figureFontInfo(surface->pixels, surface->w, surface->h))) {
        --numTextures;
        free(textureNames[numTextures]);
        textureNames[numTextures] = NULL;
        glDeleteTextures(1, &textures[numTextures]);
        SDL_FreeSurface(surface);
        return -1;
      }
    }
    SDL_FreeSurface(surface);
  }
  return (numTextures - 1);  // ok
}

void glHelpInit() {
  char str[256];

  qobj = gluNewQuadric();
  gluQuadricDrawStyle(qobj, GLU_FILL);

  for (int i = 0; i < 4711; i++) fake_rand[i] = frand();

  TTF_Init();
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  msgFont = TTF_OpenFont(str, 30);  // 30 astron
  if (!msgFont) {
    fprintf(stderr, "Error: failed to load font %s\n", str);
    exit(0);
  }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  infoFont = TTF_OpenFont(str, 18);
  if (!infoFont) {
    fprintf(stderr, "Error: failed to load font %s\n", str);
    exit(0);
  }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  ingameFont = TTF_OpenFont(str, 30);  // barbatri
  if (!ingameFont) {
    fprintf(stderr, "Error: failed to load font %s\n", str);
    exit(0);
  }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  menuFont = TTF_OpenFont(str, 40);  // barbatri
  if (!menuFont) {
    fprintf(stderr, "Error: failed to load font %s\n", str);
    exit(0);
  }
  snprintf(str, sizeof(str), "%s/fonts/%s", SHARE_DIR, "menuFont.ttf");
  scrollFont = TTF_OpenFont(str, 18);
  if (!(msgFont && infoFont && ingameFont)) {
    printf("Error: failed to load fonts\n");
    exit(0);
  }
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

  qdiamond = gluNewQuadric();
  gluQuadricDrawStyle(qdiamond, GLU_FILL);
  gluQuadricNormals(qdiamond, GLU_SMOOTH);
  gluQuadricTexture(qdiamond, GL_TRUE);

  sphereDisplayLists[0] = glGenLists(1);
  sphereDisplayLists[1] = glGenLists(1);
  sphereDisplayLists[2] = glGenLists(1);
  regenerateSphereDisplaylists();
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
  char *tempStr;
  int linear = 0;

  for (i = 0; i < numTextures; i++) {
    if (textureNames[i] == NULL) continue;
    snprintf(str, sizeof(str), "%s/images/%s", SHARE_DIR, textureNames[i]);
    tempStr = strstr(textureNames[i], ".rgb");
    if (tempStr != NULL && strcmp(tempStr, ".rgb") == 0) {
      surface = getSurfaceFromRGB(str);
      linear = 1;
    } else {
      surface = IMG_Load(str);
    }
    if (!surface) {
      printf(_("Warning: Failed to load texture %s\n"), str);
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
      printf(_("Warning: too fast framerate (%f)\n"), td);
      hasWarned = 1;
    }
  } else
    fps = fps * 0.95 + 0.05 / td;
  oldTime = t;
  realTimeNow = t;

  if (Settings::settings->showFPS) {
    if (fps > 0)
      snprintf(str, sizeof(str), _("Framerate: %.1f"), fps);
    else
      snprintf(str, sizeof(str), _("Framerate unknown"));

    // glColor3f(1.0,1.0,1.0);
    Font::drawSimpleText(0, str, 15, screenHeight - 15, 10., 10., 1., 1., 0.25, 0.8);
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
void matrixMult(Matrix4d A, Matrix4d B, Matrix4d C) {
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
    C[i] = A[i][3];
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

void Enter2DMode() {
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, screen->w, screen->h);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glOrtho(0.0, (GLdouble)screen->w, (GLdouble)screen->h, 0.0, 0.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Leave2DMode() {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glPopAttrib();
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

  /*
  printf("loadTexture: surface->w = %d, surface->h = %d\nw = %d, h = %d, maxSize = %d, scale =
  %f nw=%d nh=%d\n",
         surface->w,surface->h,w,h,maxSize,scale,(int)(w*scale),(int)(h*scale));
  */
  texcoord[0] = 0.0f;                    /* Min X */
  texcoord[1] = 0.0f;                    /* Min Y */
  texcoord[2] = (GLfloat)surface->w / w; /* Max X */
  texcoord[3] = (GLfloat)surface->h / h; /* Max Y */

  image = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)(w * scale), (int)(h * scale), 32,

#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
                               0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
                               0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
                               );
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

  /*printf("area.w = %d, area.h = %d\n",area.w,area.h);*/

  if (useMipmaps) scale = 1.0;  // this looks like a bug!

  if (scale == 1.0)
    // Easy, no scaling needed
    SDL_BlitSurface(surface, NULL, image, &area);
  else {
    printf("doing scaling!\n");

    // Scaling needed
    // This is a realy inefficient and unoptimised way to do it. Sorry!
    static int hasWarned = 0;
    if (!hasWarned) {
      printf(_("Warning: Rescaling images before loading them as textures.\n"));
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

void createForcefieldTextures() {}
void resetForcefieldTextures() {}
void bindForcefieldTexture(double t0) {}
