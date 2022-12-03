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
#include "game.h"
#include "map.h"
#include "settings.h"
#include "sparkle2d.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <map>

float fps = 50.0;
int screenWidth = 640, screenHeight = 480;
ViewParameters activeView;

typedef struct {
  GLuint prog;
  UniformLocations locations;
} GLProgram;

static GLProgram shaderWaterDay = {};
static GLProgram shaderWaterNight = {};
static GLProgram shaderTileDay = {};
static GLProgram shaderTileNight = {};
static GLProgram shaderTileShadow = {};
static GLProgram shaderLine = {};
static GLProgram shaderUI = {};
static GLProgram shaderObjectDay = {};
static GLProgram shaderObjectNight = {};
static GLProgram shaderObjectShadow = {};
static GLProgram shaderReflection = {};
GLuint textureBlank = 0;
GLuint textureGlitter = 0;
GLuint textureMousePointer = 0;
GLuint textureDizzy = 0;
GLuint textureWings = 0;
GLuint textureTrack = 0;
static GLuint dummyCascadeTexture = 0;
static GLuint dummyCubeMapTexture = 0;

TTF_Font *ingameFont;
extern struct timespec displayStartTime;
extern struct timespec lastDisplayStartTime;

const Color menuColorSelected(SRGBColor(0.86f, 0.86f, 0.86f, 1.f));
const Color menuColor(SRGBColor(0.86f, 0.86f, 0.25f, 1.f));

const Matrix4d identity4 = {
    {1., 0., 0., 0.},
    {0., 1., 0., 0.},
    {0., 0., 1., 0.},
    {0., 0., 0., 1.},
};
const Matrix3d identity3 = {
    {1., 0., 0.},
    {0., 1., 0.},
    {0., 0., 1.},
};

const double sin6[6] = {0.0, 0.8660254037844386,  0.8660254037844386,
                        0.0, -0.8660254037844386, -0.8660254037844386};
const double cos6[6] = {1.0, 0.5, -0.5, -1.0, -0.5, 0.5};
const double sin10[10] = {
    0.0, 0.5877852522924731,  0.9510565162951535,  0.9510565162951535,  0.5877852522924731,
    0.0, -0.5877852522924731, -0.9510565162951535, -0.9510565162951535, -0.5877852522924731};
const double cos10[10] = {
    1.0,  0.8090169943749475,  0.30901699437494745,  -0.30901699437494745, -0.8090169943749475,
    -1.0, -0.8090169943749475, -0.30901699437494745, 0.30901699437494745,  0.8090169943749475};
const double sin12[12] = {0.0, 0.5,  0.8660254037844386,  1.0,  0.8660254037844386,  0.5,
                          0.0, -0.5, -0.8660254037844386, -1.0, -0.8660254037844386, -0.5};
const double cos12[12] = {1.0,  0.8660254037844386,  0.5,  0.0, -0.5, -0.8660254037844386,
                          -1.0, -0.8660254037844386, -0.5, 0.0, 0.5,  0.8660254037844386};
const double sin14[14] = {0.0,
                          0.4338837391175581,
                          0.7818314824680298,
                          0.9749279121818236,
                          0.9749279121818236,
                          0.7818314824680298,
                          0.4338837391175581,
                          0.0,
                          -0.4338837391175581,
                          -0.7818314824680298,
                          -0.9749279121818236,
                          -0.9749279121818236,
                          -0.7818314824680298,
                          -0.4338837391175581};
const double cos14[14] = {1.0,
                          0.9009688679024191,
                          0.6234898018587336,
                          0.22252093395631445,
                          -0.22252093395631445,
                          -0.6234898018587336,
                          -0.9009688679024191,
                          -1.0,
                          -0.9009688679024191,
                          -0.6234898018587336,
                          -0.22252093395631445,
                          0.22252093395631445,
                          0.6234898018587336,
                          0.9009688679024191};

#define GLHELP_MAX_TEXTURES 256
GLuint textures[GLHELP_MAX_TEXTURES] = {0};  // added init. to 0 (no texture)
char *textureNames[GLHELP_MAX_TEXTURES] = {NULL};
int numTextures;

#define MAX_BALL_DETAIL 32
GLfloat *sphere_points[MAX_BALL_DETAIL];
GLfloat *sphere_texcos[MAX_BALL_DETAIL];
ushort *sphere_idxs[MAX_BALL_DETAIL];

static std::map<int, TTF_Font *> menuFontLookup;

Sparkle2D *sparkle2D = NULL;

struct StringInfo {
  TTF_Font *font;
  char string[256];
};
static bool operator<(const struct StringInfo &a, const struct StringInfo &b) {
  if (a.font != b.font) return a.font < b.font;
  return strcmp(a.string, b.string) < 0;
}

struct StringCache {
  GLuint texture;
  GLfloat texcoord[4];
  int w, h;
  long tick;
};

static std::map<StringInfo, StringCache> strcache;

