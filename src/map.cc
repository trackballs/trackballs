/** \file map.cc
   Handles all information about the current map, including the individual cells.
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

#include "map.h"

#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_image.h>
#include <zlib.h>
#include <cstdlib>

/* VISRADIUS is half-width of square of drawable cells */
#define VISRADIUS 50
#define CHUNKSIZE 32

#define WRITE_PORTABLE 1
#define READ_PORTABLE 1

const int Map::flagNone = 0, Map::flagFlashCenter = 1, Map::flagTranslucent = 2,
          Map::flagShowCross = 4;
const int Map::mapFormatVersion = 7;

inline int32_t saveInt(int32_t v) { return (int32_t)SDL_SwapBE32((uint32_t)v); }
inline int32_t loadInt(int32_t v) { return (int32_t)SDL_SwapBE32((uint32_t)v); }

/* initialization of some fields to be sure... */
Cell::Cell() {
  texture = -1;
  flags = 0;
  for (int i = 0; i < 5; i++) colors[i] = SRGBColor(1., 1., 1., 1.);
  for (int i = 0; i < 4; i++) wallColors[i] = SRGBColor(1., 1., 1., 1.);
  for (int i = 0; i < 5; i++) heights[i] = -8.0;
  for (int i = 0; i < 5; i++) waterHeights[i] = -20.0;
  velocity[0] = 0.;
  velocity[1] = 1.;
  sunken = 0.;
  displayListDirty = false;
}

static void calcNormals(const float heights[5], Coord3d normals[5]) {
  Coord3d spines[4] = {
      Coord3d(-0.5, -0.5, heights[Cell::SW] - heights[Cell::CENTER]),
      Coord3d(0.5, -0.5, heights[Cell::SE] - heights[Cell::CENTER]),
      Coord3d(0.5, 0.5, heights[Cell::NE] - heights[Cell::CENTER]),
      Coord3d(-0.5, 0.5, heights[Cell::NW] - heights[Cell::CENTER]),
  };

  Coord3d faceNormals[4];
  for (int i = 0; i < 4; i++) {
    faceNormals[i] = crossProduct(spines[i], spines[(i + 1) % 4]);
    faceNormals[i] = faceNormals[i] / length(faceNormals[i]);
  }
  for (int i = 0; i < 5; i++) { normals[i] = Coord3d(); }
  for (int i = 0; i < 4; i++) {
    normals[i] = normals[i] + faceNormals[i];
    normals[i] = normals[i] + faceNormals[(i + 3) % 4];
    normals[4] = normals[4] + faceNormals[i];
  }
  for (int i = 0; i < 5; i++) { normals[i] = normals[i] / length(normals[i]); }
}

/* Returns the average normal at a vertex of the cell */
void Cell::getNormals(Coord3d normals[5]) const { calcNormals(heights, normals); }
/* Works on water heights */
void Cell::getWaterNormals(Coord3d normals[5]) const { calcNormals(waterHeights, normals); }

static Real calcHeight(const float heights[5], Real x, Real y) {
  x = x - 0.5f;
  y = y - 0.5f;

  Real rp = std::abs(x + y);
  Real rm = std::abs(x - y);
  Real ph = y > -x ? heights[Cell::NE] : heights[Cell::SW];
  Real mh = y > x ? heights[Cell::NW] : heights[Cell::SE];
  Real ch = heights[Cell::CENTER];

  return ch * (1 - rp - rm) + ph * rp + mh * rm;
}

/* Gives the height of the cell in a specified (floatingpoint) position */
Real Cell::getHeight(Real x, Real y) const { return calcHeight(heights, x, y); }

/* Works for water */
Real Cell::getWaterHeight(Real x, Real y) const { return calcHeight(waterHeights, x, y); }

Chunk::Chunk() {
  is_active = false;
  is_visible = false;
  is_updated = true;
  last_shown = -1;
  // Init with extreme range to ensure visibility after which
  // exact range would be calculated
  maxHeight = 1e3;
  minHeight = 1e-3;
  xm = 0;
  ym = 0;
}

Chunk::~Chunk() {
  if (is_active) {
    glDeleteBuffers(3, &tile_vbo[0]);
    glDeleteBuffers(3, &wall_vbo[0]);
    glDeleteBuffers(2, &flag_vbo[0]);
    glDeleteBuffers(2, &flui_vbo[0]);
    glDeleteBuffers(2, &line_vbo[0]);
    glDeleteVertexArrays(1, &tile_vao);
    glDeleteVertexArrays(1, &tile_alpha_vao);
    glDeleteVertexArrays(1, &wall_vao);
    glDeleteVertexArrays(1, &wall_alpha_vao);
    glDeleteVertexArrays(1, &flag_vao);
    glDeleteVertexArrays(1, &flui_vao);
    glDeleteVertexArrays(1, &line_vao);
  }
}

static GLuint createTextureArray(const char** texs, int ntex, int size) {
  GLuint texture_Array = 0;
  glGenTextures(1, &texture_Array);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_Array);
  char* data = new char[ntex * size * size * 4];
  for (int i = 0; i < ntex; i++) {
    /* loadImage aborts with error if any of the above textures DNE */
    SDL_Surface* orig = loadImage(texs[i]);
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
    SDL_Surface* proj = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32, mask[0], mask[1],
                                             mask[2], mask[3]);
    SDL_Rect orect = {0, 0, orig->w, orig->h};
    SDL_Rect drect = {0, 0, size, size};
    SDL_SetSurfaceBlendMode(orig, SDL_BLENDMODE_NONE);
    SDL_BlitScaled(orig, &orect, proj, &drect);
    SDL_FreeSurface(orig);
    SDL_LockSurface(proj);
    memcpy(&data[i * size * size * 4], proj->pixels, size * size * 4);
    SDL_UnlockSurface(proj);
    SDL_FreeSurface(proj);
  }
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8_ALPHA8, size, size, ntex, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);
  delete[] data;

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_Array);
  return texture_Array;
}

class ChunkStagingBuffers {
 public:
  /* tiles */
  GLfloat tdat[CHUNKSIZE * CHUNKSIZE * 5 * 8];
  ushort toidx[CHUNKSIZE * CHUNKSIZE * 12];  // opaque
  ushort taidx[CHUNKSIZE * CHUNKSIZE * 12];  // alpha
  /* walls */
  GLfloat wdat[CHUNKSIZE * CHUNKSIZE * 8 * 8];
  ushort woidx[CHUNKSIZE * CHUNKSIZE * 12];  // opaque
  ushort waidx[CHUNKSIZE * CHUNKSIZE * 12];  // alpha
  /* flags */
  GLfloat sdat[CHUNKSIZE * CHUNKSIZE * 5 * 8];
  ushort sidx[CHUNKSIZE * CHUNKSIZE * 12];
  /* lines */
  GLfloat ldat[CHUNKSIZE * CHUNKSIZE * 4 * 3];
  ushort lidx[CHUNKSIZE * CHUNKSIZE * 8];
  /* water */
  GLfloat fdat[CHUNKSIZE * CHUNKSIZE * 8 * 5];
  ushort fidx[CHUNKSIZE * CHUNKSIZE * 12];
};