static SDL_Surface *drawStringToSurface(struct StringInfo &inf, bool outlined) {
  SDL_Color white = {255, 255, 255, 255};
  SDL_Color black = {0, 0, 0, 0};
  if (outlined) {
    TTF_SetFontOutline(inf.font, 0);

    SDL_Surface *inner = TTF_RenderUTF8_Blended(inf.font, inf.string, white);
    if (!inner) {
      warning("Failed to render string outline '%s'", inf.string);
      return NULL;
    }
    TTF_SetFontOutline(inf.font, 2);

    SDL_Surface *outline = TTF_RenderUTF8_Blended(inf.font, inf.string, black);
    if (!outline) {
      warning("Failed to render string inside '%s'", inf.string);
      return NULL;
    }
    SDL_Rect rect = {2, 2, inner->w, inner->h};
    SDL_SetSurfaceBlendMode(inner, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(inner, NULL, outline, &rect);
    SDL_FreeSurface(inner);
    return outline;
  } else {
    SDL_Surface *outline = TTF_RenderUTF8_Blended(inf.font, inf.string, white);
    if (!outline) {
      warning("Failed to render string '%s'", inf.string);
      return NULL;
    }
    return outline;
  }
}

int prepare2DString(TTF_Font *font, const char *string, bool outlined) {
  struct StringInfo inf;
  memset(&inf, 0, sizeof(inf));
  inf.font = font;
  snprintf(inf.string, sizeof(inf.string), "%s", string);
  if (strcache.count(inf) <= 0) {
    struct StringCache newentry;
    newentry.tick = displayFrameNumber;
    SDL_Surface *surf = drawStringToSurface(inf, outlined);
    if (!surf) { return 0; }
    newentry.texture = LoadTexture(surf, newentry.texcoord);
    newentry.w = surf->w;
    newentry.h = surf->h;
    SDL_FreeSurface(surf);
    strcache[inf] = newentry;
  } else {
    strcache[inf].tick = displayFrameNumber;
  }
  return strcache[inf].w;
}

int draw2DString(TTF_Font *font, const char *string, int x, int y, Color color, bool outlined,
                 int align, int maxwidth) {
  prepare2DString(font, string, outlined);

  struct StringInfo inf;
  memset(&inf, 0, sizeof(inf));
  inf.font = font;
  snprintf(inf.string, sizeof(inf.string), "%s", string);
  const struct StringCache cached = strcache[inf];

  GLfloat shrink = (maxwidth > 0 && maxwidth < cached.w) ? (maxwidth / (GLfloat)cached.w) : 1.;
  draw2DRectangle(x - shrink * align * cached.w / 2, y - shrink * cached.h / 2,
                  shrink * cached.w, shrink * cached.h, cached.texcoord[0], cached.texcoord[1],
                  cached.texcoord[2], cached.texcoord[3], color, cached.texture);
  return maxwidth > 0 ? std::min(cached.w, maxwidth) : cached.w;
}

void update2DStringCache(bool force_wipe) {
  if (!force_wipe && strcache.size() < 100) {
    /* Cache up to 100 strings */
    return;
  }

  int erased;
  do {
    erased = false;
    // TODO: better asymptotics
    for (std::map<StringInfo, StringCache>::iterator i = strcache.begin(); i != strcache.end();
         ++i) {
      if (i->second.tick < displayFrameNumber - 100 || force_wipe) {
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
  float mouseSpeedX = (mouseX - oldMouseX) / td;
  float mouseSpeedY = (mouseY - oldMouseY) / td;
  float mouseSpeedLen = std::sqrt(mouseSpeedX * mouseSpeedX + mouseSpeedY * mouseSpeedY);
  if (mouseSpeedLen > 25.f) {
    mouseSpeedX /= 25.f / mouseSpeedLen;
    mouseSpeedY /= 25.f / mouseSpeedLen;
    mouseSpeedLen = 25.f;
  }
  static Real last_sparkle = 0.0;

  oldMouseX = mouseX;
  oldMouseY = mouseY;

  mousePointerPhase += td;
  sparkle2D->tick(td);
  last_sparkle += td * (1.0 + mouseSpeedLen * 0.1);
  while (last_sparkle > 0.0) {
    last_sparkle -= 0.05;
    float pos[2] = {(float)(mouseX + 10.0f * (frandom() - 0.5f)),
                    (float)(mouseY + 10.0f * (frandom() - 0.5f))};
    float speed[2] = {(float)(mouseSpeedX / 2.f + 100.0f * (frandom() - 0.5f)),
                      (float)(mouseSpeedY / 2.f - 120.0f * frandom())};
    float color[4] = {(float)(0.8f + 0.2f * frandom()), (float)(0.8f + 0.2f * frandom()),
                      (float)(0.5f + 0.2f * frandom()), (float)(0.9f - 0.35f * frandom())};
    sparkle2D->add(pos, speed, (float)(1.5 + 0.5 * (frandom() - 0.5)),
                   (float)(11 + 3. * (frandom() - 0.5)), color);
  }
}

void draw2DRectangle(GLfloat x, GLfloat y, GLfloat w, GLfloat h, GLfloat tx, GLfloat ty,
                     GLfloat tw, GLfloat th, Color color, GLuint tex) {
  GLfloat corners[4][2] = {{x, y}, {x, y + h}, {x + w, y}, {x + w, y + h}};
  GLfloat texture[4][2] = {{tx, ty}, {tx, ty + th}, {tx + tw, ty}, {tx + tw, ty + th}};
  Color colors[4] = {color, color, color, color};
  draw2DQuad(corners, texture, colors, tex);
}

void draw2DQuad(const GLfloat ver[4][2], const GLfloat txc[4][2], const Color c[4],
                GLuint tex) {
  Require2DMode();
  if (tex == 0) { tex = textureBlank; }

  const GLfloat data[32] = {
      ver[0][0], ver[0][1], c[0].f0(), c[0].f1(), c[0].f2(), c[0].f3(), txc[0][0], txc[0][1],
      ver[1][0], ver[1][1], c[1].f0(), c[1].f1(), c[1].f2(), c[1].f3(), txc[1][0], txc[1][1],
      ver[2][0], ver[2][1], c[2].f0(), c[2].f1(), c[2].f2(), c[2].f3(), txc[2][0], txc[2][1],
      ver[3][0], ver[3][1], c[3].f0(), c[3].f1(), c[3].f2(), c[3].f3(), txc[3][0], txc[3][1],
  };

  static GLuint idxs = (GLuint)-1;
  static GLuint buf = (GLuint)-1;
  static GLuint vao = (GLuint)-1;
  if (idxs == (GLuint)-1) {
    glGenBuffers(1, &idxs);
    glGenBuffers(1, &buf);
    glGenVertexArrays(1, &vao);

    const ushort idxdata[6] = {0, 1, 2, 1, 2, 3};

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxs);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(ushort), idxdata, GL_STATIC_DRAW);
    configureUIAttributes();
  } else {
    glBindVertexArray(vao);
  }

  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 32 * sizeof(GLfloat), data);

  glBindTexture(GL_TEXTURE_2D, tex);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void *)0);
}

void drawMousePointer() {
  int mouseX, mouseY;
  sparkle2D->draw();
  SDL_GetMouseState(&mouseX, &mouseY);
  drawMouse(mouseX, mouseY, 64, 64);
}

void drawMouse(int x, int y, int w, int h) {
  GLfloat r1 = 1.0 + 0.1 * std::cos(mousePointerPhase * 1.8);
  GLfloat r2 = 1.0 + 0.1 * std::cos(mousePointerPhase * 1.9);
  GLfloat dx = 0.707f * w * r1 * std::sin(mousePointerPhase * 0.35);
  GLfloat dy = 0.707f * h * r2 * std::cos(mousePointerPhase * 0.35);

  Color white(1.f, 1.f, 1.f, 1.f);
  Color colors[4] = {white, white, white, white};
  GLfloat texco[4][2] = {{0., 0.}, {0., 1.}, {1., 0.}, {1., 1.}};

  GLfloat vco[4][2] = {{x - dx, y - dy}, {x - dy, y + dx}, {x + dy, y - dx}, {x + dx, y + dy}};
  draw2DQuad(vco, texco, colors, textureMousePointer);
}

size_t packObjectVertex(void *dest, GLfloat x, GLfloat y, GLfloat z, GLfloat tx, GLfloat ty,
                        const Color &color, const GLfloat normal[3]) {
  uint32_t *aout = (uint32_t *)dest;
  GLfloat *fout = (GLfloat *)dest;
  fout[0] = x;
  fout[1] = y;
  fout[2] = z;
  aout[3] = (((uint32_t)(65535.f * color.v[1])) << 16) + (uint32_t)(65535.f * color.v[0]);
  aout[4] = (((uint32_t)(65535.f * color.v[3])) << 16) + (uint32_t)(65535.f * color.v[2]);
  aout[5] = (((uint32_t)(65535.f * 0.5f * ty)) << 16) + (uint32_t)(65535.f * 0.5f * tx);
  aout[6] = packNormal(normal);
  return 8 * sizeof(GLfloat);
}
void configureObjectAttributes() {
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
  glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                        (void *)(3 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                        (void *)(5 * sizeof(GLfloat)));
  glVertexAttribPointer(3, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 8 * sizeof(GLfloat),
                        (void *)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
}
void configureUIAttributes() {
  // To be used by `shaderUI`
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(2 * sizeof(GLfloat)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                        (void *)(6 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
}

static const GLProgram *lastProgram = NULL;

void markViewChanged() { lastProgram = NULL; }

const UniformLocations *setActiveProgramAndUniforms(Shader_Type type) {
  const GLProgram *prog = NULL;
  switch (type) {
  case Shader_Line:
    prog = &shaderLine;
    break;
  case Shader_Object:
    if (activeView.calculating_shadows) {
      prog = &shaderObjectShadow;
    } else {
      if (activeView.day_mode) {
        prog = &shaderObjectDay;
      } else {
        prog = &shaderObjectNight;
      }
    }
    break;
  case Shader_Tile:
    if (activeView.calculating_shadows) {
      prog = &shaderTileShadow;
    } else {
      if (activeView.day_mode) {
        prog = &shaderTileDay;
      } else {
        prog = &shaderTileNight;
      }
    }
    break;
  case Shader_Water:
    if (activeView.day_mode) {
      prog = &shaderWaterDay;
    } else {
      prog = &shaderWaterNight;
    }
    break;
  case Shader_Reflection:
    prog = &shaderReflection;
    break;
  }
  const UniformLocations *uloc = &prog->locations;

  if (prog == lastProgram) return uloc;
  glUseProgram(prog->prog);
  lastProgram = prog;

  /* Set universal uniforms */
  Matrix4d mvp;
  matrixMult(activeView.modelview, activeView.projection, mvp);
  GLfloat lmvp[16], lmodel[16];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      lmvp[4 * i + j] = mvp[i][j];
      lmodel[4 * i + j] = activeView.modelview[i][j];
    }
  }
  glUniformMatrix4fv(uloc->mvp_matrix, 1, GL_FALSE, (GLfloat *)lmvp);
  glUniformMatrix4fv(uloc->model_matrix, 1, GL_FALSE, (GLfloat *)lmodel);
  if (type == Shader_Object) {
    GLfloat lobject[16];
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) { lobject[4 * i + j] = (i == j) ? 1.f : 0.f; }
    }
    glUniformMatrix4fv(uloc->object_matrix, 1, GL_FALSE, (GLfloat *)lobject);
  }

  if (!activeView.calculating_shadows) {
    /* Fog applies to all display shaders */
    glUniform1i(uloc->fog_active, activeView.fog_enabled);
    glUniform3f(uloc->fog_color, activeView.fog_color[0], activeView.fog_color[1],
                activeView.fog_color[2]);
    glUniform1f(uloc->fog_start, activeView.fog_start);
    glUniform1f(uloc->fog_end, activeView.fog_end);

    if (type == Shader_Object || type == Shader_Water || type == Shader_Tile) {
      glUniform3f(uloc->light_ambient, activeView.light_ambient[0],
                  activeView.light_ambient[1], activeView.light_ambient[2]);
      glUniform3f(uloc->light_diffuse, activeView.light_diffuse[0],
                  activeView.light_diffuse[1], activeView.light_diffuse[2]);
      if (type == Shader_Object) {
        glUniform3f(uloc->light_specular, activeView.light_specular[0],
                    activeView.light_specular[1], activeView.light_specular[2]);
      }

      if (activeView.day_mode) {
        glUniform1i(uloc->shadow_cascade0, 2);
        glUniform1i(uloc->shadow_cascade1, 3);
        glUniform1i(uloc->shadow_cascade2, 4);
        if (activeView.use_shadows) {
          glActiveTexture(GL_TEXTURE2);
          glBindTexture(GL_TEXTURE_2D, activeView.cascadeTexture[0]);
          glActiveTexture(GL_TEXTURE3);
          glBindTexture(GL_TEXTURE_2D, activeView.cascadeTexture[1]);
          glActiveTexture(GL_TEXTURE4);
          glBindTexture(GL_TEXTURE_2D, activeView.cascadeTexture[2]);
        } else {
          glActiveTexture(GL_TEXTURE2);
          glBindTexture(GL_TEXTURE_2D, dummyCascadeTexture);
          glActiveTexture(GL_TEXTURE3);
          glBindTexture(GL_TEXTURE_2D, dummyCascadeTexture);
          glActiveTexture(GL_TEXTURE4);
          glBindTexture(GL_TEXTURE_2D, dummyCascadeTexture);
        }

        const int N = 3;
        GLfloat cscmvp[N * 16], cscmodel[N * 16];
        for (int i = 0; i < N; i++) {
          Matrix4d cmvp;
          matrixMult(activeView.cascade_model[i], activeView.cascade_proj[i], cmvp);
          for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
              cscmvp[16 * i + 4 * j + k] = cmvp[j][k];
              cscmodel[16 * i + 4 * j + k] = activeView.cascade_model[i][j][k];
            }
          }
        }
        glUniformMatrix4fv(uloc->cascade_mvp, 3, GL_FALSE, (GLfloat *)cscmvp);
        glUniformMatrix4fv(uloc->cascade_model, 3, GL_FALSE, (GLfloat *)cscmodel);

        glUniform1i(uloc->shadowtex_size, activeView.cascadeTexsize);

        glUniform3f(uloc->sun_direction, activeView.sun_direction[0],
                    activeView.sun_direction[1], activeView.sun_direction[2]);
      } else {
        glUniform1i(uloc->shadow_map, 1);
        glActiveTexture(GL_TEXTURE1);
        if (activeView.use_shadows) {
          glBindTexture(GL_TEXTURE_CUBE_MAP, activeView.shadowMapTexture);
        } else {
          glBindTexture(GL_TEXTURE_CUBE_MAP, dummyCubeMapTexture);
        }

        glUniform3f(uloc->light_position, activeView.light_position[0],
                    activeView.light_position[1], activeView.light_position[2]);
      }
    }
  }

  /* Texture setup; this is last, because following code may bind texture */
  if (type == Shader_Object || type == Shader_Reflection) {
    glUniform1i(uloc->tex, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
  }
  return uloc;
}
void setObjectUniforms(const UniformLocations *uloc, const Matrix4d object_transform, Color c,
                       float sharpness, Object_Lighting lighting) {
  if (lastProgram == &shaderObjectDay || lastProgram == &shaderObjectNight) {
    glUniformC(uloc->specular, c);
    glUniform1f(uloc->sharpness, sharpness);
    glUniform1i(uloc->ignore_shadow, lighting == Lighting_Regular);
    glUniform1i(uloc->use_lighting, lighting != Lighting_None);
    glUniformMatrix4(uloc->object_matrix, object_transform);
  }
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

static void constructSphereData(int detail) {
  int radial_count = 4 * detail;
  int nrows = 2 * detail + 1;
  int nvertices = (radial_count + 1) * (nrows - 2) + 2;
  int ntriangles = 2 * radial_count * (nrows - 2);

  GLfloat *pts = new GLfloat[3 * nvertices];
  GLfloat *txs = new GLfloat[2 * nvertices];
  ushort *idxs = new ushort[3 * ntriangles];

  // Construct vertices
  int m = 0;
  for (int z = 1; z < nrows - 1; z++) {
    GLfloat theta = z * M_PI / (nrows - 1);
    for (int t = 0; t <= radial_count; t++) {
      GLfloat phi = 2 * t * M_PI / radial_count;
      // Stagger every second row
      if (z % 2 == 1) phi += M_PI / radial_count;
      txs[2 * m + 0] = phi / (2 * M_PI);
      txs[2 * m + 1] = theta / M_PI;
      pts[3 * m + 0] = std::sin(theta) * std::cos(phi);
      pts[3 * m + 1] = std::sin(theta) * std::sin(phi);
      pts[3 * m + 2] = std::cos(theta);
      m++;
    }
  }
  pts[3 * m + 0] = 0;
  pts[3 * m + 1] = 0;
  pts[3 * m + 2] = 1.;
  pts[3 * m + 3] = 0;
  pts[3 * m + 4] = 0;
  pts[3 * m + 5] = -1.;
  txs[2 * m + 0] = 0.5;
  txs[2 * m + 1] = 0.0;
  txs[2 * m + 2] = 0.5;
  txs[2 * m + 3] = 1.0;

  // Triangulate end caps
  for (int i = 0; i < radial_count; i++) {
    idxs[6 * i + 0] = i;
    idxs[6 * i + 1] = i + 1;
    idxs[6 * i + 2] = (radial_count + 1) * (nrows - 2);
    idxs[6 * i + 3] = (radial_count + 1) * (nrows - 3) + i + 1;
    idxs[6 * i + 4] = (radial_count + 1) * (nrows - 3) + i;
    idxs[6 * i + 5] = (radial_count + 1) * (nrows - 2) + 1;
  }
  // Triangulate body
  ushort *base = &idxs[2 * 3 * radial_count];
  for (int z = 1; z < nrows - 2; z++) {
    for (int i = 0; i < radial_count; i++) {
      base[6 * i + 0] = (z - 1) * (radial_count + 1) + i + 1;
      base[6 * i + 1] = (z - 1) * (radial_count + 1) + i;
      base[6 * i + 2] = z * (radial_count + 1) + i + 1;
      base[6 * i + 3] = (z - 1) * (radial_count + 1) + i;
      base[6 * i + 4] = z * (radial_count + 1) + i;
      base[6 * i + 5] = z * (radial_count + 1) + i + 1;
    }
    base = &base[6 * radial_count];
  }

  sphere_points[detail] = pts;
  sphere_texcos[detail] = txs;
  sphere_idxs[detail] = idxs;
}

void placeObjectSphere(void *data, ushort *idxs, ushort first_index, int detail,
                       const Color &color) {
  if (detail < 1) {
    warning("Sphere detail level must be > 1. Drawing nothing.");
    return;
  }
  if (!sphere_points[detail]) { constructSphereData(detail); }

  int radial_count = 4 * detail;
  int nrows = 2 * detail + 1;
  int nvertices = (radial_count + 1) * (nrows - 2) + 2;
  int ntriangles = 2 * radial_count * (nrows - 2);
  for (int i = 0; i < 3 * ntriangles; i++) { idxs[i] = first_index + sphere_idxs[detail][i]; }

  /* Copy and transform vertices */
  char *pos = (char *)data;
  GLfloat *pts = sphere_points[detail];
  GLfloat *txs = sphere_texcos[detail];
  for (int i = 0; i < nvertices; i++) {
    GLfloat loc[3] = {pts[3 * i], pts[3 * i + 1], pts[3 * i + 2]};
    GLfloat txc[2] = {txs[2 * i], txs[2 * i + 1]};

    pos += packObjectVertex(pos, loc[0], loc[1], loc[2], txc[0], txc[1], color, loc);
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
  Coord3d eye(eyeX, eyeY, eyeZ);
  Coord3d center(centerX, centerY, centerZ);
  Coord3d up(upX, upY, upZ);
  Coord3d f = center - eye;
  f = f / length(f);
  up = up / length(up);
  Coord3d s = crossProduct(f, up);
  s = s / length(s);
  Coord3d u = crossProduct(s, f);

  Matrix4d mat = {{s[0], u[0], -f[0], 0.},
                  {s[1], u[1], -f[1], 0.},
                  {s[2], u[2], -f[2], 0.},
                  {0., 0., 0., 1.}};
  Matrix4d trans = {
      {1., 0., 0., 0.}, {0., 1., 0., 0.}, {0., 0., 1., 0.}, {-eye[0], -eye[1], -eye[2], 1.}};
  matrixMult(trans, mat, out);
}

void renderShadowMap(const Coord3d &focus, Map *mp, Game *gm) {
  Matrix4d origMV, origProj;
  assign(activeView.modelview, origMV);
  assign(activeView.projection, origProj);

  activeView.calculating_shadows = true;
  static int ltexsize = 0;
  if (activeView.shadowMapTexsize <= 1 || ltexsize != Settings::settings->shadowTexsize) {
    ltexsize = Settings::settings->shadowTexsize;
    /* order doesn't matter */
    GLenum dirs[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                      GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                      GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    GLint reqSize = 1 << Settings::settings->shadowTexsize;
    activeView.shadowMapTexsize = std::min(maxSize, reqSize);

    glBindTexture(GL_TEXTURE_CUBE_MAP, activeView.shadowMapTexture);
    for (uint face = 0; face < 6; face++) {
      glTexImage2D(dirs[face], 0, GL_DEPTH_COMPONENT, activeView.shadowMapTexsize,
                   activeView.shadowMapTexsize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
  }

  GLdouble norv[6][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
  GLdouble upv[6][3] = {{0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}};
  /* order doesn't matter */
  GLenum dirs[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

  perspectiveMatrix(90., 1, 0.1, 1000., activeView.projection);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glViewport(0, 0, activeView.shadowMapTexsize, activeView.shadowMapTexsize);

  GLuint cubeFBOs[6];
  glGenFramebuffers(6, cubeFBOs);
  for (int i = 0; i < 6; i++) {
    /* issue: only fbo 1 is set? */
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cubeFBOs[i]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, dirs[i],
                           activeView.shadowMapTexture, 0);
    GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != result) {
      warning("Framebuffer is not complete. #%d w/err %x", i, result);
    }
  }

  glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
  // set Persp to square + angle
  for (int loop = 0; loop < 6; ++loop) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cubeFBOs[loop]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lookAtMatrix(activeView.light_position[0], activeView.light_position[1],
                 activeView.light_position[2], activeView.light_position[0] + norv[loop][0],
                 activeView.light_position[1] + norv[loop][1],
                 activeView.light_position[2] + norv[loop][2], upv[loop][0], upv[loop][1],
                 upv[loop][2], activeView.modelview);
    markViewChanged();
    // Render (todo: 50% alpha clip)
    if (mp) mp->draw(0, focus, gm ? gm->gameTime : 0.);
    if (gm) gm->draw();
  }

  /* back to default (to screen) frame buffer */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDeleteFramebuffers(6, cubeFBOs);

  activeView.calculating_shadows = false;
  assign(origMV, activeView.modelview);
  assign(origProj, activeView.projection);
  markViewChanged();
}

void renderShadowCascade(const Coord3d &focus, Map *mp, Game *gm) {
  Matrix4d origMV, origProj;
  assign(activeView.modelview, origMV);
  assign(activeView.projection, origProj);

  const int N = 3;

  GLfloat proj_half_angle = 40 / 2 * M_PI / 180;
  GLfloat aspect = (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1);

  /* shadow step scale by factor of 4 */
  GLfloat dts[N + 1] = {0.f, 12.5f, 50.f, 200.f};
  /* Construct ortho projection matrix clipped near at 0, far at 1000. */
  Matrix4d ortho = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, -2. / 100., -1}, {0, 0, 0, 1}};
  assign(ortho, activeView.projection);

  /* Construct light model matrices */
  Matrix4d mvmt;
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) mvmt[i][j] = origMV[j][i];
  /* extract rotation operation and camera center */
  Matrix3d mvm3t;
  for (int k = 0; k < 3; k++)
    for (int j = 0; j < 3; j++) mvm3t[j][k] = mvmt[k][j];
  Coord3d cc(mvmt[0][3], mvmt[1][3], mvmt[2][3]);
  Coord3d camera = -useMatrix(mvm3t, cc);

  Matrix4d light_align;
  lookAtMatrix(0, 0, 0, activeView.sun_direction[0], activeView.sun_direction[1],
               activeView.sun_direction[2], 0, 0, 1, light_align);
  Matrix3d light_align3;
  for (int k = 0; k < 3; k++)
    for (int j = 0; j < 3; j++) light_align3[j][k] = light_align[k][j];

  for (int i = 0; i < N; i++) {
    double bounds[3][2] = {{1e99, -1e99}, {1e99, -1e99}, {1e99, -1e99}};

    /* Estimate orthogonal projection matrix onto frustum */
    for (int j = 0; j < 2; j++) {
      double eyepts[3] = {std::tan(proj_half_angle) * dts[i + j],
                          aspect * std::tan(proj_half_angle) * dts[i + j], dts[i + j]};
      for (int k = 0; k < 4; k++) {
        eyepts[k % 2] *= -1;
        /* convert to world space */
        Coord3d resultA = useMatrix(mvm3t, Coord3d(eyepts)) + camera;

        /* reorient along light space. NOTE: this breaks total offset, but we don't care about
         * it...*/
        Coord3d result = useMatrix(light_align3, resultA);

        for (int m = 0; m < 3; m++) {
          bounds[m][0] = std::min(bounds[m][0], result[m]);
          bounds[m][1] = std::max(bounds[m][1], result[m]);
        }
      }
    }
    double dx = bounds[0][1] - bounds[0][0], dy = bounds[1][1] - bounds[1][0];  //,
    double rx = std::max(dx, dy);
    double ry = std::max(dx, dy);
    Coord3d midpoint(0, 0, -0.5 * (dts[i] + dts[i + 1]));
    Coord3d mmpt = useMatrix(mvm3t, midpoint) + camera;

    double s = 50; /* rel. the 100 of range */
    Coord3d sundir(activeView.sun_direction[0], activeView.sun_direction[1],
                   activeView.sun_direction[2]);
    Coord3d light_camera = mmpt - s * sundir;
    lookAtMatrix(light_camera[0], light_camera[1], light_camera[2], mmpt[0], mmpt[1], mmpt[2],
                 0, 0, 1, activeView.cascade_model[i]);

    /* like glOrtho; -L=R=rx, -B=T=ry, N=0,F=1000 */
    const double F = 1000.;
    Matrix4d orthoMtx = {
        {1 / rx, 0, 0, 0}, {0, 1 / ry, 0, 0}, {0, 0, -2. / F, 0}, {0, 0, 0, 1}};
    assign(orthoMtx, activeView.cascade_proj[i]);
  }

  /* Render cascade map using the given matrices */
  static int ltexsize = 0;
  if (activeView.cascadeTexsize <= 1 || ltexsize != Settings::settings->shadowTexsize) {
    ltexsize = Settings::settings->shadowTexsize;
    /* order doesn't matter */
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    GLint reqSize = 1 << Settings::settings->shadowTexsize;
    activeView.cascadeTexsize = std::min(maxSize, reqSize);
  }
  for (int i = 0; i < N; i++) {
    glBindTexture(GL_TEXTURE_2D, activeView.cascadeTexture[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, activeView.cascadeTexsize,
                 activeView.cascadeTexsize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glViewport(0, 0, activeView.cascadeTexsize, activeView.cascadeTexsize);
  activeView.calculating_shadows = true;

  GLuint cascadeFBOs[N];
  glGenFramebuffers(N, cascadeFBOs);
  for (int i = 0; i < N; i++) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cascadeFBOs[i]);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           activeView.cascadeTexture[i], 0);
    GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != result) {
      warning("Framebuffer is not complete. #%d w/err %x", i, result);
    }
  }

  glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
  // set Persp to square + angle
  for (int loop = 0; loop < N; ++loop) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cascadeFBOs[loop]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    assign(activeView.cascade_model[loop], activeView.modelview);
    assign(activeView.cascade_proj[loop], activeView.projection);
    markViewChanged();
    if (mp) mp->draw(0, focus, gm ? gm->gameTime : 0.);
    if (gm) gm->draw();
  }

  /* back to default (to screen) frame buffer */
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDeleteFramebuffers(N, cascadeFBOs);

  assign(origMV, activeView.modelview);
  assign(origProj, activeView.projection);
  activeView.calculating_shadows = false;
  markViewChanged();
}