Map::Map(char* filename) {
  gzFile gp;
  int x, y, i;

  isBonus = 0;

  memset(mapname, 0, sizeof(mapname));
  memset(author, 0, sizeof(author));
  gp = gzopen(filename, "rb");
  if (gp) {
    int version;
    int32_t data[6];
    gzread(gp, data, sizeof(int32_t) * 6);
    for (i = 0; i < 3; i++) startPosition[i] = 0.5 + loadInt(data[i]);
    width = loadInt(data[3]);
    height = loadInt(data[4]);
    version = loadInt(data[5]);

    if (version < mapFormatVersion)
      warning("Warning. Map %s is of an old format (v%d, latest is v%d)", filename, version,
              mapFormatVersion);
    else if (version > mapFormatVersion) {
      error(
          "Error. Map %s is from the future (v%d, I know only format v%d)\n"
          "This error usually occurs because or broken maps or big/small endian issues",
          filename, version, mapFormatVersion);
    }

    if (version >= 7) { /* Read texture indices */
      gzread(gp, data, sizeof(int32_t) * 1);
      int nt = loadInt(data[0]);  // num textures used in map
      char textureName[64];
      for (i = 0; i < nt; i++) {
        gzread(gp, textureName, 64);
        indexTranslation[i] = loadTexture(textureName);
      }
    } else  // for old maps we just assume that all loaded textures are in the same order as
            // from creator
      for (i = 0; i < 256; i++) indexTranslation[i] = i;

    cells = new Cell[width * height];

    for (y = 0; y < height; y++)
      for (x = 0; x < width; x++) {
        // We have to do this here since Cell::load does not know it's own coordinates
        Cell& c = cell(x, y);
        c.load(this, gp, version);
      }
    gzclose(gp);
  } else {
    warning("could not open %s", filename);
    width = height = 256;
    cells = new Cell[width * height];
    startPosition[0] = startPosition[1] = 252;
    startPosition[2] = 0.0;
    for (x = 0; x < width; x++)
      for (y = 0; y < height; y++) {
        Cell& c = cells[x + y * width];

        c.flags = 0;
        for (i = 0; i < 5; i++) {
          c.heights[i] = -8.0;
          c.waterHeights[i] = -8.5;  // this is the groundwater =)
          c.colors[i] = SRGBColor(0.9, 0.9, 0.9, 1.0);
        }
        for (i = 0; i < 4; i++) { c.wallColors[i] = SRGBColor(0.7, 0.2, 0.2, 1.0); }
        c.velocity[0] = 0.0;
        c.velocity[1] = 0.0;
      }
  }

  /* Fix display lists used by each cell */
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      Cell& c = cells[x + y * width];
      c.displayListDirty = true;
    }

  flags = flagNone;
  startPosition[2] = getHeight(startPosition[0], startPosition[1]);

  tx_Ice = loadTexture("ice.png");
  tx_Acid = loadTexture("acid.png");
  tx_Sand = loadTexture("sand.png");
  tx_Track = loadTexture("track.png");
  tx_1 = loadTexture("texture.png");
  tx_2 = loadTexture("texture2.png");
  tx_3 = loadTexture("texture3.png");
  tx_4 = loadTexture("texture4.png");
  tx_Water = loadTexture("water.png");

  chunks.clear();

  // Construct texture array
  const int nsubtextures = 9;
  const int size = 1 << 8;
  const char* texs[nsubtextures] = {"blank.png",    "ice.png",      "sand.png",
                                    "acid.png",     "track.png",    "texture.png",
                                    "texture2.png", "texture3.png", "texture4.png"};
  texture_Array = createTextureArray(texs, nsubtextures, size);

  const int nflags = 8;
  const char* flags[nflags] = {"transparent.png",     "cell_multi.png", "cell_ice.png",
                               "cell_sand.png",       "cell_track.png", "cell_acid.png",
                               "cell_trampoline.png", "cell_kill.png"};
  flag_Array = createTextureArray(flags, nflags, size);

  bufs = new ChunkStagingBuffers();
}
Map::~Map() {
  delete[] cells;
  chunks.clear();
  delete bufs;

  glDeleteTextures(1, &texture_Array);
  glDeleteTextures(1, &flag_Array);
}

static void configureCellAttributes(bool water) {
  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
  // Color
  glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                        (void*)(3 * sizeof(GLfloat)));
  // Texture
  if (water) {
    glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(5 * sizeof(GLfloat)));
  } else {
    glVertexAttribPointer(2, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(5 * sizeof(GLfloat)));
  }
  // Normal
  glVertexAttribPointer(3, 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, 8 * sizeof(GLfloat),
                        (void*)(6 * sizeof(GLfloat)));
  // Velocity
  glVertexAttribPointer(4, 2, GL_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                        (void*)(7 * sizeof(GLfloat)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);
}

/* Draws the map on the screen from current viewpoint */
void Map::draw(int stage, const Coord3d& focus, GLfloat gameTime) {
  if (stage == 0) {
    glDisable(GL_BLEND);
  } else {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  int cx = (int)focus[0], cy = (int)focus[1];
  int origx = cx - cx % CHUNKSIZE, origy = cy - cy % CHUNKSIZE;
  int prad = (VISRADIUS / CHUNKSIZE) + 1;

  Matrix4d mvp;
  matrixMult(activeView.modelview, activeView.projection, mvp);

  int nchunks = 0;
  Chunk* drawlist[1024];
  for (int i = -prad; i <= prad; i++) {
    for (int j = -prad; j < +prad; j++) {
      int hx = origx + i * CHUNKSIZE, hy = origy + j * CHUNKSIZE;
      Chunk* cur = chunk(hx, hy);
      int update = 0;
      if (stage == 0) {
        // Detect if update needed.
        update = cur->is_updated;

        int visible = testBboxClip(cur->xm, cur->xm + CHUNKSIZE, cur->ym, cur->ym + CHUNKSIZE,
                                   cur->minHeight, cur->maxHeight, mvp);

        // Current cell is in viewport
        int ox = hx + CHUNKSIZE / 2;
        int oy = hy + CHUNKSIZE / 2;
        int inrad = (ox - cx) * (ox - cx) + (oy - cy) * (oy - cy) < 2 * VISRADIUS * VISRADIUS;
        visible = visible && inrad;

        cur->is_visible = visible;
      }
      if (cur->is_visible) {
        cur->last_shown = displayFrameNumber;
        drawlist[nchunks] = cur;
        if (update || !cur->is_active) { fillChunkVBO(drawlist[nchunks]); }
        nchunks++;
      } else {
        // Cleanup buffers for zones that have long since dropped out of view
        if (cur->is_active && cur->last_shown < displayFrameNumber - 10) {
          glDeleteBuffers(3, &cur->tile_vbo[0]);
          glDeleteBuffers(3, &cur->wall_vbo[0]);
          glDeleteBuffers(2, &cur->flag_vbo[0]);
          glDeleteBuffers(2, &cur->flui_vbo[0]);
          glDeleteBuffers(2, &cur->line_vbo[0]);
          glDeleteVertexArrays(1, &cur->tile_vao);
          glDeleteVertexArrays(1, &cur->tile_alpha_vao);
          glDeleteVertexArrays(1, &cur->wall_vao);
          glDeleteVertexArrays(1, &cur->wall_alpha_vao);
          glDeleteVertexArrays(1, &cur->flag_vao);
          glDeleteVertexArrays(1, &cur->flui_vao);
          glDeleteVertexArrays(1, &cur->line_vao);
          cur->is_active = false;
        }
      }
    }
  }

  // Put into shader
  const UniformLocations* uloc = setActiveProgramAndUniforms(Shader_Tile);
  glUniform1f(uloc->gameTime, gameTime);

  // Link in texture atlas :-)
  glUniform1i(uloc->arrtex, 0);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture_Array);

  // Run through ye olde draw loop
  if (stage == 0) {
    // WALLS
    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->wall_vao);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }

    // TOPS
    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->tile_vao);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }
  } else {  // stage == 1
    // WALLS
    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->wall_alpha_vao);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }

    // TOPS
    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->tile_alpha_vao);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }
  }

  if (stage == 1 && activeView.show_flag_state) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, flag_Array);

    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->flag_vao);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }
  }

  if (stage == 1 && !activeView.calculating_shadows) {
    uloc = setActiveProgramAndUniforms(Shader_Water);
    glUniform1f(uloc->gameTime, gameTime);
    glUniform1i(uloc->wtex, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Water]);

    // Water
    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->flui_vao);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }
  }

  if (!activeView.calculating_shadows) {
    uloc = setActiveProgramAndUniforms(Shader_Line);
    glUniformC(uloc->line_color, Color(0., 0., 0., 1.));

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);
    for (int i = 0; i < nchunks; i++) {
      glBindVertexArray(drawlist[i]->line_vao);
      glDrawElements(GL_LINES, CHUNKSIZE * CHUNKSIZE * 8, GL_UNSIGNED_SHORT, (void*)0);
    }
    glDisable(GL_LINE_SMOOTH);
  }

  // Don't keep any VAOs bound/overwritable
  glBindVertexArray(0);
  warnForGLerrors(stage ? "Map drawing 1" : "Map drawing 0");
}