/* generates a snapshot of the screen */
int createSnapshot() {
  static int snap_number = 0;
  char name[1024];
  int again = 9999;

  /* find the name for the image */
  do {
    snprintf(name, 1023, "./snapshot_%04d.png", snap_number++);
    FILE *f;
    if ((f = fopen(name, "r")) == NULL) {
      break;
    } else {
      fclose(f);
      again--;
    }
  } while (again > 0);

  /* Check against symlinks */
  if (pathIsLink(name)) {
    warning("file %s is a symlink.", name);
    return 0;
  }

  /* allocate buffer */
  unsigned char *buffer = new unsigned char[screenWidth * screenHeight * 4];
  unsigned char *linebuffer = new unsigned char[screenWidth * 4];
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

  /* get the screen */
  glReadPixels(0, 0, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)buffer);
  /* Flip pixels vertically */
  for (int i = 0; i < screenHeight / 2; i++) {
    memcpy(linebuffer, &buffer[i * screenWidth * 4], screenWidth * 4);
    memcpy(&buffer[i * screenWidth * 4], &buffer[(screenHeight - 1 - i) * screenWidth * 4],
           screenWidth * 4);
    memcpy(&buffer[(screenHeight - 1 - i) * screenWidth * 4], linebuffer, screenWidth * 4);
  }
  /* Construct surface and save */
  SDL_Surface *surf =
      SDL_CreateRGBSurfaceFrom(buffer, screenWidth, screenHeight, 32, screenWidth * 4, mask[0],
                               mask[1], mask[2], mask[3]);
  if (IMG_SavePNG(surf, name)) { warning("Failed to save screenshot to %s", name); }
  SDL_FreeSurface(surf);
  /* Cleanup */
  delete[] buffer;
  delete[] linebuffer;
  return 1;
}