static inline void packCell(GLfloat* fout, GLfloat px, GLfloat py, GLfloat pz, Color color,
                            GLfloat tx, GLfloat ty, int txno, float velx, float vely,
                            const GLfloat normal[3]) {
  uint32_t* aout = (uint32_t*)fout;
  fout[0] = px;
  fout[1] = py;
  fout[2] = pz;
  aout[3] = ((uint32_t)(65535.f * color.v[1]) << 16) + (uint32_t)(65535.f * color.v[0]);
  aout[4] = ((uint32_t)(65535.f * color.v[3]) << 16) + (uint32_t)(65535.f * color.v[2]);

  uint32_t txco = ((uint32_t)(1023.f * ty) << 10) | ((uint32_t)(1023.f * tx) << 0) |
                  ((uint32_t)(1023.f * (txno / 16.f)) << 20);
  aout[5] = txco;
  aout[6] = packNormal(normal);

  int vx = std::max(std::min((int)(32677.f * (velx)), 32677), -32677);
  int vy = std::max(std::min((int)(32677.f * (vely)), 32677), -32677);
  aout[7] = ((uint32_t)vy << 16) + (uint32_t)vx;
}
static inline void packWaterCell(GLfloat* fout, GLfloat px, GLfloat py, GLfloat pz,
                                 const float vel[2], GLfloat tx, GLfloat ty,
                                 const GLfloat normal[3]) {
  uint32_t* aout = (uint32_t*)fout;
  fout[0] = px;
  fout[1] = py;
  fout[2] = pz;
  aout[3] = -1;
  aout[4] = -1;
  aout[5] = (((uint32_t)(65535.f * ty)) << 16) + (uint32_t)(65535.f * tx);
  aout[6] = packNormal(normal);
  int vx = std::max(std::min((int)(32677.f * (0.25f * vel[0])), 32677), -32677);
  int vy = std::max(std::min((int)(32677.f * (0.25f * vel[1])), 32677), -32677);
  aout[7] = ((uint32_t)vy << 16) + (uint32_t)vx;
}

static inline int cmp(float l, float r) {
  if (l > r) return 1;
  if (r > l) return -1;
  return 0;
}