/* Displays a centered semi-transparent sign with two text rows */
void message(char *A, char *B) {
  int size = 16;

  TTF_Font *font = menuFontForSize(size);

  int w1, w2, h1, h2, w;
  w1 = prepare2DString(font, A, true);
  w2 = prepare2DString(font, B, true);
  h1 = 32;
  h2 = 32;

  w = std::max(w1, w2) + 20;

  int x1 = screenWidth / 2 - w / 2, x2 = screenWidth / 2 + w / 2;
  int y1 = screenHeight / 2 - h1 - 5, y2 = screenHeight / 2 + h2 + 5;
  Enter2DMode();
  draw2DRectangle(x1, y1, x2 - x1, y2 - y1, 0., 0., 1., 1.,
                  Color(SRGBColor(0.2, 0.5, 0.2, 0.5)));

  drawCenterSimpleText(A, screenWidth / 2, screenHeight / 2 - size, size,
                       Color(SRGBColor(0.5, 1.0, 0.2, 1.0)));
  drawCenterSimpleText(B, screenWidth / 2, screenHeight / 2 + 14, size,
                       Color(SRGBColor(0.5, 1.0, 0.2, 1.0)));

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
  draw2DRectangle(x1, y1, x2 - x1, y2 - y1, 0., 0., 1., 1.,
                  Color(SRGBColor(0.2, 0.5, 0.2, 0.5)));

  h_now = -size;
  for (int i = 0; i < nlines; i++) {
    h_now += 2 * size;
    if (left[i]) {
      drawSimpleText(left[i], screenWidth / 2 - width / 2,
                     screenHeight / 2 - total_height / 2 + h_now, size,
                     Color(SRGBColor(0.5, 1.0, 0.2, 1.0)));
    }
    if (right[i]) {
      drawRightSimpleText(right[i], screenWidth / 2 + width / 2,
                          screenHeight / 2 - total_height / 2 + h_now, size,
                          Color(SRGBColor(0.5, 1.0, 0.2, 1.0)));
    }
  }
  Leave2DMode();
}