void Map::fillChunkVBO(Chunk* chunk) const {
  // Create data if not already there
  bool first_time = false;
  if (!chunk->is_active) {
    glGenBuffers(3, &chunk->tile_vbo[0]);
    glGenBuffers(3, &chunk->wall_vbo[0]);
    glGenBuffers(2, &chunk->flag_vbo[0]);
    glGenBuffers(2, &chunk->line_vbo[0]);
    glGenBuffers(2, &chunk->flui_vbo[0]);
    glGenVertexArrays(1, &chunk->tile_vao);
    glGenVertexArrays(1, &chunk->tile_alpha_vao);
    glGenVertexArrays(1, &chunk->wall_vao);
    glGenVertexArrays(1, &chunk->wall_alpha_vao);
    glGenVertexArrays(1, &chunk->flag_vao);
    glGenVertexArrays(1, &chunk->line_vao);
    glGenVertexArrays(1, &chunk->flui_vao);
    first_time = true;
  }
  chunk->is_active = true;

  // Update exact chunk bounds as well, and mark all first-timers as
  // updated
  GLfloat minz = 1e99, maxz = -1e99;
  for (int x = chunk->xm; x < chunk->xm + CHUNKSIZE; x++) {
    for (int y = chunk->ym; y < chunk->ym + CHUNKSIZE; y++) {
      Cell& c = cell(x, y);
      c.displayListDirty |= first_time;
      for (int k = 0; k < 5; k++) {
        minz = std::min(minz, c.heights[k]);
        minz = std::min(minz, c.waterHeights[k]);
        maxz = std::max(maxz, c.heights[k]);
        maxz = std::max(maxz, c.waterHeights[k]);
      }
    }
  }
  int jstart = -1;
  for (int j = 0; j < CHUNKSIZE * CHUNKSIZE; j++) {
    int x = chunk->xm + j % CHUNKSIZE;
    int y = chunk->ym + j / CHUNKSIZE;
    Cell& c = cell(x, y);
    if (c.displayListDirty) {
      if (jstart < 0) { jstart = j; }

      int txno = 0;
      int typed = c.flags & (CELL_ICE | CELL_ACID | CELL_TRACK | CELL_SAND);
      if (typed || c.texture >= 0) {
        if ((c.flags & CELL_ICE) || (!typed && c.texture == tx_Ice)) {
          txno = 1;
        } else if ((c.flags & CELL_SAND) || (!typed && c.texture == tx_Sand)) {
          txno = 2;
        } else if ((c.flags & CELL_TRACK) || (!typed && c.texture == tx_Track)) {
          txno = 4;
        } else if ((c.flags & CELL_ACID) || (!typed && c.texture == tx_Acid)) {
          txno = 3;
        } else if (c.texture == tx_1) {
          txno = 5;
        } else if (c.texture == tx_2) {
          txno = 6;
        } else if (c.texture == tx_3) {
          txno = 7;
        } else if (c.texture == tx_4) {
          txno = 8;
        }
      } else {
        // Nothing much...
      }

      GLfloat nnormal[3] = {0.f, 1.f, 0.f};
      GLfloat snormal[3] = {0.f, -1.f, 0.f};
      GLfloat enormal[3] = {-1.f, 0.f, 0.f};
      GLfloat wnormal[3] = {1.f, 0.f, 0.f};

      float fcnormal[5][3];
      if (c.flags & CELL_SHADE_FLAT) {
        for (size_t i = 0; i < 15; i++) { fcnormal[i / 3][i % 3] = 0.f; }
      } else {
        Coord3d cnormal[5];
        c.getNormals(cnormal);
        for (size_t i = 0; i < 15; i++) {
          fcnormal[i / 3][i % 3] = (float)cnormal[i / 3][i % 3];
        }
      }

      int k = j * 5 * 8;
      const int texscale = 3.;
      const float irs = 1.f / texscale;
      // ((. % t)+t)%t handles negative numbers
      GLfloat tx = (((x % texscale) + texscale) % texscale) * irs;
      GLfloat ty = (((y % texscale) + texscale) % texscale) * irs;
      if (tx >= 1.) tx = 0.;
      if (ty >= 1.) ty = 0.;
      packCell(&bufs->tdat[k], x, y, c.heights[Cell::SW], Color(c.colors[Cell::SW]), tx, ty,
               txno, c.velocity[0] * irs, c.velocity[1] * irs, fcnormal[Cell::SW]);
      packCell(&bufs->tdat[k + 8], x + 1, y, c.heights[Cell::SE], Color(c.colors[Cell::SE]),
               tx + irs, ty, txno, c.velocity[0] * irs, c.velocity[1] * irs,
               fcnormal[Cell::SE]);
      packCell(&bufs->tdat[k + 16], x + 1, y + 1, c.heights[Cell::NE],
               Color(c.colors[Cell::NE]), tx + irs, ty + irs, txno, c.velocity[0] * irs,
               c.velocity[1] * irs, fcnormal[Cell::NE]);
      packCell(&bufs->tdat[k + 24], x, y + 1, c.heights[Cell::NW], Color(c.colors[Cell::NW]),
               tx, ty + irs, txno, c.velocity[0] * irs, c.velocity[1] * irs,
               fcnormal[Cell::NW]);
      packCell(&bufs->tdat[k + 32], x + 0.5f, y + 0.5f, c.heights[Cell::CENTER],
               Color(c.colors[Cell::CENTER]), tx + irs / 2, ty + irs / 2, txno,
               c.velocity[0] * irs, c.velocity[1] * irs, fcnormal[Cell::CENTER]);

      const ushort topindices[4][3] = {{0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}};
      for (uint f = 0; f < 4; f++) {
        int corners[] = {Cell::SW, Cell::SE, Cell::NE, Cell::NW, Cell::CENTER};
        bool is_opaque = c.colors[corners[topindices[f][0]]].isOpaque() &&
                         c.colors[corners[topindices[f][1]]].isOpaque() &&
                         c.colors[corners[topindices[f][2]]].isOpaque();

        if (is_opaque) {
          for (uint i = 0; i < 3; i++) {
            bufs->toidx[j * 12 + 3 * f + i] = 5 * j + topindices[f][i];
          }
          for (uint i = 0; i < 3; i++) { bufs->taidx[j * 12 + 3 * f + i] = 0; }
        } else {
          for (uint i = 0; i < 3; i++) { bufs->toidx[j * 12 + 3 * f + i] = 0; }
          for (uint i = 0; i < 3; i++) {
            bufs->taidx[j * 12 + 3 * f + i] = 5 * j + topindices[f][i];
          }
        }
      }

      if (activeView.show_flag_state) {
        Color white(1.f, 1.f, 1.f, 1.f);
        float height_boost = 0.001f;

        int flag_list[6] = {CELL_ICE,  CELL_SAND,       CELL_TRACK,
                            CELL_ACID, CELL_TRAMPOLINE, CELL_KILL};

        int flagno = 0;
        int nflags = 0;
        for (int k = 0; k < 6; k++) {
          if (c.flags & flag_list[k]) {
            nflags++;
            flagno = k + 2;
          }
        }
        if (nflags > 1) { flagno = 1; }

        packCell(&bufs->sdat[k], x, y, c.heights[Cell::SW] + height_boost, white, 0.f, 0.f,
                 flagno, 0.f, 0.f, fcnormal[Cell::SW]);
        packCell(&bufs->sdat[k + 8], x + 1, y, c.heights[Cell::SE] + height_boost, white, 1.f,
                 0.f, flagno, 0.f, 0.f, fcnormal[Cell::SE]);
        packCell(&bufs->sdat[k + 16], x + 1, y + 1, c.heights[Cell::NE] + height_boost, white,
                 1.f, 1.f, flagno, 0.f, 0.f, fcnormal[Cell::NE]);
        packCell(&bufs->sdat[k + 24], x, y + 1, c.heights[Cell::NW] + height_boost, white, 0.f,
                 1.f, flagno, 0.f, 0.f, fcnormal[Cell::NW]);
        packCell(&bufs->sdat[k + 32], x + 0.5f, y + 0.5f,
                 c.heights[Cell::CENTER] + height_boost, white, 0.5f, 0.5f, flagno, 0.f, 0.f,
                 fcnormal[Cell::CENTER]);
        for (uint f = 0; f < 4; f++) {
          for (uint i = 0; i < 3; i++) {
            bufs->sidx[j * 12 + 3 * f + i] = 5 * j + topindices[f][i];
          }
        }
      }

      // We assume that a quad suffices to render each wall
      // (otherwise, in the worst case, we'd need 6 vertices/side!
      int p = j * 8 * 8;
      const Cell& ns = cell(x, y - 1);
      const float nv[2] = {0.f, 0.f};
      int nsover = cmp(c.heights[Cell::SW], ns.heights[Cell::NW]) +
                       cmp(c.heights[Cell::SE], ns.heights[Cell::NE]) >
                   0;
      packCell(&bufs->wdat[p], x, y, c.heights[Cell::SW], Color(c.wallColors[Cell::SW]), 0.5f,
               0.5f, 0, nv[0], nv[1], nsover ? snormal : nnormal);
      packCell(&bufs->wdat[p + 8], x, y, ns.heights[Cell::NW], Color(ns.wallColors[Cell::NW]),
               0.5f, 0.5f, 0, nv[0], nv[1], nsover ? snormal : nnormal);
      packCell(&bufs->wdat[p + 16], x + 1, y, c.heights[Cell::SE],
               Color(c.wallColors[Cell::SE]), 0.5f, 0.5f, 0, nv[0], nv[1],
               nsover ? snormal : nnormal);
      packCell(&bufs->wdat[p + 24], x + 1, y, ns.heights[Cell::NE],
               Color(ns.wallColors[Cell::NE]), 0.5f, 0.5f, 0, nv[0], nv[1],
               nsover ? snormal : nnormal);

      const Cell& ne = cell(x - 1, y);
      int ewover = cmp(c.heights[Cell::SW], ne.heights[Cell::SE]) +
                       cmp(c.heights[Cell::NW], ne.heights[Cell::NE]) >
                   0;
      packCell(&bufs->wdat[p + 32], x, y, c.heights[Cell::SW], Color(c.wallColors[Cell::SW]),
               0.5f, 0.5f, 0, nv[0], nv[1], ewover ? enormal : wnormal);
      packCell(&bufs->wdat[p + 40], x, y, ne.heights[Cell::SE], Color(ne.wallColors[Cell::SE]),
               0.5f, 0.5f, 0, nv[0], nv[1], ewover ? enormal : wnormal);
      packCell(&bufs->wdat[p + 48], x, y + 1, c.heights[Cell::NW],
               Color(c.wallColors[Cell::NW]), 0.5f, 0.5f, 0, nv[0], nv[1],
               ewover ? enormal : wnormal);
      packCell(&bufs->wdat[p + 56], x, y + 1, ne.heights[Cell::NE],
               Color(ne.wallColors[Cell::NE]), 0.5f, 0.5f, 0, nv[0], nv[1],
               ewover ? enormal : wnormal);

      // Render the quads, overlapping triangles if twisted
      bool quad_opaque =
          c.wallColors[Cell::SW].isOpaque() && ns.wallColors[Cell::NW].isOpaque() &&
          c.wallColors[Cell::SE].isOpaque() && ns.wallColors[Cell::NE].isOpaque();
      const ushort quadmap[6] = {0, 1, 2, 1, 3, 2};
      const ushort utriang[6] = {0, 1, 3, 0, 3, 2};
      const ushort dtriang[6] = {0, 1, 2, 1, 2, 3};
      const ushort* sel;
      if (cmp(c.heights[Cell::SW], ns.heights[Cell::NW]) *
              cmp(c.heights[Cell::SE], ns.heights[Cell::NE]) >=
          0) {
        sel = quadmap;
      } else if (c.heights[Cell::SW] < ns.heights[Cell::NW]) {
        sel = utriang;
      } else {
        sel = dtriang;
      }
      if (quad_opaque) {
        for (uint i = 0; i < 6; i++) bufs->woidx[j * 12 + i] = j * 8 + sel[i];
        for (uint i = 0; i < 6; i++) bufs->waidx[j * 12 + i] = 0;
      } else {
        for (uint i = 0; i < 6; i++) bufs->woidx[j * 12 + i] = 0;
        for (uint i = 0; i < 6; i++) bufs->waidx[j * 12 + i] = j * 8 + sel[i];
      }

      bool aquad_opaque =
          c.wallColors[Cell::SW].isOpaque() && ne.wallColors[Cell::NE].isOpaque() &&
          c.wallColors[Cell::NW].isOpaque() && ne.wallColors[Cell::SE].isOpaque();
      const ushort aquadmap[6] = {0, 2, 1, 2, 3, 1};
      const ushort autriang[6] = {0, 2, 1, 2, 3, 1};
      const ushort adtriang[6] = {0, 3, 1, 2, 3, 0};
      if (cmp(c.heights[Cell::SW], ne.heights[Cell::SE]) +
          cmp(c.heights[Cell::NW], ne.heights[Cell::NE])) {
        sel = aquadmap;
      } else if (c.heights[Cell::SW] > ne.heights[Cell::SE]) {
        sel = autriang;
      } else {
        sel = adtriang;
      }
      if (aquad_opaque) {
        for (uint i = 0; i < 6; i++) bufs->woidx[j * 12 + 6 + i] = j * 8 + 4 + sel[i];
        for (uint i = 0; i < 6; i++) bufs->waidx[j * 12 + 6 + i] = 0;
      } else {
        for (uint i = 0; i < 6; i++) bufs->woidx[j * 12 + 6 + i] = 0;
        for (uint i = 0; i < 6; i++) bufs->waidx[j * 12 + 6 + i] = j * 8 + 4 + sel[i];
      }

      // Line data ! (thankfully they're all black)
      int s = j * 12;
      bufs->ldat[s++] = float(x);
      bufs->ldat[s++] = float(y);
      bufs->ldat[s++] = c.heights[Cell::SW];
      bufs->ldat[s++] = float(x + 1);
      bufs->ldat[s++] = float(y);
      bufs->ldat[s++] = c.heights[Cell::SE];
      bufs->ldat[s++] = float(x + 1);
      bufs->ldat[s++] = float(y + 1);
      bufs->ldat[s++] = c.heights[Cell::NE];
      bufs->ldat[s++] = float(x);
      bufs->ldat[s++] = float(y + 1);
      bufs->ldat[s++] = c.heights[Cell::NW];

      int t = j * 8;
      // We disable lines by doubling indices
      bufs->lidx[t++] = 4 * j + 0;
      bufs->lidx[t++] = 4 * j + (c.flags & (CELL_NOLINESOUTH | CELL_NOGRID) ? 0 : 1);
      bufs->lidx[t++] = 4 * j + 1;
      bufs->lidx[t++] = 4 * j + (c.flags & (CELL_NOLINEEAST | CELL_NOGRID) ? 1 : 2);
      bufs->lidx[t++] = 4 * j + 2;
      bufs->lidx[t++] = 4 * j + (c.flags & (CELL_NOLINENORTH | CELL_NOGRID) ? 2 : 3);
      bufs->lidx[t++] = 4 * j + 3;
      bufs->lidx[t++] = 4 * j + (c.flags & (CELL_NOLINEWEST | CELL_NOGRID) ? 3 : 0);

      // Water
      int wvis = c.isWaterVisible();
      if (wvis) {
        Coord3d onormal[5];
        c.getWaterNormals(onormal);
        float fonormal[5][3];
        for (size_t i = 0; i < 15; i++) {
          fonormal[i / 3][i % 3] = (float)onormal[i / 3][i % 3];
        }

        int u = j * 8 * 5;
        packWaterCell(&bufs->fdat[u], x, y, c.waterHeights[Cell::SW], c.velocity, 0.0f, 0.0f,
                      fonormal[Cell::SW]);
        packWaterCell(&bufs->fdat[u + 8], x + 1, y, c.waterHeights[Cell::SE], c.velocity, 1.0f,
                      0.0f, fonormal[Cell::SE]);
        packWaterCell(&bufs->fdat[u + 16], x + 1, y + 1, c.waterHeights[Cell::NE], c.velocity,
                      1.0f, 1.0f, fonormal[Cell::NE]);
        packWaterCell(&bufs->fdat[u + 24], x, y + 1, c.waterHeights[Cell::NW], c.velocity,
                      0.0f, 1.0f, fonormal[Cell::NW]);
        packWaterCell(&bufs->fdat[u + 32], x + 0.5f, y + 0.5f, c.waterHeights[Cell::CENTER],
                      c.velocity, 0.5f, 0.5f, fonormal[Cell::CENTER]);
        for (uint f = 0; f < 4; f++) {
          for (uint i = 0; i < 3; i++) {
            bufs->fidx[j * 12 + 3 * f + i] = 5 * j + topindices[f][i];
          }
        }
      } else {
        // Zero tile (safer than uninitialized)
        memset(&bufs->fdat[j * 5 * 8], 0, 5 * 8 * sizeof(GLfloat));
        for (int i = 0; i < 12; i++) { bufs->fidx[j * 12 + i] = 0; }
      }
    }
    if ((!c.displayListDirty || j == CHUNKSIZE * CHUNKSIZE - 1) && jstart >= 0) {
      int jend = c.displayListDirty ? CHUNKSIZE * CHUNKSIZE : j;
      size_t tile_data_size = 5 * 8 * sizeof(GLfloat);
      size_t tile_index_size = 12 * sizeof(ushort);
      size_t wall_data_size = 8 * 8 * sizeof(GLfloat);
      size_t wall_index_size = 12 * sizeof(ushort);
      size_t line_data_size = 4 * 3 * sizeof(GLfloat);
      size_t line_index_size = 8 * sizeof(ushort);
      size_t flui_data_size = 5 * 8 * sizeof(GLfloat);
      size_t flui_index_size = 12 * sizeof(ushort);

      if (first_time) {
        // Tiles
        glBindVertexArray(chunk->tile_vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->tile_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * tile_data_size, bufs->tdat,
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tile_vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * tile_index_size,
                     bufs->toidx, GL_DYNAMIC_DRAW);
        configureCellAttributes(false);

        glBindVertexArray(chunk->tile_alpha_vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->tile_vbo[0]);
        // already uploaded data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tile_vbo[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * tile_index_size,
                     bufs->taidx, GL_DYNAMIC_DRAW);
        configureCellAttributes(false);

        if (activeView.show_flag_state) {
          glBindVertexArray(chunk->flag_vao);
          glBindBuffer(GL_ARRAY_BUFFER, chunk->flag_vbo[0]);
          glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * tile_data_size, bufs->sdat,
                       GL_DYNAMIC_DRAW);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->flag_vbo[1]);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * tile_index_size,
                       bufs->sidx, GL_DYNAMIC_DRAW);
          configureCellAttributes(false);
        }

        // Walls
        glBindVertexArray(chunk->wall_vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->wall_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * wall_data_size, bufs->wdat,
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->wall_vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * wall_index_size,
                     bufs->woidx, GL_DYNAMIC_DRAW);
        configureCellAttributes(false);

        glBindVertexArray(chunk->wall_alpha_vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->wall_vbo[0]);
        // already uploaded data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->wall_vbo[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * wall_index_size,
                     bufs->waidx, GL_DYNAMIC_DRAW);
        configureCellAttributes(false);

        // Lines
        glBindVertexArray(chunk->line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->line_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * line_data_size, bufs->ldat,
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->line_vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * line_index_size,
                     bufs->lidx, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        // Water
        glBindVertexArray(chunk->flui_vao);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->flui_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * flui_data_size, bufs->fdat,
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->flui_vbo[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * flui_index_size,
                     bufs->fidx, GL_DYNAMIC_DRAW);
        configureCellAttributes(true);
      } else {
        // chunk VAOs will automatically use their previously bound VBOs
        glBindVertexArray(0);

        // Tiles
        glBindBuffer(GL_ARRAY_BUFFER, chunk->tile_vbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, jstart * tile_data_size,
                        (jend - jstart) * tile_data_size,
                        &bufs->tdat[jstart * tile_data_size / sizeof(GLfloat)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tile_vbo[1]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * tile_index_size,
                        (jend - jstart) * tile_index_size,
                        &bufs->toidx[jstart * tile_index_size / sizeof(ushort)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tile_vbo[2]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * tile_index_size,
                        (jend - jstart) * tile_index_size,
                        &bufs->taidx[jstart * tile_index_size / sizeof(ushort)]);

        if (activeView.show_flag_state) {
          glBindBuffer(GL_ARRAY_BUFFER, chunk->flag_vbo[0]);
          glBufferSubData(GL_ARRAY_BUFFER, jstart * tile_data_size,
                          (jend - jstart) * tile_data_size,
                          &bufs->sdat[jstart * tile_data_size / sizeof(GLfloat)]);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->flag_vbo[1]);
          glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * tile_index_size,
                          (jend - jstart) * tile_index_size,
                          &bufs->sidx[jstart * tile_index_size / sizeof(ushort)]);
        }

        // Walls
        glBindBuffer(GL_ARRAY_BUFFER, chunk->wall_vbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, jstart * wall_data_size,
                        (jend - jstart) * wall_data_size,
                        &bufs->wdat[jstart * wall_data_size / sizeof(GLfloat)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->wall_vbo[1]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * wall_index_size,
                        (jend - jstart) * wall_index_size,
                        &bufs->woidx[jstart * wall_index_size / sizeof(ushort)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->wall_vbo[2]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * wall_index_size,
                        (jend - jstart) * wall_index_size,
                        &bufs->waidx[jstart * wall_index_size / sizeof(ushort)]);

        // Lines
        glBindBuffer(GL_ARRAY_BUFFER, chunk->line_vbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, jstart * line_data_size,
                        (jend - jstart) * line_data_size,
                        &bufs->ldat[jstart * line_data_size / sizeof(GLfloat)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->line_vbo[1]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * line_index_size,
                        (jend - jstart) * line_index_size,
                        &bufs->lidx[jstart * line_index_size / sizeof(ushort)]);
        // Water
        glBindBuffer(GL_ARRAY_BUFFER, chunk->flui_vbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, jstart * flui_data_size,
                        (jend - jstart) * flui_data_size,
                        &bufs->fdat[jstart * flui_data_size / sizeof(GLfloat)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->flui_vbo[1]);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, jstart * flui_index_size,
                        (jend - jstart) * flui_index_size,
                        &bufs->fidx[jstart * flui_index_size / sizeof(ushort)]);
      }
      jstart = -1;
    }
  }

  // Wipe dirty list. This _MUST_ be a separate phase
  // since cell(x,y) is not guaranteed to be unique!
  for (int x = chunk->xm; x < chunk->xm + CHUNKSIZE; x++) {
    for (int y = chunk->ym; y < chunk->ym + CHUNKSIZE; y++) {
      cell(x, y).displayListDirty = false;
    }
  }

  // Update view parameters
  chunk->minHeight = minz;
  chunk->maxHeight = maxz;
  chunk->is_updated = false;
}