/** Loads a texture from file and returns a reference to it.
    It is safe to load the same texture multiple times since the results are cached
*/
int loadTexture(const char *name) {
  /* Check in cache if texture already loaded */
  for (int i = 0; i < numTextures; i++)
    if (strcmp(textureNames[i], name) == 0) return i;

  if (numTextures >= GLHELP_MAX_TEXTURES) {
    warning("Warning: max. number of textures reached (%d). Texture '%s' not loaded.",
            GLHELP_MAX_TEXTURES, name);
    return 0;
  }

  char str[256];
  snprintf(str, sizeof(str), "%s/images/%s", effectiveShareDir, name);
  SDL_Surface *surface = IMG_Load(str);
  if (!surface) {
    warning("Failed to load texture %s", str);
    // Override texture name... (to avoid carrying on outdated tex names)
    textureNames[numTextures] = strdup(textureNames[0]);
    textures[numTextures++] =
        textures[0];  // just assume we managed to load this, better than nothing
    return -1;
  } else {
    textureNames[numTextures] = strdup(name);
    GLfloat texCoord[4];
    textures[numTextures] = LoadTexture(surface, texCoord);
    numTextures++;
    SDL_FreeSurface(surface);
  }
  return (numTextures - 1);  // ok
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

static GLuint loadShaderPart(const char *name, GLuint shader_type) {
  char path[256];
  const char *tdesc = NULL;
  if (shader_type == GL_VERTEX_SHADER) {
    tdesc = "Vertex";
  } else if (shader_type == GL_FRAGMENT_SHADER) {
    tdesc = "Fragment";
  }

  snprintf(path, 256, "%s/shaders/%s", effectiveShareDir, name);
  GLchar *source = filetobuf(path);
  if (source == NULL) { error("%s shader %s could not be read", tdesc, path); }
  GLuint shader = glCreateShader(shader_type);
  if (shader == 0) { error("Failed to even create shader object"); }
  glShaderSource(shader, 1, (const GLchar **)&source, 0);
  free(source);

  glCompileShader(shader);
  int isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == 0) {
    int maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    char *vertexInfoLog = (char *)malloc(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, vertexInfoLog);
    warning("%s shader %s error (len %d): %s", tdesc, name, maxLength, vertexInfoLog);
    glDeleteShader(shader);
    free(vertexInfoLog);
    return 0;
  }
  return shader;
}

static GLint logUniformLocation(GLuint prog, const char *str) {
  GLint loc = glGetUniformLocation(prog, str);
  // fprintf(stderr, "prog %d, str '%s' -> %d\n", prog, str, loc);
  return loc;
}

static GLProgram linkShader(GLuint vertexshader, GLuint fragmentshader, const char *descr) {
  GLProgram prog = {};

  prog.prog = glCreateProgram();
  glAttachShader(prog.prog, vertexshader);
  glAttachShader(prog.prog, fragmentshader);
  /* Shaders use attribs 1..k for some k */
  glBindAttribLocation(prog.prog, 0, "in_Position");
  glBindAttribLocation(prog.prog, 1, "in_Color");
  glBindAttribLocation(prog.prog, 2, "in_Texcoord");
  glBindAttribLocation(prog.prog, 3, "in_Normal");
  glBindAttribLocation(prog.prog, 4, "in_Velocity");
  glLinkProgram(prog.prog);
  int is_linked;
  glGetProgramiv(prog.prog, GL_LINK_STATUS, (int *)&is_linked);
  glDeleteShader(vertexshader);
  glDeleteShader(fragmentshader);
  if (is_linked == 0) {
    int maxLength = 0;
    glGetProgramiv(prog.prog, GL_INFO_LOG_LENGTH, &maxLength);
    char *shaderProgramInfoLog = (char *)malloc(maxLength);
    glGetProgramInfoLog(prog.prog, maxLength, &maxLength, shaderProgramInfoLog);
    warning("Program (%s) link error (len %d): %s", descr, maxLength, shaderProgramInfoLog);
    free(shaderProgramInfoLog);
    prog.prog = 0;
    return prog;
  }

  /* Load all uniform locations. If the shader does not have the uniform,
   * value will be set to -1 */
  prog.locations.mvp_matrix = logUniformLocation(prog.prog, "mvp_matrix");
  prog.locations.model_matrix = logUniformLocation(prog.prog, "model_matrix");
  prog.locations.render_stage = logUniformLocation(prog.prog, "render_stage");
  prog.locations.gameTime = logUniformLocation(prog.prog, "gameTime");
  prog.locations.line_color = logUniformLocation(prog.prog, "line_color");
  prog.locations.arrtex = logUniformLocation(prog.prog, "arrtex");
  prog.locations.wtex = logUniformLocation(prog.prog, "wtex");
  prog.locations.specular = logUniformLocation(prog.prog, "specular");
  prog.locations.sharpness = logUniformLocation(prog.prog, "sharpness");
  prog.locations.ignore_shadow = logUniformLocation(prog.prog, "ignore_shadow");
  prog.locations.use_lighting = logUniformLocation(prog.prog, "use_lighting");
  prog.locations.refl_color = logUniformLocation(prog.prog, "refl_color");
  prog.locations.tex = logUniformLocation(prog.prog, "tex");
  prog.locations.fog_active = logUniformLocation(prog.prog, "fog_active");
  prog.locations.fog_color = logUniformLocation(prog.prog, "fog_color");
  prog.locations.fog_start = logUniformLocation(prog.prog, "fog_start");
  prog.locations.fog_end = logUniformLocation(prog.prog, "fog_end");
  prog.locations.light_ambient = logUniformLocation(prog.prog, "light_ambient");
  prog.locations.light_diffuse = logUniformLocation(prog.prog, "light_diffuse");
  prog.locations.light_specular = logUniformLocation(prog.prog, "light_specular");
  prog.locations.shadow_cascade0 = logUniformLocation(prog.prog, "shadow_cascade0");
  prog.locations.shadow_cascade1 = logUniformLocation(prog.prog, "shadow_cascade1");
  prog.locations.shadow_cascade2 = logUniformLocation(prog.prog, "shadow_cascade2");
  prog.locations.cascade_mvp = logUniformLocation(prog.prog, "cascade_mvp");
  prog.locations.cascade_model = logUniformLocation(prog.prog, "cascade_model");
  prog.locations.shadowtex_size = logUniformLocation(prog.prog, "shadowtex_size");
  prog.locations.sun_direction = logUniformLocation(prog.prog, "sun_direction");
  prog.locations.shadow_map = logUniformLocation(prog.prog, "shadow_map");
  prog.locations.light_position = logUniformLocation(prog.prog, "light_position");
  prog.locations.screen_width = logUniformLocation(prog.prog, "screen_width");
  prog.locations.screen_height = logUniformLocation(prog.prog, "screen_height");
  prog.locations.object_matrix = logUniformLocation(prog.prog, "object_matrix");

  return prog;
}

void glHelpInit() {
  warnForGLerrors("preGLinit");

  TTF_Init();
  char str[256];
  snprintf(str, sizeof(str), "%s/fonts/%s", effectiveShareDir, "menuFont.ttf");
  ingameFont = TTF_OpenFont(str, 30);
  if (!ingameFont) { error("failed to load font %s", str); }

  /* Note: all textures must be powers of 2 since we ignore texcoords */
  loadTexture("ice.png");
  loadTexture("acid.png");
  loadTexture("sand.png");
  textureTrack = textures[loadTexture("track.png")];
  loadTexture("texture.png");
  loadTexture("texture2.png");
  loadTexture("texture3.png");
  loadTexture("texture4.png");
  textureWings = textures[loadTexture("wings.png")];
  textureMousePointer = textures[loadTexture("mousePointer.png")];
  textureGlitter = textures[loadTexture("glitter.png")];
  textureDizzy = textures[loadTexture("dizzy.png")];
  textureBlank = textures[loadTexture("blank.png")];

  sparkle2D = new Sparkle2D();

  // Errors handled in loadShaderPart and linkShader
  GLuint vBasic = loadShaderPart("basic.vert", GL_VERTEX_SHADER);
  GLuint vLine = loadShaderPart("line.vert", GL_VERTEX_SHADER);
  GLuint vWater = loadShaderPart("water.vert", GL_VERTEX_SHADER);
  GLuint vUI = loadShaderPart("ui.vert", GL_VERTEX_SHADER);
  GLuint vObject = loadShaderPart("object.vert", GL_VERTEX_SHADER);
  GLuint vReflection = loadShaderPart("reflection.vert", GL_VERTEX_SHADER);

  GLuint fBasicDay = loadShaderPart("basic_day.frag", GL_FRAGMENT_SHADER);
  GLuint fBasicNight = loadShaderPart("basic_night.frag", GL_FRAGMENT_SHADER);
  GLuint fBasicShadow = loadShaderPart("basic_shadow.frag", GL_FRAGMENT_SHADER);
  GLuint fLine = loadShaderPart("line.frag", GL_FRAGMENT_SHADER);
  GLuint fWaterDay = loadShaderPart("water_day.frag", GL_FRAGMENT_SHADER);
  GLuint fWaterNight = loadShaderPart("water_night.frag", GL_FRAGMENT_SHADER);
  GLuint fUI = loadShaderPart("ui.frag", GL_FRAGMENT_SHADER);
  GLuint fObjectDay = loadShaderPart("object_day.frag", GL_FRAGMENT_SHADER);
  GLuint fObjectNight = loadShaderPart("object_night.frag", GL_FRAGMENT_SHADER);
  GLuint fObjectShadow = loadShaderPart("object_shadow.frag", GL_FRAGMENT_SHADER);
  GLuint fReflection = loadShaderPart("reflection.frag", GL_FRAGMENT_SHADER);

  shaderTileDay = linkShader(vBasic, fBasicDay, "BasicDay");
  shaderTileNight = linkShader(vBasic, fBasicNight, "BasicNight");
  shaderTileShadow = linkShader(vBasic, fBasicShadow, "BasicShadow");
  shaderLine = linkShader(vLine, fLine, "Line");
  shaderWaterDay = linkShader(vWater, fWaterDay, "WaterDay");
  shaderWaterNight = linkShader(vWater, fWaterNight, "WaterNight");
  shaderUI = linkShader(vUI, fUI, "UI");
  shaderObjectDay = linkShader(vObject, fObjectDay, "ObjectDay");
  shaderObjectNight = linkShader(vObject, fObjectNight, "ObjectNight");
  shaderObjectShadow = linkShader(vObject, fObjectShadow, "ObjectShadow");
  shaderReflection = linkShader(vReflection, fReflection, "Reflection");

  glDeleteShader(vBasic);
  glDeleteShader(vLine);
  glDeleteShader(vWater);
  glDeleteShader(vUI);
  glDeleteShader(vObject);
  glDeleteShader(vReflection);

  glDeleteShader(fBasicDay);
  glDeleteShader(fBasicNight);
  glDeleteShader(fBasicShadow);
  glDeleteShader(fLine);
  glDeleteShader(fWaterDay);
  glDeleteShader(fWaterNight);
  glDeleteShader(fUI);
  glDeleteShader(fObjectDay);
  glDeleteShader(fObjectNight);
  glDeleteShader(fObjectShadow);
  glDeleteShader(fReflection);

  // Wipe ball cache
  for (int i = 0; i < MAX_BALL_DETAIL; i++) {
    sphere_points[i] = NULL;
    sphere_texcos[i] = NULL;
    sphere_idxs[i] = NULL;
  }

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
  activeView.sun_direction[0] = 6 / 11.;
  activeView.sun_direction[1] = 34 / 121.;
  activeView.sun_direction[2] = -93 / 121.;

  activeView.calculating_shadows = false;
  activeView.show_flag_state = false;

  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glGenTextures(1, &activeView.shadowMapTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, activeView.shadowMapTexture);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  /* We don't use an array for the cascade texture because it reqs.
   * more shaders/etc */
  activeView.cascadeTexture[0] = 0;
  activeView.cascadeTexture[1] = 0;
  activeView.cascadeTexture[2] = 0;

  glGenTextures(3, activeView.cascadeTexture);
  for (int i = 0; i < 3; i++) {
    glBindTexture(GL_TEXTURE_2D, activeView.cascadeTexture[i]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  glGenTextures(1, &dummyCubeMapTexture);
  GLenum dirs[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};
  GLfloat buf[1] = {1.0f};
  glBindTexture(GL_TEXTURE_CUBE_MAP, dummyCubeMapTexture);
  for (int face = 0; face < 6; face++) {
    glTexImage2D(dirs[face], 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 buf);
  }

  glGenTextures(1, &dummyCascadeTexture);
  glBindTexture(GL_TEXTURE_2D, dummyCascadeTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
               buf);

  activeView.shadowMapTexsize = 1;
  activeView.cascadeTexsize = 1;
  activeView.day_mode = true;

  glEnable(GL_FRAMEBUFFER_SRGB);

  warnForGLerrors("postGLinit");
}
void glHelpCleanup() {
  if (shaderTileDay.prog) glDeleteProgram(shaderTileDay.prog);
  if (shaderTileNight.prog) glDeleteProgram(shaderTileNight.prog);
  if (shaderTileShadow.prog) glDeleteProgram(shaderTileShadow.prog);
  if (shaderLine.prog) glDeleteProgram(shaderLine.prog);
  if (shaderWaterDay.prog) glDeleteProgram(shaderWaterDay.prog);
  if (shaderWaterNight.prog) glDeleteProgram(shaderWaterNight.prog);
  if (shaderUI.prog) glDeleteProgram(shaderUI.prog);
  if (shaderObjectDay.prog) glDeleteProgram(shaderObjectDay.prog);
  if (shaderObjectNight.prog) glDeleteProgram(shaderObjectNight.prog);
  if (shaderObjectShadow.prog) glDeleteProgram(shaderObjectShadow.prog);
  if (shaderReflection.prog) glDeleteProgram(shaderReflection.prog);
  shaderLine.prog = 0;
  shaderTileDay.prog = 0;
  shaderTileNight.prog = 0;
  shaderTileShadow.prog = 0;
  shaderWaterDay.prog = 0;
  shaderWaterNight.prog = 0;
  shaderUI.prog = 0;
  shaderObjectDay.prog = 0;
  shaderObjectNight.prog = 0;
  shaderObjectShadow.prog = 0;

  if (sparkle2D) delete sparkle2D;

  for (int i = 0; i < MAX_BALL_DETAIL; i++) {
    if (sphere_points[i]) delete[] sphere_points[i];
    if (sphere_texcos[i]) delete[] sphere_texcos[i];
    if (sphere_idxs[i]) delete[] sphere_idxs[i];
  }

  /* Invalidate all strings so they get cleaned up */
  update2DStringCache(true);

  for (std::map<int, TTF_Font *>::iterator i = menuFontLookup.begin();
       i != menuFontLookup.end(); ++i) {
    TTF_CloseFont(i->second);
  }
  menuFontLookup.clear();
  TTF_CloseFont(ingameFont);
  ingameFont = 0;

  glDeleteTextures(1, &dummyCascadeTexture);
  glDeleteTextures(1, &dummyCubeMapTexture);

  for (int i = 0; i < numTextures; i++) { glDeleteTextures(1, &textures[i]); }
}

void warnForGLerrors(const char *where_am_i) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    const char *type = NULL;
    switch (err) {
    default:
      break;
    case GL_INVALID_ENUM:
      type = "Invalid enum";
      break;
    case GL_INVALID_VALUE:
      type = "Invalid value";
      break;
    case GL_INVALID_OPERATION:
      type = "Invalid operation";
      break;
    }

    warning("GL error %x (%s) at location: %s", err, type, where_am_i);
  }
}

/* Calculates and displays current framerate */
void displayFrameRate() {
  /* Frame rate is given for the previous period. */
  double td = getTimeDifference(lastDisplayStartTime, displayStartTime) / timeDilationFactor;
  if (td > 1.0) {
    fps = 1.0;
  } else if (td <= 1e-4) {
    static int hasWarned = 0;
    if (!hasWarned) {
      warning("Warning: too fast framerate (%f)", td);
      hasWarned = 1;
    }
  } else {
    fps = fps * 0.95 + 0.05 / td;
  }

  if (Settings::settings->showFPS > 0) {
    char slow[256];
    char fast[256];
    if (Settings::settings->showFPS == 1) {
      snprintf(slow, sizeof(slow), "%s ", _("Framerate:"));
      if (fps > 0) {
        snprintf(fast, sizeof(fast), "%.1f", fps);
      } else {
        snprintf(fast, sizeof(fast), _("unknown"));
      }
    } else {
      snprintf(slow, sizeof(slow), "%s ", _("ms/Frame:"));
      snprintf(fast, sizeof(fast), "%.1f", 1e3 * td);
    }
    TTF_Font *active = menuFontForSize(10);
    int w1 = draw2DString(active, slow, 15, screenHeight - 15, menuColor, true, 0, 0);
    /* Write rate letter by letter; this ensures they remain in cache and avoids
     * rerendering */
    int pos = 15 + w1;
    for (char *cur = fast; *cur; cur++) {
      char letter[2] = {*cur, 0};
      pos += draw2DString(active, letter, pos, screenHeight - 15, menuColor, true, 0, 0) - 4;
    }
  }
}