void Map::markCellsUpdated(int x1, int y1, int x2, int y2, bool changed_walls) {
  /* If the walls of a cell have changed, update its neighbors */
  int dd = changed_walls ? 1 : 0;

  /* Apply the same clamping to bounds as for cell lookups */
  int xmin = std::max(std::min(std::min(x1, x2) - dd, width - 1), 0);
  int xmax = std::max(std::min(std::max(x1, x2) + dd, width - 1), 0);
  int ymin = std::max(std::min(std::min(y1, y2) - dd, height - 1), 0);
  int ymax = std::max(std::min(std::max(y1, y2) + dd, height - 1), 0);

  for (int xp = xmin - xmin % CHUNKSIZE; xp <= xmax - xmax % CHUNKSIZE; xp += CHUNKSIZE) {
    for (int yp = ymin - ymin % CHUNKSIZE; yp <= ymax - ymax % CHUNKSIZE; yp += CHUNKSIZE) {
      Chunk* z = chunk(xp, yp);
      z->is_updated = true;
      for (int x = std::max(xmin, xp); x <= std::min(xmax, xp + CHUNKSIZE - 1); x++) {
        for (int y = std::max(ymin, yp); y <= std::min(ymax, yp + CHUNKSIZE - 1); y++) {
          Cell& c = cells[x + width * y];
          c.displayListDirty = true;
          for (int k = 0; k < 5; k++) {
            z->minHeight = std::min(z->minHeight, std::min(c.heights[k], c.waterHeights[k]));
            z->maxHeight = std::max(z->maxHeight, std::max(c.heights[k], c.waterHeights[k]));
          }
        }
      }
    }
  }
}