/*********************/
/* Matrix operations */
/*********************/

void debugMatrix(Matrix4d m) {
  warning("%8.5f \t%8.5f \t%8.5f \t%8.5f", m[0][0], m[0][1], m[0][2], m[0][3]);
  warning("%8.5f \t%8.5f \t%8.5f \t%8.5f", m[1][0], m[1][1], m[1][2], m[1][3]);
  warning("%8.5f \t%8.5f \t%8.5f \t%8.5f", m[2][0], m[2][1], m[2][2], m[2][3]);
  warning("%8.5f \t%8.5f \t%8.5f \t%8.5f", m[3][0], m[3][1], m[3][2], m[3][3]);
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
Coord3d useMatrix(Matrix4d A, const Coord3d &B) {
  Coord3d C;
  for (int i = 0; i < 3; i++) {
    C[i] = A[i][3];
    for (int k = 0; k < 3; k++) C[i] += A[i][k] * B[k];
  }
  double h = A[3][3];
  for (int k = 0; k < 3; k++) h += A[3][k];
  for (int k = 0; k < 3; k++) C[k] /= h;
  return C;
}

/* C <- A(B) */
Coord3d useMatrix(Matrix3d A, const Coord3d &B) {
  Coord3d C;
  for (int i = 0; i < 3; i++) {
    for (int k = 0; k < 3; k++) C[i] += A[i][k] * B[k];
  }
  return C;
}

/* C <- A */
void assign(const Matrix4d A, Matrix4d C) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) C[i][j] = A[i][j];
}

/* C <- A */
void identityMatrix(Matrix4d m) {
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) m[i][j] = i == j ? 1.0 : 0.0;
}

void affineMatrix(Matrix4d m, const Matrix3d r, const Coord3d &p) {
  m[0][0] = r[0][0];
  m[0][1] = r[0][1];
  m[0][2] = r[0][2];

  m[1][0] = r[1][0];
  m[1][1] = r[1][1];
  m[1][2] = r[1][2];

  m[2][0] = r[2][0];
  m[2][1] = r[2][1];
  m[2][2] = r[2][2];

  m[0][3] = p[0];
  m[1][3] = p[1];
  m[2][3] = p[2];

  m[3][0] = 0.;
  m[3][1] = 0.;
  m[3][2] = 0.;
  m[3][3] = 1.;
}

void rotateX(double v, Matrix4d m) {
  double cv = std::cos(v), sv = std::sin(v);
  for (int i = 0; i < 4; i++) {
    double r1 = m[i][1], r2 = m[i][2];
    m[i][1] = cv * r1 - sv * r2;
    m[i][2] = sv * r1 + cv * r2;
  }
}
void rotateY(double v, Matrix4d m) {
  double cv = std::cos(v), sv = std::sin(v);
  for (int i = 0; i < 4; i++) {
    double r0 = m[i][0], r2 = m[i][2];
    m[i][0] = cv * r0 - sv * r2;
    m[i][2] = sv * r0 + cv * r2;
  }
}
void rotateZ(double v, Matrix4d m) {
  double cv = std::cos(v), sv = std::sin(v);
  for (int i = 0; i < 4; i++) {
    double r0 = m[i][0], r1 = m[i][1];
    m[i][0] = cv * r0 - sv * r1;
    m[i][1] = sv * r0 + cv * r1;
  }
}
bool testBboxClip(double x1, double x2, double y1, double y2, double z1, double z2,
                  const Matrix4d mvp) {
  // Construct axis-aligned bounding box in window space
  double vxl = 1e99, vyl = 1e99, vzl = 1e99;
  double vxh = -1e99, vyh = -1e99, vzh = -1e99;
  for (int i = 0; i < 8; i++) {
    double w[3] = {0., 0., 0.};
    double p[3] = {(i & 4) ? x1 : x2, (i & 2) ? y1 : y2, (i & 1) ? z1 : z2};
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
  if (vxl > 1 || vxh < -1) return false;
  if (vyl > 1 || vyh < -1) return false;
  if (vzl > 1 || vzh < 0) return false;
  return true;
}

static bool is_2d_mode = false;

void Require2DMode() {
  if (!is_2d_mode) { warning("Function `Require2DMode` should only be called in 2D mode"); }
}

void Enter2DMode() {
  if (is_2d_mode) { return; }

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Load program and set up uniforms. VAO will by bound at point of use
  glUseProgram(shaderUI.prog);
  glUniform1f(shaderUI.locations.screen_width, (GLfloat)screenWidth);
  glUniform1f(shaderUI.locations.screen_height, (GLfloat)screenHeight);
  glUniform1i(shaderUI.locations.tex, 0);

  glActiveTexture(GL_TEXTURE0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  is_2d_mode = true;
}

void Leave2DMode() {
  if (!is_2d_mode) {
    warning("Function `Leave2DMode` should only be called in 2D mode");
    return;
  }

  is_2d_mode = false;
}

int powerOfTwo(int input) {
  int value = 1;

  while (value < input) { value <<= 1; }
  return value;
}

GLuint LoadTexture(SDL_Surface *surface, GLfloat *texcoord) {
  /* Use the surface width and height expanded to powers of 2 */
  int w = powerOfTwo(surface->w);
  int h = powerOfTwo(surface->h);

  texcoord[0] = 0.0f;                    /* Min X */
  texcoord[1] = 0.0f;                    /* Min Y */
  texcoord[2] = (GLfloat)surface->w / w; /* Max X */
  texcoord[3] = (GLfloat)surface->h / h; /* Max Y */

  /* Rescale image if needed? */
  GLint maxSize;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
  double scale = 1.0;
  if (w > maxSize || h > maxSize) scale = std::min(maxSize / (double)w, maxSize / (double)h);

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

  SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)(w * scale), (int)(h * scale),
                                            32, mask[0], mask[1], mask[2], mask[3]);

  if (image == NULL) { return 0; }

  /* Copy the surface into the GL texture image */
  SDL_Rect area;
  area.x = 0;
  area.y = 0;
  area.w = (int)(surface->w * scale);
  area.h = (int)(surface->h * scale);

  SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
  if (scale == 1.0)
    // Easy, no scaling needed
    SDL_BlitSurface(surface, NULL, image, &area);
  else {
    SDL_Rect sarea;
    sarea.x = 0;
    sarea.y = 0;
    sarea.h = surface->h;
    sarea.w = surface->w;
    SDL_BlitScaled(surface, &sarea, image, &area);
  }

  /* Create an OpenGL texture for the image */
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, (int)(w * scale), (int)(h * scale), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
  SDL_FreeSurface(image); /* No longer needed */

  /* Set current sampler with these just in case */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return texture;
}

SDL_Surface *loadImage(const char *imagename) {
  char path[512];
  snprintf(path, 511, "%s/images/%s", effectiveShareDir, imagename);
  path[511] = '\0';

  SDL_Surface *img = IMG_Load(path);
  if (!img) error("Failed to load image '%s' because: %s", path, IMG_GetError());
  return img;
}