void Map::drawFootprint(int x1, int y1, int x2, int y2, int kind) {
  Color color(SRGBColor(kind ? 0.5 : 0.2, 0.2, kind ? 0.2 : 0.5, 1.0));

  int ncells = (std::abs(x1 - x2) + 1) * (std::abs(y1 - y2) + 1);
  if (ncells > 2 * 256 * 256) {
    warning("Footprint requested for too large an area. Drawing nothing.");
    glEnable(GL_DEPTH_TEST);
    return;
  }

  GLfloat* data = new GLfloat[8 * 8 * ncells];
  uint* idxs = new uint[8 * 3 * ncells];

  GLfloat edge = 0.05f;
  const GLfloat flat[3] = {0.f, 0.f, 0.f};

  int j = 0;
  for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
    for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
      Cell& center = cell(x, y);
      packObjectVertex(&data[j * 8 * 8], x + edge, y + edge, center.heights[Cell::SW], 0., 0.,
                       color, flat);
      packObjectVertex(&data[j * 8 * 8 + 8], x + 1 - edge, y + edge, center.heights[Cell::SE],
                       0., 0., color, flat);
      packObjectVertex(&data[j * 8 * 8 + 16], x + 1 - edge, y + 1 - edge,
                       center.heights[Cell::NE], 0., 0., color, flat);
      packObjectVertex(&data[j * 8 * 8 + 24], x + edge, y + 1 - edge, center.heights[Cell::NW],
                       0., 0., color, flat);
      packObjectVertex(&data[j * 8 * 8 + 32], x - edge, y - edge, center.heights[Cell::SW], 0.,
                       0., color, flat);
      packObjectVertex(&data[j * 8 * 8 + 40], x + 1 + edge, y - edge, center.heights[Cell::SE],
                       0., 0., color, flat);
      packObjectVertex(&data[j * 8 * 8 + 48], x + 1 + edge, y + 1 + edge,
                       center.heights[Cell::NE], 0., 0., color, flat);
      packObjectVertex(&data[j * 8 * 8 + 56], x - edge, y + 1 + edge, center.heights[Cell::NW],
                       0., 0., color, flat);

      uint local_triangles[8][3] = {{4, 5, 0}, {0, 5, 1}, {6, 1, 5}, {1, 6, 2},
                                    {7, 2, 6}, {2, 7, 3}, {4, 3, 7}, {3, 4, 0}};
      for (int i = 0; i < 24; i++) {
        idxs[j * 24 + i] = local_triangles[i / 3][i % 3] + 8 * j;
      }
      j++;
    }
  }

  GLuint databuf, idxbuf, vao;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, 8 * 8 * ncells * sizeof(GLfloat), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * 3 * ncells * sizeof(uint), idxs, GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  const UniformLocations* uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, identity4, Color(0., 0., 0., 1.), 0., Lighting_None);

  glBindTexture(GL_TEXTURE_2D, textureBlank);

  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 8 * 3 * ncells, GL_UNSIGNED_INT, (void*)0);

  glBindVertexArray(0);
  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);
  glDeleteVertexArrays(1, &vao);

  glEnable(GL_DEPTH_TEST);
}

void Map::drawLoop(int x1, int y1, int x2, int y2, int kind) {
  Color color(SRGBColor(0.2, kind ? 0.2 : 0.8, 0.2, 1.0));

  int npts = 4 * std::abs(x1 - x2) + 4 * std::abs(y1 - y2) + 8;
  Coord3d* ringPoints = new Coord3d[npts + 2];
  Coord3d* rp = ringPoints;
  int k = 1;
  /* Traverse loop in order. SW is x,y; N is y+1, E is x+1. */
  int xl = std::min(x1, x2), xh = std::max(x1, x2), yl = std::min(y1, y2),
      yh = std::max(y1, y2);
  for (int x = xl; x <= xh; x++) {
    Cell& c = cell(x, yl);
    Coord3d lx((double)x, (double)yl, c.heights[Cell::SW]);
    Coord3d ly((double)x + 1, (double)yl, c.heights[Cell::SE]);
    rp[k] = lx;
    k++;
    rp[k] = ly;
    k++;
  }
  for (int y = yl; y <= yh; y++) {
    Cell& c = cell(xh, y);
    Coord3d lx((double)xh + 1, (double)y, c.heights[Cell::SE]);
    Coord3d ly((double)xh + 1, (double)y + 1, c.heights[Cell::NE]);
    rp[k] = lx;
    k++;
    rp[k] = ly;
    k++;
  }
  for (int x = xh; x >= xl; x--) {
    Cell& c = cell(x, yh);
    Coord3d lx((double)x + 1, (double)yh + 1, c.heights[Cell::NE]);
    Coord3d ly((double)x, (double)yh + 1, c.heights[Cell::NW]);
    rp[k] = lx;
    k++;
    rp[k] = ly;
    k++;
  }
  for (int y = yh; y >= yl; y--) {
    Cell& c = cell(xl, y);
    Coord3d lx((double)xl, (double)y + 1, c.heights[Cell::NW]);
    Coord3d ly((double)xl, (double)y, c.heights[Cell::NE]);
    rp[k] = lx;
    k++;
    rp[k] = ly;
    k++;
  }
  /* Close the loop */
  ringPoints[0] = ringPoints[k - 1];
  ringPoints[k] = ringPoints[1];

  GLfloat* data = new GLfloat[8 * 2 * npts];
  GLfloat width = 0.05f;
  GLfloat flat[3] = {0., 0., 0.};
  char* pos = (char*)data;
  int nontrivial = 0;
  for (int i = 1; i < npts + 1; i++) {
    Coord3d prev = ringPoints[i - 1], cur = ringPoints[i], nxt = ringPoints[i + 1];
    Coord3d dir1 = nxt - cur, dir2 = cur - prev;
    if (length(dir1) < 1e-2 && length(dir1) < 1e-2) { continue; }
    if (length(dir1) < 1e-2) { dir1 = dir2; }
    if (length(dir2) < 1e-2) { dir2 = dir1; }
    dir1 = dir1 / length(dir1);
    dir2 = dir2 / length(dir2);
    Coord3d dir = dir1 + dir2;
    if (length(dir) < 1e-2) { continue; }
    nontrivial++;
    dir = dir / length(dir);
    dir[2] = 0;
    Coord3d up(0., 0., 1.);
    Coord3d res = crossProduct(up, dir);
    res[2] = 0.;
    if (length(res) > 0) res = res / length(res);
    Coord3d anti = crossProduct(dir, res);
    GLfloat off = 1e-2;
    pos += packObjectVertex(pos, cur[0] + width * res[0] + off * anti[0],
                            cur[1] + width * res[1] + off * anti[1], cur[2] + off * anti[2],
                            0., 0., color, flat);
    pos += packObjectVertex(pos, cur[0] - width * res[0] + off * anti[0],
                            cur[1] - width * res[1] + off * anti[1], cur[2] + off * anti[2],
                            0., 0., color, flat);
  }
  ushort* idxs = new ushort[2 * nontrivial * 3];
  for (int i = 0; i < nontrivial; i++) {
    int j = (i + 1) % nontrivial;
    idxs[6 * i + 0] = 2 * j;
    idxs[6 * i + 1] = 2 * i;
    idxs[6 * i + 2] = 2 * j + 1;
    idxs[6 * i + 3] = 2 * i;
    idxs[6 * i + 4] = 2 * i + 1;
    idxs[6 * i + 5] = 2 * j + 1;
  }
  delete[] ringPoints;

  // Transfer data
  GLuint databuf, idxbuf, vao;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, 8 * 2 * nontrivial * sizeof(GLfloat), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * nontrivial * sizeof(ushort), idxs,
               GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  const UniformLocations* uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, identity4, Color(0., 0., 0., 1.), 0., Lighting_None);

  glBindTexture(GL_TEXTURE_2D, textureBlank);

  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 2 * 3 * npts, GL_UNSIGNED_SHORT, (void*)0);

  glBindVertexArray(0);
  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);
  glDeleteVertexArrays(1, &vao);
}

void Map::drawSpotRing(Real x1, Real y1, Real r, int kind) {
  int nfacets = std::max(12, (int)(10. * r));
  GLfloat width = std::min(0.5 * r, 0.05);
  Color color(SRGBColor(kind == 0 ? 1. : 0.5, kind == 1 ? 1. : 0.5, kind == 2 ? 1. : 0.5, 1.));
  GLfloat flat[3] = {0., 0., 0.};
  Real height = Map::getHeight(x1, y1);

  GLfloat* data = new GLfloat[8 * 2 * nfacets];
  ushort* idxs = new ushort[3 * 2 * nfacets];
  char* pos = (char*)data;

  for (int i = 0; i < nfacets; i++) {
    GLfloat angle = 2 * i * M_PI / nfacets;
    pos += packObjectVertex(pos, x1 + (r - width) * std::cos(angle),
                            y1 + (r - width) * std::sin(angle), height + 1e-2, 0., 0., color,
                            flat);
    pos += packObjectVertex(pos, x1 + (r + width) * std::cos(angle),
                            y1 + (r + width) * std::sin(angle), height + 1e-2, 0., 0., color,
                            flat);
  }

  for (int i = 0; i < nfacets; i++) {
    int j = (i + 1) % nfacets;
    idxs[6 * i + 0] = 2 * j;
    idxs[6 * i + 1] = 2 * i;
    idxs[6 * i + 2] = 2 * j + 1;
    idxs[6 * i + 3] = 2 * i;
    idxs[6 * i + 4] = 2 * i + 1;
    idxs[6 * i + 5] = 2 * j + 1;
  }

  // Transfer data
  GLuint databuf, idxbuf, vao;
  glGenBuffers(1, &databuf);
  glGenBuffers(1, &idxbuf);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, databuf);
  glBufferData(GL_ARRAY_BUFFER, 8 * 2 * nfacets * sizeof(GLfloat), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * nfacets * sizeof(ushort), idxs,
               GL_STATIC_DRAW);
  delete[] data;
  delete[] idxs;

  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  const UniformLocations* uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, identity4, Color(0., 0., 0., 1.), 0., Lighting_None);

  glBindTexture(GL_TEXTURE_2D, textureBlank);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 2 * 3 * nfacets, GL_UNSIGNED_SHORT, (void*)0);

  glBindVertexArray(0);
  glDeleteBuffers(1, &databuf);
  glDeleteBuffers(1, &idxbuf);
  glDeleteVertexArrays(1, &vao);
}

Chunk* Map::chunk(int cx, int cy) const {
  if (cx % CHUNKSIZE != 0 || cy % CHUNKSIZE != 0) {
    warning("Bad chunk access %d %d", cx, cy);
    return NULL;
  }
  std::pair<int, int> cpos(cx, cy);
  if (chunks.count(cpos) == 0) {
    // If local region not present, create associated chunk
    // & establish height boundaries
    Chunk c = Chunk();
    c.is_active = false;
    c.is_updated = true;
    c.is_visible = false;
    c.xm = cx;
    c.ym = cy;
    c.minHeight = 1e99;
    c.maxHeight = 1e-99;
    for (int dx = c.xm; dx < c.xm + CHUNKSIZE; dx++) {
      for (int dy = c.xm; dy < c.xm + CHUNKSIZE; dy++) {
        const Cell& w = cell(dx, dy);
        for (int k = 0; k < 5; k++) {
          c.maxHeight = std::max(c.maxHeight, std::max(w.heights[k], w.waterHeights[k]));
          c.minHeight = std::min(c.minHeight, std::min(w.heights[k], w.waterHeights[k]));
        }
      }
    }
    chunks[cpos] = c;
  }
  return &chunks[cpos];
}

/** Saves the map to file in compressed binary or compressed ascii format */
int Map::save(char* pathname, int x, int y) {
  int version = mapFormatVersion;

  if (pathIsLink(pathname)) {
    warning("%s is a link, cannot save map", pathname);
    return 0;
  }

  gzFile gp = gzopen(pathname, "wb9");
  if (!gp) return 0;
  Coord3d tmp = startPosition;
  startPosition = Coord3d(x, y, cell(x, y).heights[Cell::CENTER]);
  int32_t data[6];
  for (int i = 0; i < 3; i++)
    data[i] = saveInt((int32_t)startPosition[i]);  // no decimal part needed
  tmp = startPosition;
  data[3] = saveInt((int32_t)width);
  data[4] = saveInt((int32_t)height);
  data[5] = saveInt((int32_t)version);
  gzwrite(gp, data, sizeof(int32_t) * 6);

  /* new from version 7, save texture names */
  data[0] = saveInt(numTextures);
  gzwrite(gp, data, sizeof(int32_t) * 1);
  for (int i = 0; i < numTextures; i++) {
    char textureName[64];
    memset(textureName, 0, sizeof(textureName));
    strncpy(textureName, textureNames[i], 63);
    gzwrite(gp, textureName, 64);
  }

  for (int i = 0; i < width * height; i++) cells[i].dump(gp);
  gzclose(gp);
  return 1;
}

void Cell::dump(gzFile gp) const {
  int32_t data[8];
  for (int i = 0; i < 5; i++) data[i] = saveInt((int32_t)std::round(heights[i] / 0.0025));
  gzwrite(gp, data, sizeof(int32_t) * 5);
  for (int i = 0; i < 5; i++) {
    data[0] = saveInt((int32_t)std::round(colors[i].f0() / 0.01));
    data[1] = saveInt((int32_t)std::round(colors[i].f1() / 0.01));
    data[2] = saveInt((int32_t)std::round(colors[i].f2() / 0.01));
    data[3] = saveInt((int32_t)std::round(colors[i].f3() / 0.01));
    gzwrite(gp, data, sizeof(int32_t) * 4);
  }
  data[0] = saveInt((int32_t)flags);
  data[1] = saveInt((int32_t)texture);
  gzwrite(gp, data, sizeof(int32_t) * 2);

  for (int i = 0; i < 4; i++) {
    data[0] = saveInt((int32_t)std::round(wallColors[i].f0() / 0.01));
    data[1] = saveInt((int32_t)std::round(wallColors[i].f1() / 0.01));
    data[2] = saveInt((int32_t)std::round(wallColors[i].f2() / 0.01));
    data[3] = saveInt((int32_t)std::round(wallColors[i].f3() / 0.01));
    gzwrite(gp, data, sizeof(int32_t) * 4);
  }

  data[0] = saveInt((int32_t)std::round(velocity[0] / 0.01));
  data[1] = saveInt((int32_t)std::round(velocity[1] / 0.01));
  gzwrite(gp, data, sizeof(int32_t) * 2);

  for (int i = 0; i < 5; i++) data[i] = saveInt((int32_t)std::round(waterHeights[i] / 0.0025));
  gzwrite(gp, data, sizeof(int32_t) * 5);

  for (int i = 0; i < 4; i++) {
    /* used to refer to texture coordinates */
    data[i * 2] = 0;
    data[i * 2 + 1] = 0;
  }
  gzwrite(gp, data, sizeof(int32_t) * 8);
}

void Cell::load(Map* map, gzFile gp, int version) {
  int32_t data[8];

  gzread(gp, data, sizeof(int32_t) * 5);
  for (int i = 0; i < 5; i++) heights[i] = 0.0025 * loadInt(data[i]);
  for (int i = 0; i < 5; i++) {
    if (version < 4) {
      // old maps do not have an alpha channel defined
      gzread(gp, data, sizeof(int32_t) * 3);
      for (int j = 0; j < 3; j++) colors[i].w[j] = 65535 * 0.01 * loadInt(data[j]);
      colors[i].w[3] = 65535;
    } else {
      gzread(gp, data, sizeof(int32_t) * 4);
      for (int j = 0; j < 4; j++) colors[i].w[j] = 65535 * 0.01 * loadInt(data[j]);
    }
  }

  gzread(gp, data, sizeof(int32_t) * 2);
  flags = loadInt(data[0]);
  if (version <= 1) flags = flags & (CELL_ICE | CELL_ACID);
  int k = loadInt(data[1]);
  texture = (k >= 0 && k < numTextures ? map->indexTranslation[k] : -1);
  // in older maps, this field was not initialized
  if (version < 5) texture = -1;
  if (version < 3) { /* Old maps do not have wallColors defined */
    for (int i = 0; i < 4; i++) { wallColors[i] = SRGBColor(0.7, 0.2, 0.2, 1.0); }
  } else {
    for (int i = 0; i < 4; i++) {
      if (version < 4) {
        // old maps do not have an alpha channel defined
        gzread(gp, data, sizeof(int32_t) * 3);
        for (int j = 0; j < 3; j++) wallColors[i].w[j] = 65535 * 0.01 * loadInt(data[j]);
        wallColors[i].w[3] = 65535;
      } else {
        gzread(gp, data, sizeof(int32_t) * 4);
        for (int j = 0; j < 4; j++) wallColors[i].w[j] = 65535 * 0.01 * loadInt(data[j]);
      }
    }
  }

  if (version >= 5) {
    gzread(gp, data, sizeof(int32_t) * 2);
    velocity[0] = 0.01 * loadInt(data[0]);
    velocity[1] = 0.01 * loadInt(data[1]);
  } else {
    velocity[0] = 0.0;
    velocity[1] = 0.0;
  }

  // currently we just reset the ground water
  if (version >= 6) {
    gzread(gp, data, sizeof(int32_t) * 5);
    for (int i = 0; i < 5; i++) waterHeights[i] = 0.0025 * loadInt(data[i]);
  } else {
    for (int i = 0; i < 5; i++) waterHeights[i] = heights[i] - 0.5;
  }
  sunken = 0.0;

  if (version >= 7) {
    gzread(gp, data, sizeof(int32_t) * 8);
    /* used to be texture coordinates; now is free space */
  }
}
