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

#include "general.h"
#include "map.h"
#include "SDL2/SDL_endian.h"
#include "SDL2/SDL_image.h"

#include "settings.h"
#include "game.h"
#include "gameMode.h"
#include "editMode.h"

using namespace std;

/* VISRADIUS is half-width of square of drawable cells
   MARGIN    is px zone on edge of screen where cells can be skipped. */
#define VISRADIUS 50
#define CACHE_SIZE (VISRADIUS * 2 + 1)
#define MARGIN 10
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
  for (int i = 0; i < 20; i++) colors[i / 4][i % 4] = 1.;
  for (int i = 0; i < 16; i++) wallColors[i / 4][i % 4] = 1.;
  for (int i = 0; i < 5; i++) heights[i] = -8.0;
  for (int i = 0; i < 5; i++) waterHeights[i] = -20.0;
}

/* Returns the normal for a point on the edge of the cell */
void Cell::getNormal(Coord3d normal, int vertex) const {
  Coord3d v1;
  Coord3d v2;

  switch (vertex) {
  case SOUTH + WEST:
    v1[0] = 1.0;
    v1[1] = 0.0;
    v1[2] = heights[SOUTH + EAST] - heights[SOUTH + WEST];
    v2[0] = 0.5;
    v2[1] = 0.5;
    v2[2] = heights[CENTER] - heights[SOUTH + WEST];
    break;
  case SOUTH + EAST:
    v1[0] = 0.0;
    v1[1] = 1.0;
    v1[2] = heights[NORTH + EAST] - heights[SOUTH + EAST];
    v2[0] = -.5;
    v2[1] = 0.5;
    v2[2] = heights[CENTER] - heights[SOUTH + EAST];
    break;
  case NORTH + EAST:
    v1[0] = -1.0;
    v1[1] = 0.0;
    v1[2] = heights[NORTH + WEST] - heights[NORTH + EAST];
    v2[0] = -0.5;
    v2[1] = -0.5;
    v2[2] = heights[CENTER] - heights[NORTH + EAST];
    break;
  case NORTH + WEST:
    v1[0] = 0.0;
    v1[1] = -1.0;
    v1[2] = heights[SOUTH + WEST] - heights[NORTH + WEST];
    v2[0] = 0.5;
    v2[1] = -0.5;
    v2[2] = heights[CENTER] - heights[NORTH + WEST];
    break;
  case CENTER:
    getNormal(normal, SOUTH + WEST);
    getNormal(v1, SOUTH + EAST);
    add(v1, normal, normal);
    getNormal(v1, NORTH + WEST);
    add(v1, normal, normal);
    getNormal(v1, NORTH + EAST);
    add(v1, normal, normal);
    normalize(normal);
    return;
  }
  crossProduct(v1, v2, normal);
  normalize(normal);
}
/* Works on water heights */
void Cell::getWaterNormal(Coord3d normal, int vertex) const {
  Coord3d v1;
  Coord3d v2;

  switch (vertex) {
  case SOUTH + WEST:
    v1[0] = 1.0;
    v1[1] = 0.0;
    v1[2] = waterHeights[SOUTH + EAST] - waterHeights[SOUTH + WEST];
    v2[0] = 0.5;
    v2[1] = 0.5;
    v2[2] = waterHeights[CENTER] - waterHeights[SOUTH + WEST];
    break;
  case SOUTH + EAST:
    v1[0] = 0.0;
    v1[1] = 1.0;
    v1[2] = waterHeights[NORTH + EAST] - waterHeights[SOUTH + EAST];
    v2[0] = -.5;
    v2[1] = 0.5;
    v2[2] = waterHeights[CENTER] - waterHeights[SOUTH + EAST];
    break;
  case NORTH + EAST:
    v1[0] = -1.0;
    v1[1] = 0.0;
    v1[2] = waterHeights[NORTH + WEST] - waterHeights[NORTH + EAST];
    v2[0] = -0.5;
    v2[1] = -0.5;
    v2[2] = waterHeights[CENTER] - waterHeights[NORTH + EAST];
    break;
  case NORTH + WEST:
    v1[0] = 0.0;
    v1[1] = -1.0;
    v1[2] = waterHeights[SOUTH + WEST] - waterHeights[NORTH + WEST];
    v2[0] = 0.5;
    v2[1] = -0.5;
    v2[2] = waterHeights[CENTER] - waterHeights[NORTH + WEST];
    break;
  case CENTER:
    getNormal(normal, SOUTH + WEST);
    getNormal(v1, SOUTH + EAST);
    add(v1, normal, normal);
    getNormal(v1, NORTH + WEST);
    add(v1, normal, normal);
    getNormal(v1, NORTH + EAST);
    add(v1, normal, normal);
    normalize(normal);
    return;
  }
  crossProduct(v1, v2, normal);
  normalize(normal);
}

/* Gives the height of the cell in a specified (floatingpoint) position */
Real Cell::getHeight(Real x, Real y) const {
  Real h1, h2, h3, c;

  c = heights[CENTER];
  if (y <= x)
    if (y <= 1 - x) { /* SOUTH */
      h1 = heights[SOUTH + WEST];
      h2 = heights[SOUTH + EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2 - h1) * x + (c - h3) * 2 * y;
    } else { /* EAST */
      h1 = heights[SOUTH + EAST];
      h2 = heights[NORTH + EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2 - h1) * y + (c - h3) * 2 * (1 - x);
    }
  else if (y <= 1 - x) { /* WEST */
    h1 = heights[NORTH + WEST];
    h2 = heights[SOUTH + WEST];
    h3 = (h1 + h2) / 2;
    return h1 + (h2 - h1) * (1 - y) + (c - h3) * 2 * x;
  } else { /* NORTH */
    h1 = heights[NORTH + EAST];
    h2 = heights[NORTH + WEST];
    h3 = (h1 + h2) / 2;
    return h1 + (h2 - h1) * (1 - x) + (c - h3) * 2 * (1 - y);
  }
}

/* Works for water */
Real Cell::getWaterHeight(Real x, Real y) const {
  Real h1, h2, h3, c;

  c = waterHeights[CENTER];
  if (y <= x)
    if (y <= 1 - x) { /* SOUTH */
      h1 = waterHeights[SOUTH + WEST];
      h2 = waterHeights[SOUTH + EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2 - h1) * x + (c - h3) * 2 * y;
    } else { /* EAST */
      h1 = waterHeights[SOUTH + EAST];
      h2 = waterHeights[NORTH + EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2 - h1) * y + (c - h3) * 2 * (1 - x);
    }
  else if (y <= 1 - x) { /* WEST */
    h1 = waterHeights[NORTH + WEST];
    h2 = waterHeights[SOUTH + WEST];
    h3 = (h1 + h2) / 2;
    return h1 + (h2 - h1) * (1 - y) + (c - h3) * 2 * x;
  } else { /* NORTH */
    h1 = waterHeights[NORTH + EAST];
    h2 = waterHeights[NORTH + WEST];
    h3 = (h1 + h2) / 2;
    return h1 + (h2 - h1) * (1 - x) + (c - h3) * 2 * (1 - y);
  }
}

Chunk::Chunk() {
  is_active = false;
  is_visible = false;
  is_updated = true;
  last_shown = -1;
  // Init with extreme range to ensure visibility after which
  // exact range would be calculated
  maxHeight = 1e3;
  minHeight = 1e-3;
}

Chunk::~Chunk() {
  if (is_active) {
    glDeleteBuffers(2, &tile_vbo[0]);
    glDeleteBuffers(2, &flui_vbo[0]);
    glDeleteBuffers(2, &wall_vbo[0]);
    glDeleteBuffers(2, &line_vbo[0]);
  }
}

Map::Map(char* filename) {
  gzFile gp;
  int x, y, i, north, east;

  isBonus = 0;
  isTransparent = 0;

  cachedCX = cachedCY = -1, cacheCount = 0;

  /* Data structures for maintaining displaylists.
         We allocate rad^2 * 2 display lists for rad^2 x/y coordinates and 2 drawing stages
  */
  nListsWide = CACHE_SIZE;
  nLists = nListsWide * nListsWide * 2;
  displayLists = (int)glGenLists(nLists);

  snprintf(mapname, sizeof(mapname), _("Unknown track"));
  snprintf(author, sizeof(author), _("Unknown author"));
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
      printf("Warning. Map %s is of an old format (v%d, latest is v%d)\n", filename, version,
             mapFormatVersion);
    else if (version > mapFormatVersion) {
      fprintf(stderr, "Error. Map %s is from the future (v%d, I know only format v%d)",
              filename, version, mapFormatVersion);
      fprintf(stderr,
              "This error usually occurs because or broken maps or big/small endian issues\n");
      exit(0);
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
        for (north = 0; north < 2; north++)
          for (east = 0; east < 2; east++) {
            c.textureCoords[north * Cell::NORTH + east * Cell::EAST][0] = (x + east) / 4.0;
            c.textureCoords[north * Cell::NORTH + east * Cell::EAST][1] = (y + north) / 4.0;
          }

        c.load(this, gp, version);
      }
    gzclose(gp);
  } else {
    printf("Warning: could not open %s\n", filename);
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
          c.colors[i][0] = c.colors[i][1] = c.colors[i][2] = 0.9;
          c.colors[i][3] = 1.0;
        }
        for (i = 0; i < 4; i++) {
          c.wallColors[i][0] = 0.7;
          c.wallColors[i][1] = 0.2;
          c.wallColors[i][2] = 0.2;
          c.wallColors[i][3] = 1.0;
        }
        c.velocity[0] = 0.0;
        c.velocity[1] = 0.0;
        for (north = 0; north < 2; north++)
          for (east = 0; east < 2; east++) {
            c.textureCoords[north * Cell::NORTH + east * Cell::EAST][0] = (x + east) / 4.0;
            c.textureCoords[north * Cell::NORTH + east * Cell::EAST][1] = (y + north) / 4.0;
          }
      }
  }

  /* Fix display lists used by each cell */
  for (y = 0; y < height; y++)
    for (x = 0; x < width; x++) {
      Cell& c = cells[x + y * width];
      c.displayList = displayLists + ((x % nListsWide) + (y % nListsWide) * nListsWide) * 2;
      c.displayListLastCompiled = -1;
      c.displayListDirty = 1;
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
  tx_Map = loadTexture("maptex.png");

  cacheVisible = new char[(2 * VISRADIUS + 1) * (2 * VISRADIUS + 1)];
  chunks.clear();
}
Map::~Map() {
  //  glDeleteLists(cache[0].lists[0],(width/4)*(height/4));
  glDeleteLists(displayLists, nLists);
  delete[] cells;
  delete[] cacheVisible;
  chunks.clear();
}

/* Draws the map on the screen from current viewpoint */
void Map::draw(int birdsEye, int stage, int cx, int cy) {
  if (1) {
    if (stage == 0) {
      glDisable(GL_BLEND);
    } else {
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0.01);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    drawMapVBO(birdsEye, cx, cy, stage);
    return;
  }
  int x, y, ix, iy;

  if (stage > 0 && !isTransparent) return;
  static int frameCount = 0;
  frameCount++;

  glPushAttrib(GL_ENABLE_BIT);

  GLfloat specular[4] = {0.0, 0.0, 0.0, 0.0};
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 0.0);
  glDisable(GL_TEXTURE_2D);

  if (stage > 0) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else
    glDisable(GL_BLEND);

  GLint viewport[4];
  GLdouble model_matrix[16], proj_matrix[16];

  double t0 = getSystemTime();
  if (cx != cachedCX || cy != cachedCY || cacheCount > 10) {
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
    glGetDoublev(GL_PROJECTION_MATRIX, proj_matrix);

    Coord3d worldCoord, screenCoord;

    int ix, iy;
    int visibleCnt = 0;

    for (ix = 0; ix < 2 * VISRADIUS + 1; ix++)
      for (iy = 0; iy < 2 * VISRADIUS + 1; iy++) {
        x = cx + ix - VISRADIUS;
        y = cy + iy - VISRADIUS;
        cacheVisible[ix * CACHE_SIZE + iy] = 0;
        worldCoord[0] = x + .5;
        worldCoord[1] = y + .5;
        worldCoord[2] = cell(x, y).heights[Cell::CENTER];
        gluProject(worldCoord[0], worldCoord[1], worldCoord[2], model_matrix, proj_matrix,
                   viewport, &screenCoord[0], &screenCoord[1], &screenCoord[2]);
        if (screenCoord[0] >= -MARGIN && screenCoord[0] <= screenWidth + MARGIN &&
            screenCoord[1] >= -MARGIN && screenCoord[1] <= screenHeight + MARGIN) {
          cacheVisible[ix * CACHE_SIZE + iy] = 1;
          visibleCnt++;
          continue;
        }
        for (int north = 0; north < 2; north++)
          for (int east = 0; east < 2; east++) {
            worldCoord[0] = x + 1.0 * east;
            worldCoord[1] = y + 1.0 * north;
            worldCoord[2] = cell(x, y).heights[north * Cell::NORTH + east * Cell::EAST];
            gluProject(worldCoord[0], worldCoord[1], worldCoord[2], model_matrix, proj_matrix,
                       viewport, &screenCoord[0], &screenCoord[1], &screenCoord[2]);
            if (screenCoord[0] >= -MARGIN && screenCoord[0] <= screenWidth + MARGIN &&
                screenCoord[1] >= -MARGIN && screenCoord[1] <= screenHeight + MARGIN) {
              cacheVisible[ix * CACHE_SIZE + iy] = 1;
              visibleCnt++;
              goto cont;  // The almighty goto statment, don't do this at home kids!
            }
          }
      cont:
        x = x;
      }
    // OPT. use only the +1,+1 versions if birdsEye == true
    for (ix = 0; ix < 2 * VISRADIUS + 1; ix++)
      for (iy = 0; iy < 2 * VISRADIUS + 1; iy++)
        cacheVisible[ix * CACHE_SIZE + iy] |=
            (iy > 0 ? cacheVisible[ix * CACHE_SIZE + iy - 1] : 0) |
            (iy < 2 * VISRADIUS ? cacheVisible[ix * CACHE_SIZE + iy + 1] : 0) |
            (ix > 0 ? cacheVisible[(ix - 1) * CACHE_SIZE + iy] : 0) |
            (ix < 2 * VISRADIUS ? cacheVisible[(ix + 1) * CACHE_SIZE + iy] : 0);
    cachedCX = cx;
    cachedCY = cy;
    cacheCount = 0;

    printf("Time for cache check: %3.3fms\n", 1000.0 * (getSystemTime() - t0));
    printf("%d cells visible\n", visibleCnt);
  } else
    cacheCount++;

  /* If we are doing stage 0 then recompile all display lists (both stage 0 and stage 1)
         that are either too old or have been marked as dirty */
  int redrawCnt = 0;
  if (stage == 0)
    for (ix = 0; ix < 2 * VISRADIUS + 1; ix++)
      for (iy = 0; iy < 2 * VISRADIUS + 1; iy++)
        if (cacheVisible[ix * CACHE_SIZE + iy]) {
          x = cx + ix - VISRADIUS;
          y = cy + iy - VISRADIUS;
          if (x >= 0 && x < width && y >= 0 && y < height) {
            Cell& c = cell(x, y);
            if (c.displayListLastCompiled + 10 < frameCount || c.displayListDirty ||
                c.velocity[0] != 0.0 || c.velocity[1] != 0.0) {
              redrawCnt++;
              c.displayListLastCompiled = frameCount;
              c.displayListDirty = 0;
              int thisStage = 0;
              glNewList(c.displayList + thisStage, GL_COMPILE);
              drawCell(birdsEye, thisStage, x, y);
              glEndList();
              if (isTransparent) {
                thisStage = 1;
                glNewList(c.displayList + thisStage, GL_COMPILE);
                drawCell(birdsEye, thisStage, x, y);
                glEndList();
              }
            }
          }
        }
  // printf("%d cells redrawn\n",redrawCnt);

  double t1 = getSystemTime();

  /* Call all the display lists to draw the actual ground */
  for (ix = 0; ix < 2 * VISRADIUS + 1; ix++)
    for (iy = 0; iy < 2 * VISRADIUS + 1; iy++)
      if (cacheVisible[ix * CACHE_SIZE + iy]) {
        x = cx + ix - VISRADIUS;
        y = cy + iy - VISRADIUS;
        if (x >= 0 && x < width && y >= 0 && y < height) {
          Cell& c = cell(x, y);
          glCallList(c.displayList + stage);
        }
      }
  glPopMatrix();

  glPopAttrib();

  //  printf("Time for total draw (stage %d): %03.3fms (%3.3fms). Redraw %d\n", stage,
  //         1000.0 * (getSystemTime() - t1), 1e3 * (t1 - t0), redrawCnt);
}
char* filetobuf(const char* filename) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) { printf("E: %x\n", err); }
  // Copypasted
  FILE* fptr;
  long length;
  char* buf;

  fptr = fopen(filename, "rb"); /* Open file for reading */
  if (!fptr)                    /* Return NULL on failure */
    return NULL;
  fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
  length = ftell(fptr);     /* Find out how many bytes into the file we are */
  buf = (char*)malloc(
      length +
      1); /* Allocate a buffer for the entire length of the file and a null terminator */
  fseek(fptr, 0, SEEK_SET);    /* Go back to the beginning of the file */
  fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
  fclose(fptr);                /* Close the file */
  buf[length] = 0;             /* Null terminator */

  return buf; /* Return the buffer */
}

void indicateErrors(const char* key) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) { printf("%s: %x\n", key, err); }
}

GLuint loadProgramFromFiles(const char* vertname, const char* fragname) {
  /* Read our shaders into the appropriate buffers */
  char path[256];
  snprintf(path, 256, "%s/shaders/%s", effectiveShareDir, vertname);
  GLchar* vertexsource = filetobuf(path);
  if (vertexsource == NULL) {
    printf("NULL V %s\n", path);
    return -1;
  }
  snprintf(path, 256, "%s/shaders/%s", effectiveShareDir, fragname);
  GLchar* fragmentsource = filetobuf(path);
  if (vertexsource == NULL) {
    printf("NULL F %s\n", path);
    return -1;
  }
  GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
  int maxLength;
  glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
  glCompileShader(vertexshader);
  int IsCompiled_VS, IsCompiled_FS, IsLinked;
  glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
  if (IsCompiled_VS == 0) {
    glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);
    char* vertexInfoLog = (char*)malloc(maxLength);
    glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);
    fprintf(stderr, "%s VILOG |%s|\n", vertname, vertexInfoLog);
    free(vertexInfoLog);
    return -1;
  }
  GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
  glCompileShader(fragmentshader);
  glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
  if (IsCompiled_FS == 0) {
    glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);
    char* fragmentInfoLog = (char*)malloc(maxLength);
    glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);
    free(fragmentInfoLog);
    fprintf(stderr, "%s FILOG |%s|\n", fragname, fragmentInfoLog);
    return -1;
  }
  GLuint shaderprogram = glCreateProgram();
  glAttachShader(shaderprogram, vertexshader);
  glAttachShader(shaderprogram, fragmentshader);
  glBindAttribLocation(shaderprogram, 0, "in_Position");
  glBindAttribLocation(shaderprogram, 1, "in_Color");
  glBindAttribLocation(shaderprogram, 2, "in_Texcoord");
  glBindAttribLocation(shaderprogram, 3, "in_Velocity");
  glLinkProgram(shaderprogram);
  glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int*)&IsLinked);
  if (IsLinked == 0) {
    glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);
    char* shaderProgramInfoLog = (char*)malloc(maxLength);
    glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);
    fprintf(stderr, "%s %s SILOG |%s|\n", vertname, fragname, shaderProgramInfoLog);
    free(shaderProgramInfoLog);
    return -1;
  }
  return shaderprogram;
}

inline ushort clampUShort(GLfloat val, ushort low, ushort high) {
  if (val < low) return low;
  if (val > high) return high;
  return (ushort)val;
}

inline short clampSShort(GLfloat val, short low, short high) {
  if (val <= low) return low;
  if (val >= high) return high;
  return (short)val;
}

inline void packPair(GLfloat ztoa, GLfloat ztob, uint32_t* out) {
  ushort av = clampUShort(65535.f * ztoa, 0, -1);
  ushort bv = clampUShort(65535.f * ztob, 0, -1);
  *out = (bv * (1 << 16) + av);
}
inline void packSPair(GLfloat ztoa, GLfloat ztob, uint32_t* out) {
  short av = clampSShort(32677.f * ztoa, -32677, 32677);
  short bv = clampSShort(32677.f * ztob, -32677, 32677);
  *out = (bv * (1 << 16) + av);
}

inline void packCell(GLfloat* out, GLfloat px, GLfloat py, GLfloat pz, const GLfloat* colors,
                     GLfloat tx, GLfloat ty, const float vel[2]) {
  out[0] = px;
  out[1] = py;
  out[2] = pz;
  packPair(colors[0], colors[1], (uint32_t*)&out[3]);
  packPair(colors[2], colors[3], (uint32_t*)&out[4]);
  packPair(tx, ty, (uint32_t*)&out[5]);
  packSPair(0.25 * vel[0], 0.25 * vel[1], (uint32_t*)&out[6]);
  out[7] = 0.;
}
inline void packWaterCell(GLfloat* out, GLfloat px, GLfloat py, GLfloat pz, const float vel[2],
                          GLfloat tx, GLfloat ty) {
  out[0] = px;
  out[1] = py;
  out[2] = pz;
  ((uint32_t*)out)[3] = -1;
  ((uint32_t*)out)[4] = -1;
  packPair(tx, ty, (uint32_t*)&out[5]);
  packSPair(0.25 * vel[0], 0.25 * vel[1], (uint32_t*)&out[6]);
  out[7] = 0.;
}

inline double smoothSemiRand(int x, int y, double scale) {
  double dt =
      (Game::current ? Game::current->gameTime : ((EditMode*)GameMode::current)->time) * scale;
  int t = (int)dt;
  double frac = dt - t;
  return semiRand(x, y, t) * (1. - frac) + semiRand(x, y, t + 1) * frac;
}

void Map::fillChunkVBO(Chunk* chunk) {
  GLfloat* tdat = new GLfloat[CHUNKSIZE * CHUNKSIZE * 5 * 8];
  ushort* tidx = new ushort[CHUNKSIZE * CHUNKSIZE * 12];
  GLfloat* wdat = new GLfloat[CHUNKSIZE * CHUNKSIZE * 8 * 8];
  ushort* widx = new ushort[CHUNKSIZE * CHUNKSIZE * 12];
  GLfloat* ldat = new GLfloat[CHUNKSIZE * CHUNKSIZE * 4 * 3];
  ushort* lidx = new ushort[CHUNKSIZE * CHUNKSIZE * 8];
  GLfloat* fdat = new GLfloat[CHUNKSIZE * CHUNKSIZE * 8 * 5];
  ushort* fidx = new ushort[CHUNKSIZE * CHUNKSIZE * 12];

  // Update exact chunk bounds as well
  GLfloat minz = 1e99, maxz = -1e99;

  if (chunk->is_active) {
    // Stopgap: wipe/reload style.
    glDeleteBuffers(2, &chunk->tile_vbo[0]);
    glDeleteBuffers(2, &chunk->flui_vbo[0]);
    glDeleteBuffers(2, &chunk->wall_vbo[0]);
    glDeleteBuffers(2, &chunk->line_vbo[0]);
    chunk->is_active = 0;
  }
  glGenBuffers(2, &chunk->tile_vbo[0]);
  glGenBuffers(2, &chunk->wall_vbo[0]);
  glGenBuffers(2, &chunk->line_vbo[0]);
  glGenBuffers(2, &chunk->flui_vbo[0]);
  chunk->is_active = 1;

  // Further speed boosts will come from finer-grained updates
  // (ex. finite cell height shifting)
  for (int x = chunk->xm; x < chunk->xm + CHUNKSIZE; x++) {
    for (int y = chunk->ym; y < chunk->ym + CHUNKSIZE; y++) {
      // Could speed entry up a lot with a geometry shader
      // to eliminate redundant coordinates...
      Cell& c = cell(x, y);
      c.displayListDirty = 0;

      // Update height table for future actions
      for (int k = 0; k < 5; k++) {
        minz = min(minz, c.heights[k]);
        minz = min(minz, c.waterHeights[k]);
        maxz = max(maxz, c.heights[k]);
        maxz = max(maxz, c.waterHeights[k]);
      }

      int j = (x - chunk->xm) + (y - chunk->ym) * CHUNKSIZE;
      // TODO: determine if texture arrays are available!
      // (then we just pass short/short/short in
      float txo = 0.5f, tyo = 0.5f;
      int typed = c.flags & (CELL_ICE | CELL_ACID | CELL_TRACK | CELL_SAND);
      if (typed || c.texture >= 0) {
        if ((c.flags & CELL_ICE) || (!typed && c.texture == tx_Ice)) {
          txo = 0.25f;
          tyo = 0.0f;
        } else if ((c.flags & CELL_SAND) || (!typed && c.texture == tx_Sand)) {
          txo = 0.25f;
          tyo = 0.75f;
        } else if ((c.flags & CELL_TRACK) || (!typed && c.texture == tx_Track)) {
          txo = 0.25f;
          tyo = 0.5f;
        } else if ((c.flags & CELL_ACID) || (!typed && c.texture == tx_Acid)) {
          txo = 0.0f;
          tyo = 0.0f;
        } else if (c.texture == tx_1) {
          txo = 0.0f;
          tyo = 0.25f;
        } else if (c.texture == tx_2) {
          txo = 0.25f;
          tyo = 0.25f;
        } else if (c.texture == tx_3) {
          txo = 0.0f;
          tyo = 0.75f;
        } else if (c.texture == tx_4) {
          txo = 0.0f;
          tyo = 0.5f;
        }
        // TODO: link given map to texture map. Best would be 1d array constructed
        // from up/downscaled given textures
      } else {
        // Nothing much...
      }

      int k = j * 5 * 8;
      packCell(&tdat[k], x, y, c.heights[Cell::SOUTH + Cell::WEST],
               &c.colors[Cell::SOUTH + Cell::WEST][0], txo, tyo, c.velocity);
      packCell(&tdat[k + 8], x + 1, y, c.heights[Cell::SOUTH + Cell::EAST],
               &c.colors[Cell::SOUTH + Cell::EAST][0], txo + 0.125f, tyo, c.velocity);
      packCell(&tdat[k + 16], x + 1, y + 1, c.heights[Cell::NORTH + Cell::EAST],
               &c.colors[Cell::NORTH + Cell::EAST][0], txo + 0.125f, tyo + 0.125f, c.velocity);
      packCell(&tdat[k + 24], x, y + 1, c.heights[Cell::NORTH + Cell::WEST],
               &c.colors[Cell::NORTH + Cell::WEST][0], txo, tyo + 0.125f, c.velocity);
      packCell(&tdat[k + 32], 0.5f * float(2 * x + 1), 0.5f * float(2 * y + 1),
               c.heights[Cell::CENTER], &c.colors[Cell::CENTER][0], txo + 0.0625f,
               tyo + 0.0625f, c.velocity);

      int m = j * 12;
      ushort b = (ushort)5 * j;
      tidx[m++] = b + 0;
      tidx[m++] = b + 1;
      tidx[m++] = b + 4;

      // render both sides just in case
      tidx[m++] = b + 1;
      tidx[m++] = b + 2;
      tidx[m++] = b + 4;

      tidx[m++] = b + 2;
      tidx[m++] = b + 3;
      tidx[m++] = b + 4;

      tidx[m++] = b + 3;
      tidx[m++] = b + 0;
      tidx[m++] = b + 4;

      // We assume that a quad suffices to render each wall
      // (otherwise, in the worst case, we'd need 6 vertices/side!
      int p = j * 8 * 8;
      const Cell& ns = cell(x, y - 1);
      const float nv[2] = {0.f, 0.f};
      packCell(&wdat[p], x, y, c.heights[Cell::SOUTH + Cell::WEST],
               c.wallColors[Cell::SOUTH + Cell::WEST], 0.5f, 0.5f, nv);
      packCell(&wdat[p + 8], x, y, ns.heights[Cell::NORTH + Cell::WEST],
               ns.wallColors[Cell::NORTH + Cell::WEST], 0.5f, 0.5f, nv);
      packCell(&wdat[p + 16], x + 1, y, c.heights[Cell::SOUTH + Cell::EAST],
               c.wallColors[Cell::SOUTH + Cell::EAST], 0.5f, 0.5f, nv);
      packCell(&wdat[p + 24], x + 1, y, ns.heights[Cell::NORTH + Cell::EAST],
               ns.wallColors[Cell::NORTH + Cell::EAST], 0.5f, 0.5f, nv);

      const Cell& ne = cell(x - 1, y);
      packCell(&wdat[p + 32], x, y, c.heights[Cell::WEST + Cell::SOUTH],
               c.wallColors[Cell::WEST + Cell::SOUTH], 0.5f, 0.5f, nv);
      packCell(&wdat[p + 40], x, y, ne.heights[Cell::EAST + Cell::SOUTH],
               ne.wallColors[Cell::EAST + Cell::SOUTH], 0.5f, 0.5f, nv);
      packCell(&wdat[p + 48], x, y + 1, c.heights[Cell::WEST + Cell::NORTH],
               c.wallColors[Cell::WEST + Cell::NORTH], 0.5f, 0.5f, nv);
      packCell(&wdat[p + 56], x, y + 1, ne.heights[Cell::EAST + Cell::NORTH],
               ne.wallColors[Cell::EAST + Cell::NORTH], 0.5f, 0.5f, nv);

      // Render the quad. The heights autocorrect orientations
      int q = j * 12;
      widx[q++] = j * 8 + 0;
      widx[q++] = j * 8 + 1;
      widx[q++] = j * 8 + 2;
      widx[q++] = j * 8 + 1;
      widx[q++] = j * 8 + 3;
      widx[q++] = j * 8 + 2;

      widx[q++] = j * 8 + 4;
      widx[q++] = j * 8 + 6;
      widx[q++] = j * 8 + 5;
      widx[q++] = j * 8 + 6;
      widx[q++] = j * 8 + 7;
      widx[q++] = j * 8 + 5;

      // Line data ! (thankfully they're all black)
      int s = j * 12;
      ldat[s++] = float(x);
      ldat[s++] = float(y);
      ldat[s++] = c.heights[Cell::SOUTH + Cell::WEST];
      ldat[s++] = float(x + 1);
      ldat[s++] = float(y);
      ldat[s++] = c.heights[Cell::SOUTH + Cell::EAST];
      ldat[s++] = float(x + 1);
      ldat[s++] = float(y + 1);
      ldat[s++] = c.heights[Cell::NORTH + Cell::EAST];
      ldat[s++] = float(x);
      ldat[s++] = float(y + 1);
      ldat[s++] = c.heights[Cell::NORTH + Cell::WEST];

      int t = j * 8;
      // We disable lines by doubling indices
      lidx[t++] = 4 * j + 0;
      lidx[t++] = 4 * j + (c.flags & (CELL_NOLINESOUTH | CELL_NOGRID) ? 0 : 1);
      lidx[t++] = 4 * j + 1;
      lidx[t++] = 4 * j + (c.flags & (CELL_NOLINEEAST | CELL_NOGRID) ? 1 : 2);
      lidx[t++] = 4 * j + 2;
      lidx[t++] = 4 * j + (c.flags & (CELL_NOLINENORTH | CELL_NOGRID) ? 2 : 3);
      lidx[t++] = 4 * j + 3;
      lidx[t++] = 4 * j + (c.flags & (CELL_NOLINEWEST | CELL_NOGRID) ? 3 : 0);

      // Water
      int wvis = c.isWaterVisible();
      if (wvis) {
        int u = j * 8 * 5;
        packWaterCell(&fdat[u], x, y, c.waterHeights[Cell::SOUTH + Cell::WEST], c.velocity,
                      0.0f, 0.0f);
        packWaterCell(&fdat[u + 8], x + 1, y, c.waterHeights[Cell::SOUTH + Cell::EAST],
                      c.velocity, 1.0f, 0.0f);
        packWaterCell(&fdat[u + 16], x + 1, y + 1, c.waterHeights[Cell::NORTH + Cell::EAST],
                      c.velocity, 1.0f, 1.0f);
        packWaterCell(&fdat[u + 24], x, y + 1, c.waterHeights[Cell::NORTH + Cell::WEST],
                      c.velocity, 0.0f, 1.0f);
        packWaterCell(&fdat[u + 32], x + 0.5f, y + 0.5f, c.waterHeights[Cell::CENTER],
                      c.velocity, 0.5f, 0.5f);

        int v = j * 12;
        ushort f = (ushort)5 * j;
        fidx[v++] = f + 0;
        fidx[v++] = f + 1;
        fidx[v++] = f + 4;
        fidx[v++] = f + 1;
        fidx[v++] = f + 2;
        fidx[v++] = f + 4;
        fidx[v++] = f + 2;
        fidx[v++] = f + 3;
        fidx[v++] = f + 4;
        fidx[v++] = f + 3;
        fidx[v++] = f + 0;
        fidx[v++] = f + 4;
      } else {
        // Zero tile (safer than uninitialized)
        memset(&fdat[j * 5 * 8], 0, 5 * 8 * sizeof(GLfloat));
        for (int i = 0; i < 12; i++) { fidx[j * 12 + i] = 0; }
      }
    }
  }

  // Tiles
  glBindBuffer(GL_ARRAY_BUFFER, chunk->tile_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 5 * 8 * sizeof(GLfloat), tdat,
               GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->tile_vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 12 * sizeof(ushort), tidx,
               GL_STREAM_DRAW);
  // Walls
  glBindBuffer(GL_ARRAY_BUFFER, chunk->wall_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 8 * 8 * sizeof(GLfloat), wdat,
               GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->wall_vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 12 * sizeof(ushort), widx,
               GL_STREAM_DRAW);
  // Lines
  glBindBuffer(GL_ARRAY_BUFFER, chunk->line_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 4 * 3 * sizeof(GLfloat), ldat,
               GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->line_vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 8 * sizeof(ushort), lidx,
               GL_STREAM_DRAW);
  // Water (alt layer ?)
  glBindBuffer(GL_ARRAY_BUFFER, chunk->flui_vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 5 * 8 * sizeof(GLfloat), fdat,
               GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->flui_vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, CHUNKSIZE * CHUNKSIZE * 12 * sizeof(ushort), fidx,
               GL_STREAM_DRAW);

  delete[] tdat;
  delete[] tidx;
  delete[] wdat;
  delete[] widx;
  delete[] ldat;
  delete[] lidx;
  delete[] fdat;
  delete[] fidx;

  // Update view parameters
  chunk->minHeight = minz;
  chunk->maxHeight = maxz;
  chunk->is_updated = 0;
}

void Map::drawMapVBO(int birdseye, int cx, int cy, int stage) {
  // Cost to pay once and only once
  static long tickno = 0;
  static GLfloat lastTime = 0.f;
  // Record render cycles
  GLfloat gameTime =
      (Game::current ? Game::current->gameTime : ((EditMode*)GameMode::current)->time);
  if (gameTime != lastTime) {
    tickno++;
    lastTime = gameTime;
  }

  static GLuint shaderprogram = -1;
  static GLuint lineprogram = -1;
  static GLuint waterprogram = -1;
  static GLuint vao;
  static GLuint textureloc;
  static GLuint wtextureloc;
  if (shaderprogram == (GLuint)-1) {
    shaderprogram = loadProgramFromFiles("basic.vert", "basic.frag");
    if (shaderprogram == (GLuint)-1) { return; }
    lineprogram = loadProgramFromFiles("line.vert", "line.frag");
    if (lineprogram == (GLuint)-1) { return; }
    waterprogram = loadProgramFromFiles("water.vert", "water.frag");
    if (waterprogram == (GLuint)-1) { return; }
    textureloc = glGetUniformLocation(shaderprogram, "tex");
    wtextureloc = glGetUniformLocation(waterprogram, "wtex");
    glGenVertexArrays(1, &vao);
    // Or: Array_Texture, indexed by short&(short,short)/alpha
    // (is precise enough to cover animation, save memory, yet wraparound)
  }

  double t0 = getSystemTime();

  // Load matrices from other GL
  GLfloat proj[16];
  GLfloat model[16];
  GLdouble proj_d[16];
  GLdouble model_d[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);
  glGetFloatv(GL_MODELVIEW_MATRIX, model);
  glGetDoublev(GL_PROJECTION_MATRIX, proj_d);
  glGetDoublev(GL_MODELVIEW_MATRIX, model_d);

  int origx = cx - cx % CHUNKSIZE, origy = cy - cy % CHUNKSIZE;
  int prad = (VISRADIUS / CHUNKSIZE) + 1;

  int nchunks = 0;
  Chunk* drawlist[1024];
  for (int i = -prad; i <= prad; i++) {
    char debugline[128];
    for (int j = -prad; j < +prad; j++) {
      int hx = origx + i * CHUNKSIZE, hy = origy + j * CHUNKSIZE;
      Chunk* cur = chunk(hx, hy);
      int update = 0;
      if (stage == 0) {
        // Detect if update needed. Birdseye does't require retexturing
        update = cur->is_updated;

        int visible = testBboxClip(cur->xm, cur->xm + CHUNKSIZE, cur->ym, cur->ym + CHUNKSIZE,
                                   cur->minHeight, cur->maxHeight, model_d, proj_d);

        // Current cell is in viewport
        int ox = hx + CHUNKSIZE / 2;
        int oy = hy + CHUNKSIZE / 2;
        int iscent = max(abs(ox - cx), abs(oy - cy)) <= CHUNKSIZE / 2;
        int inrad = (ox - cx) * (ox - cx) + (oy - cy) * (oy - cy) < 2 * VISRADIUS * VISRADIUS;
        visible = visible && inrad;

        // Mark self with Z:!
        debugline[j] = iscent ? 'Z' : (visible ? 'X' : '.');

        cur->is_visible = visible;
      }
      if (cur->is_visible) {
        cur->last_shown = tickno;
        drawlist[nchunks] = cur;
        if (update || !cur->is_active) { fillChunkVBO(drawlist[nchunks]); }
        nchunks++;
      } else {
        // Cleanup buffers for zones that have long since dropped out of view
        if (cur->is_active && cur->last_shown < tickno - 10) {
          glDeleteBuffers(2, &cur->tile_vbo[0]);
          glDeleteBuffers(2, &cur->flui_vbo[0]);
          glDeleteBuffers(2, &cur->wall_vbo[0]);
          glDeleteBuffers(2, &cur->line_vbo[0]);
          cur->is_active = 0;
        }
      }
    }
    if (stage == 0 && 0) {
      debugline[2 * prad + 1] = '\0';
      printf("%s\n", debugline);
    }
  }

  double t1 = getSystemTime();

  // The obligatory VAO
  glBindVertexArray(vao);

  // Put into shader
  glUseProgram(shaderprogram);
  glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "proj_matrix"), 1, GL_FALSE,
                     (GLfloat*)&proj[0]);
  glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "model_matrix"), 1, GL_FALSE,
                     (GLfloat*)&model[0]);
  GLint fogActive = (Game::current && Game::current->fogThickness != 0);
  glUniform1i(glGetUniformLocation(shaderprogram, "fog_active"), fogActive);
  glUniform1i(glGetUniformLocation(shaderprogram, "render_stage"), stage);
  glUniform1f(glGetUniformLocation(shaderprogram, "gameTime"), gameTime);

  // Link in texture atlas :-)
  glUniform1i(textureloc, 0);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, textures[tx_Map]);

  // Run through ye olde draw loop
  // WALLS
  for (int i = 0; i < nchunks; i++) {
    glBindBuffer(GL_ARRAY_BUFFER, drawlist[i]->wall_vbo[0]);
    //                    #  W   type      norm  stride (skip)  offset
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(5 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 2, GL_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    // Index & draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawlist[i]->wall_vbo[1]);
    glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
  }

  // TOPS
  for (int i = 0; i < nchunks; i++) {
    glBindBuffer(GL_ARRAY_BUFFER, drawlist[i]->tile_vbo[0]);
    //                    #  W   type      norm  stride (skip)  offset
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(5 * sizeof(GLfloat)));
    glVertexAttribPointer(3, 2, GL_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                          (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    // Index & draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawlist[i]->tile_vbo[1]);
    glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
  }

  if (stage == 1) {
    glUseProgram(waterprogram);
    glUniformMatrix4fv(glGetUniformLocation(waterprogram, "proj_matrix"), 1, GL_FALSE,
                       (GLfloat*)&proj[0]);
    glUniformMatrix4fv(glGetUniformLocation(waterprogram, "model_matrix"), 1, GL_FALSE,
                       (GLfloat*)&model[0]);
    glUniform1f(glGetUniformLocation(waterprogram, "gameTime"), gameTime);
    glUniform1i(glGetUniformLocation(waterprogram, "fog_active"), fogActive);
    glUniform1i(wtextureloc, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Map]);

    // Water
    for (int i = 0; i < nchunks; i++) {
      glBindBuffer(GL_ARRAY_BUFFER, drawlist[i]->flui_vbo[0]);
      //                    #  W   type      norm  stride (skip)  offset
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
      glVertexAttribPointer(1, 4, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                            (void*)(3 * sizeof(GLfloat)));
      glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                            (void*)(5 * sizeof(GLfloat)));
      glVertexAttribPointer(3, 2, GL_SHORT, GL_TRUE, 8 * sizeof(GLfloat),
                            (void*)(6 * sizeof(GLfloat)));
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glEnableVertexAttribArray(3);
      // Index & draw
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawlist[i]->flui_vbo[1]);
      glDrawElements(GL_TRIANGLES, CHUNKSIZE * CHUNKSIZE * 12, GL_UNSIGNED_SHORT, (void*)0);
    }
  }

  glUseProgram(lineprogram);
  glUniformMatrix4fv(glGetUniformLocation(lineprogram, "proj_matrix"), 1, GL_FALSE,
                     (GLfloat*)&proj[0]);
  glUniformMatrix4fv(glGetUniformLocation(lineprogram, "model_matrix"), 1, GL_FALSE,
                     (GLfloat*)&model[0]);
  glUniform1i(glGetUniformLocation(lineprogram, "fog_active"), fogActive);

  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glLineWidth(2.0f);
  for (int i = 0; i < nchunks; i++) {
    glBindBuffer(GL_ARRAY_BUFFER, drawlist[i]->line_vbo[0]);
    // Just dot connecting
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawlist[i]->line_vbo[1]);
    glDrawElements(GL_LINES, CHUNKSIZE * CHUNKSIZE * 8, GL_UNSIGNED_SHORT, (void*)0);
  }
  glDisable(GL_LINE_SMOOTH);

  // Revert to fixed-function
  glUseProgram(0);

  //  printf("VBO draw time %3.3fms (%3.3fms) on %d chunks. %d %d\n", 1e3 * (getSystemTime() -
  //  t1),
  //         1e3 * (t1 - t0), nchunks, stage, birdseye);
}

void Map::markCellUpdated(int x, int y) {
  Cell& c = cell(x, y);
  if (0 <= x && x < width && 0 <= y && y <= width) {
    Chunk& z = *chunk(x - x % CHUNKSIZE, y - y % CHUNKSIZE);
    z.is_updated = 1;
    c.displayListDirty = 1;

    // Extend bounding box
    for (int k = 0; k < 5; k++) {
      z.minHeight = min(z.minHeight, c.heights[k]);
      z.minHeight = min(z.minHeight, c.waterHeights[k]);
      z.maxHeight = max(z.maxHeight, c.heights[k]);
      z.maxHeight = max(z.maxHeight, c.waterHeights[k]);
    }
  }
}

void Map::drawFootprint(int x, int y, int kind) {
  Cell& center = cell(x, y);
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);

  if (Settings::settings->gfx_details >= 4) {
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  }
  glLineWidth(4);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glTranslatef(0, 0, 0.02);

  glDisable(GL_LIGHTING);
  glBegin(GL_LINE_LOOP);
  {
    if (kind == 0)
      glColor4f(0.2, 0.2, 0.5, 1.0);
    else if (kind == 1)
      glColor4f(0.5, 0.2, 0.2, 1.0);
    glVertex3f(x, y, center.heights[Cell::SOUTH + Cell::WEST]);
    glVertex3f(x + 1, y, center.heights[Cell::SOUTH + Cell::EAST]);
    glVertex3f(x + 1, y + 1, center.heights[Cell::NORTH + Cell::EAST]);
    glVertex3f(x, y + 1, center.heights[Cell::NORTH + Cell::WEST]);
  }
  glEnd();

  glLineWidth(1);
  glPopMatrix();
  glPopAttrib();
}

void Map::drawCell(int birdsEye, int stage, int x, int y) {
  Coord3d normal, normal1;
  Real texScale = 0.25;
  int i, draw;
  int gfx_details = Settings::settings->gfx_details;
  double txOffset = 0.0, tyOffset = 0.0;
  Cell& c = cell(x, y);

  if (c.flags & CELL_ICE) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Ice]);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  } else if (c.flags & CELL_ACID) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Acid]);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  } else if (c.flags & CELL_SAND) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Sand]);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  } else if (c.flags & CELL_TRACK) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Track]);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  } else if (c.texture >= 0) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[c.texture]);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
  }
  if (c.flags & CELL_TRACK) {
    txOffset =
        -(Game::current ? Game::current->gameTime : ((EditMode*)GameMode::current)->time) *
        c.velocity[0] * texScale;
    tyOffset =
        -(Game::current ? Game::current->gameTime : ((EditMode*)GameMode::current)->time) *
        c.velocity[1] * texScale;
  }
  /* TODO. Do txOffset as a texture matrix transformation instead? */

  int topTransparent = 0;
  for (i = 0; i < 5; i++)
    if (c.colors[i][3] < 0.95) topTransparent = 1;

  /*** Draw the filled content of the top of this cell ***/
  if ((topTransparent && stage == 1) || (!topTransparent && stage == 0) || !isTransparent) {
    if (gfx_details >= 1)
      glBegin(GL_TRIANGLE_FAN);
    else
      glBegin(GL_QUADS);
    {
      /* Specify vertices */
      if (gfx_details >= 1) {
        c.getNormal(normal, Cell::CENTER);
        glNormal3dv(normal);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.colors[Cell::CENTER]);
        glTexCoord2f((c.textureCoords[0][0] + c.textureCoords[1][0] + c.textureCoords[2][0] +
                      c.textureCoords[3][0]) /
                             4.0 +
                         txOffset,
                     (c.textureCoords[0][1] + c.textureCoords[1][1] + c.textureCoords[2][1] +
                      c.textureCoords[3][1]) /
                             4.0 +
                         tyOffset);
        glVertex3f(x + 0.5, y + 0.5, c.heights[Cell::CENTER]);
      }

      /* Note the use of "normal1" here so we can reuse it for the last vertice */
      c.getNormal(normal1, Cell::SOUTH + Cell::WEST);
      glNormal3dv(normal1);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.colors[Cell::SOUTH + Cell::WEST]);
      glTexCoord2f(c.textureCoords[Cell::SOUTH + Cell::WEST][0] + txOffset,
                   c.textureCoords[Cell::SOUTH + Cell::WEST][1] + tyOffset);
      glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);

      c.getNormal(normal, Cell::SOUTH + Cell::EAST);
      glNormal3dv(normal);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.colors[Cell::SOUTH + Cell::EAST]);
      glTexCoord2f(c.textureCoords[Cell::SOUTH + Cell::EAST][0] + txOffset,
                   c.textureCoords[Cell::SOUTH + Cell::EAST][1] + tyOffset);
      glVertex3f(x + 1.01, y, c.heights[Cell::SOUTH + Cell::EAST]);

      c.getNormal(normal, Cell::NORTH + Cell::EAST);
      glNormal3dv(normal);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.colors[Cell::NORTH + Cell::EAST]);
      glTexCoord2f(c.textureCoords[Cell::NORTH + Cell::EAST][0] + txOffset,
                   c.textureCoords[Cell::NORTH + Cell::EAST][1] + tyOffset);
      glVertex3f(x + 1.01, y + 1.01, c.heights[Cell::NORTH + Cell::EAST]);

      c.getNormal(normal, Cell::NORTH + Cell::WEST);
      glNormal3dv(normal);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.colors[Cell::NORTH + Cell::WEST]);
      glTexCoord2f(c.textureCoords[Cell::NORTH + Cell::WEST][0] + txOffset,
                   c.textureCoords[Cell::NORTH + Cell::WEST][1] + tyOffset);
      glVertex3f(x, y + 1.01, c.heights[Cell::NORTH + Cell::WEST]);

      if (gfx_details >= 1) {
        glNormal3dv(normal1);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.colors[Cell::SOUTH + Cell::WEST]);
        glTexCoord2f(c.textureCoords[Cell::SOUTH + Cell::WEST][0] + txOffset,
                     c.textureCoords[Cell::SOUTH + Cell::WEST][1] + tyOffset);
        glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);
      }
    }
    glEnd();
  }
  glDisable(GL_TEXTURE_2D);
  glMaterialf(GL_FRONT, GL_SHININESS, 0.0);

  /*** Draw the lines around this cell */
  if (stage == 0 && !(c.flags & CELL_NOGRID)) {
    glPolygonMode(GL_FRONT, GL_LINE);
    GLfloat gridColor[4] = {0.0, 0.0, 0.0, 0.8};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gridColor);
    glNormal3f(0.0, 0.0, 1.0);
    /* Use polygon offset to avoid stippling of the lines */
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0, -1.0);

    if (0 && Settings::settings->gfx_details >= 4) {
      /* SORRY - no antialiasing for now! We have to draw the cells in two passes for this and
       * I don't want to that for efficiency now (two display lists!) */
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glLineWidth(1.0);
      if (Settings::settings->gfx_details >= 5) {
        /* TODO. Should we realy use polygon smooth here? */
        glEnable(GL_POLYGON_SMOOTH);
      }
    } else {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
      glLineWidth(1.0);
    }

    glBegin(GL_QUADS);
    if (c.flags & CELL_NOLINESOUTH)
      glEdgeFlag(GL_FALSE);
    else
      glEdgeFlag(GL_TRUE);
    glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);
    if (c.flags & CELL_NOLINEEAST)
      glEdgeFlag(GL_FALSE);
    else
      glEdgeFlag(GL_TRUE);
    glVertex3f(x + 1.01, y, c.heights[Cell::SOUTH + Cell::EAST]);
    if (c.flags & CELL_NOLINENORTH)
      glEdgeFlag(GL_FALSE);
    else
      glEdgeFlag(GL_TRUE);
    glVertex3f(x + 1.01, y + 1.01, c.heights[Cell::NORTH + Cell::EAST]);
    if (c.flags & CELL_NOLINEWEST)
      glEdgeFlag(GL_FALSE);
    else
      glEdgeFlag(GL_TRUE);
    glVertex3f(x, y + 1.01, c.heights[Cell::NORTH + Cell::WEST]);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT, GL_FILL);
  }

  /* Draw south side of cell */
  {
    Cell& c2 = cell(x, y - 1);
    draw = 0;
    int southTransparent = c.wallColors[Cell::SOUTH + Cell::EAST][3] < 0.95 ||
                           c.wallColors[Cell::SOUTH + Cell::WEST][3] < 0.95 ||
                           c2.wallColors[Cell::NORTH + Cell::EAST][3] < 0.95 ||
                           c2.wallColors[Cell::NORTH + Cell::WEST][3] < 0.95;
    if ((southTransparent && stage == 1) || (!southTransparent && stage == 0) ||
        !isTransparent) {
      if (c2.heights[Cell::NORTH + Cell::WEST] < c.heights[Cell::SOUTH + Cell::WEST] ||
          c2.heights[Cell::NORTH + Cell::EAST] < c.heights[Cell::SOUTH + Cell::EAST]) {
        glNormal3f(0.0, -1.0, 0.0);
        draw = 1;
      } else
        glNormal3f(0.0, +1.0, 0.0);

      if (birdsEye || draw) {
        glBegin(GL_TRIANGLE_STRIP);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     c2.wallColors[Cell::NORTH + Cell::EAST]);
        glVertex3f(x + 1.01, y, c2.heights[Cell::NORTH + Cell::EAST]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::SOUTH + Cell::EAST]);
        glVertex3f(x + 1.01, y, c.heights[Cell::SOUTH + Cell::EAST]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     c2.wallColors[Cell::NORTH + Cell::WEST]);
        glVertex3f(x, y, c2.heights[Cell::NORTH + Cell::WEST]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::SOUTH + Cell::WEST]);
        glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);
        glEnd();
      }
    }
  }

  /* Draw west side of cell */
  {
    Cell& c2 = cell(x - 1, y);
    draw = 0;
    int westTransparent = c.wallColors[Cell::SOUTH + Cell::WEST][3] < 0.95 ||
                          c.wallColors[Cell::NORTH + Cell::WEST][3] < 0.95 ||
                          c2.wallColors[Cell::SOUTH + Cell::EAST][3] < 0.95 ||
                          c2.wallColors[Cell::NORTH + Cell::EAST][3] < 0.95;
    if ((westTransparent && stage == 1) || (!westTransparent && stage == 0) ||
        !isTransparent) {
      if (c2.heights[Cell::SOUTH + Cell::EAST] < c.heights[Cell::SOUTH + Cell::WEST] ||
          c2.heights[Cell::NORTH + Cell::EAST] < c.heights[Cell::NORTH + Cell::WEST]) {
        glNormal3f(-1.0, 0.0, 0.0);
        draw = 1;
      } else
        glNormal3f(+1.0, 0.0, 0.0);

      if (birdsEye || draw) {
        glColor4f(0.0, 0.0, 0.0, 1.0);
        glBegin(GL_TRIANGLE_STRIP);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     c2.wallColors[Cell::SOUTH + Cell::EAST]);
        glVertex3f(x, y, c2.heights[Cell::SOUTH + Cell::EAST]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::SOUTH + Cell::WEST]);
        glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,
                     c2.wallColors[Cell::NORTH + Cell::EAST]);
        glVertex3f(x, y + 1.01, c2.heights[Cell::NORTH + Cell::EAST]);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::NORTH + Cell::WEST]);
        glVertex3f(x, y + 1.01, c.heights[Cell::NORTH + Cell::WEST]);
        glEnd();
      }
    }
  }

  /******************* draw water ****************/
  if (stage == 1 && (c.waterHeights[0] > c.heights[0] || c.waterHeights[1] > c.heights[1] ||
                     c.waterHeights[2] > c.heights[2] || c.waterHeights[3] > c.heights[3] ||
                     c.waterHeights[4] > c.heights[4])) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[tx_Ice]);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0);
    txOffset =
        -(Game::current ? Game::current->gameTime : ((EditMode*)GameMode::current)->time) *
        c.velocity[0] * texScale;
    tyOffset =
        -(Game::current ? Game::current->gameTime : ((EditMode*)GameMode::current)->time) *
        c.velocity[1] * texScale;

    if (gfx_details >= 1)
      glBegin(GL_TRIANGLE_FAN);
    else
      glBegin(GL_POLYGON);
    {
      GLfloat colors[4] = {0.3, 0.3, 0.7, 0.6};

      /* Specify vertices */
      if (gfx_details >= 1) {
        c.getWaterNormal(normal, Cell::CENTER);
        glNormal3dv(normal);
        // colors[3]=max(0.7,(c.waterHeights[Cell::CENTER]-c.heights[Cell::CENTER])*1.0);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colors);
        glTexCoord2f((0.5 + x) * texScale + txOffset, (0.5 + y) * texScale + tyOffset);
        glVertex3f(x + 0.5, y + 0.5, c.waterHeights[Cell::CENTER]);
      }

      c.getWaterNormal(normal, Cell::SOUTH + Cell::WEST);
      glNormal3dv(normal);
      // colors[3]=max(0.60,(c.waterHeights[Cell::SOUTH+Cell::WEST]-c.heights[Cell::SOUTH+Cell::WEST])*1.0);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colors);
      glTexCoord2f((0.0 + x) * texScale + txOffset + smoothSemiRand(x, y, 1.0),
                   (0.0 + y) * texScale + tyOffset + smoothSemiRand(x + 17, y, 1.0));
      glVertex3f(x, y, c.waterHeights[Cell::SOUTH + Cell::WEST]);

      c.getWaterNormal(normal, Cell::SOUTH + Cell::EAST);
      glNormal3dv(normal);
      // colors[3]=max(0.60,(c.waterHeights[Cell::SOUTH+Cell::EAST]-c.heights[Cell::SOUTH+Cell::EAST])*1.0);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colors);
      glTexCoord2f((1.0 + x) * texScale + txOffset + smoothSemiRand(x + 1, y, 0.5),
                   (0.0 + y) * texScale + tyOffset + smoothSemiRand(x + 17, y, 0.5));
      glVertex3f(x + 1.01, y, c.waterHeights[Cell::SOUTH + Cell::EAST]);

      c.getWaterNormal(normal, Cell::NORTH + Cell::EAST);
      glNormal3dv(normal);
      // colors[3]=max(0.60,(c.waterHeights[Cell::NORTH+Cell::EAST]-c.heights[Cell::NORTH+Cell::EAST])*1.0);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colors);
      glTexCoord2f((1.0 + x) * texScale + txOffset + smoothSemiRand(x + 1, y, 0.5),
                   (1.0 + y) * texScale + tyOffset + smoothSemiRand(x + 17, y + 1, 0.5));
      glVertex3f(x + 1.01, y + 1.01, c.waterHeights[Cell::NORTH + Cell::EAST]);

      c.getWaterNormal(normal, Cell::NORTH + Cell::WEST);
      glNormal3dv(normal);
      // colors[3]=max(0.60,(c.waterHeights[Cell::NORTH+Cell::WEST]-c.heights[Cell::NORTH+Cell::WEST])*1.0);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colors);
      glTexCoord2f((0.0 + x) * texScale + txOffset + smoothSemiRand(x, y, 0.5),
                   (1.0 + y) * texScale + tyOffset + smoothSemiRand(x + 17, y + 1, 0.5));
      glVertex3f(x, y + 1.01, c.waterHeights[Cell::NORTH + Cell::WEST]);

      c.getWaterNormal(normal, Cell::SOUTH + Cell::WEST);
      glNormal3dv(normal);
      // colors[3]=max(0.60,(c.waterHeights[Cell::SOUTH+Cell::WEST]-c.heights[Cell::SOUTH+Cell::WEST])*1.0);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colors);
      glTexCoord2f((0.0 + x) * texScale + txOffset + smoothSemiRand(x, y, 0.5),
                   (0.0 + y) * texScale + tyOffset + smoothSemiRand(x + 17, y, 0.5));
      glVertex3f(x, y, c.waterHeights[Cell::SOUTH + Cell::WEST]);
    }
    glEnd();

    glMaterialf(GL_FRONT, GL_SHININESS, 0.0);
    glDisable(GL_TEXTURE_2D);
  }
}

/****************************************************
 drawCellAA
****************************************************/

void Map::drawCellAA(int birdsEye, int x, int y) {
  int draw;

  Cell& c = cell(x, y);

  /* Draw south side of cell */
  {
    Cell& c2 = cell(x, y - 1);
    draw = 0;
    if (c2.heights[Cell::NORTH + Cell::WEST] < c.heights[Cell::SOUTH + Cell::WEST] ||
        c2.heights[Cell::NORTH + Cell::EAST] < c.heights[Cell::SOUTH + Cell::EAST]) {
      glNormal3f(0.0, -1.0, 0.0);
      draw = 1;
    } else
      glNormal3f(0.0, +1.0, 0.0);

    if (birdsEye || draw) {
      glBegin(GL_LINES);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c2.wallColors[Cell::NORTH + Cell::EAST]);
      glVertex3f(x + 1.01, y, c2.heights[Cell::NORTH + Cell::EAST]);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::SOUTH + Cell::EAST]);
      glVertex3f(x + 1.01, y, c.heights[Cell::SOUTH + Cell::EAST]);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c2.wallColors[Cell::NORTH + Cell::WEST]);
      glVertex3f(x, y, c2.heights[Cell::NORTH + Cell::WEST]);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::SOUTH + Cell::WEST]);
      glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);
      glEnd();
    }
  }

  /* Draw west side of cell */
  {
    Cell& c2 = cell(x - 1, y);
    draw = 0;
    if (c2.heights[Cell::SOUTH + Cell::EAST] < c.heights[Cell::SOUTH + Cell::WEST] ||
        c2.heights[Cell::NORTH + Cell::EAST] < c.heights[Cell::NORTH + Cell::WEST]) {
      glNormal3f(-1.0, 0.0, 0.0);
      draw = 1;
    } else
      glNormal3f(+1.0, 0.0, 0.0);

    if (birdsEye || draw) {
      glBegin(GL_LINES);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c2.wallColors[Cell::SOUTH + Cell::EAST]);
      glVertex3f(x, y, c2.heights[Cell::SOUTH + Cell::EAST]);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::SOUTH + Cell::WEST]);
      glVertex3f(x, y, c.heights[Cell::SOUTH + Cell::WEST]);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c2.wallColors[Cell::NORTH + Cell::EAST]);
      glVertex3f(x, y + 1.01, c2.heights[Cell::NORTH + Cell::EAST]);
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, c.wallColors[Cell::NORTH + Cell::WEST]);
      glVertex3f(x, y + 1.01, c.heights[Cell::NORTH + Cell::WEST]);
      glEnd();
    }
  }
}

Chunk* Map::chunk(int cx, int cy) {
  if (cx % CHUNKSIZE != 0 || cy % CHUNKSIZE != 0) {
    fprintf(stderr, "Bad chunk access %d %d\n", cx, cy);
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
          c.maxHeight = max(c.maxHeight, max(w.heights[k], w.waterHeights[k]));
          c.minHeight = min(c.minHeight, min(w.heights[k], w.waterHeights[k]));
        }
      }
    }
    chunks[cpos] = c;
  }
  return &chunks[cpos];
}

/** Saves the map to file in compressed binary or compressed ascii format */
int Map::save(char* pathname, int x, int y) {
  int i, version = mapFormatVersion;

  if (pathIsLink(pathname)) {
    fprintf(stderr, "Error, %s is a link, cannot save map\n", pathname);
    return 0;
  }

  gzFile gp = gzopen(pathname, "wb9");
  if (!gp) return 0;
  startPosition[0] = x;
  startPosition[1] = y;
  startPosition[2] = cell(x, y).heights[Cell::CENTER];
  int32_t data[6];
  for (i = 0; i < 3; i++)
    data[i] = saveInt((int32_t)startPosition[i]);  // no decimal part needed
  data[3] = saveInt((int32_t)width);
  data[4] = saveInt((int32_t)height);
  data[5] = saveInt((int32_t)version);
  gzwrite(gp, data, sizeof(int32_t) * 6);

  /* new from version 7, save texture names */
  data[0] = saveInt(numTextures);
  gzwrite(gp, data, sizeof(int32_t) * 1);
  for (i = 0; i < numTextures; i++) {
    char textureName[64];
    snprintf(textureName, 63, textureNames[i]);
    gzwrite(gp, textureName, 64);
  }

  for (i = 0; i < width * height; i++) cells[i].dump(gp);
  gzclose(gp);
  return 1;
}

void Cell::dump(gzFile gp) const {
  int32_t data[8];
  int i, j;
  for (i = 0; i < 5; i++) data[i] = saveInt((int32_t)(heights[i] / 0.0025));
  gzwrite(gp, data, sizeof(int32_t) * 5);
  for (i = 0; i < 5; i++) {
    for (j = 0; j < 4; j++) data[j] = saveInt((int32_t)(colors[i][j] / 0.01));
    gzwrite(gp, data, sizeof(int32_t) * 4);
  }
  data[0] = saveInt((int32_t)flags);
  data[1] = saveInt((int32_t)texture);
  gzwrite(gp, data, sizeof(int32_t) * 2);

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) data[j] = saveInt((int32_t)(wallColors[i][j] / 0.01));
    gzwrite(gp, data, sizeof(int32_t) * 4);
  }

  data[0] = saveInt((int32_t)(velocity[0] / 0.01));
  data[1] = saveInt((int32_t)(velocity[1] / 0.01));
  gzwrite(gp, data, sizeof(int32_t) * 2);

  for (i = 0; i < 5; i++) data[i] = saveInt((int32_t)(waterHeights[i] / 0.0025));
  gzwrite(gp, data, sizeof(int32_t) * 5);

  for (i = 0; i < 4; i++) {
    data[i * 2] = texture >= 0 || flags ? saveInt((int32_t)(textureCoords[i][0] / 0.0025)) : 0;
    data[i * 2 + 1] =
        texture >= 0 || flags ? saveInt((int32_t)(textureCoords[i][1] / 0.0025)) : 0;
  }
  gzwrite(gp, data, sizeof(int32_t) * 8);
}

void Cell::load(Map* map, gzFile gp, int version) {
  int32_t data[8];
  int i, j;

  gzread(gp, data, sizeof(int32_t) * 5);
  for (i = 0; i < 5; i++) heights[i] = 0.0025 * loadInt(data[i]);
  for (i = 0; i < 5; i++) {
    if (version < 4) {
      // old maps do not have an alpha channel defined
      gzread(gp, data, sizeof(int32_t) * 3);
      for (j = 0; j < 3; j++) colors[i][j] = 0.01 * loadInt(data[j]);
      colors[i][3] = 1.0;
    } else {
      gzread(gp, data, sizeof(int32_t) * 4);
      for (j = 0; j < 4; j++) colors[i][j] = 0.01 * loadInt(data[j]);
    }
  }

  gzread(gp, data, sizeof(int32_t) * 2);
  flags = loadInt(data[0]);
  if (version <= 1) flags = flags & (CELL_ICE | CELL_ACID);
  i = loadInt(data[1]);
  texture = (i >= 0 && i < numTextures ? map->indexTranslation[i] : -1);
  // in older maps, this field was not initialized
  if (version < 5) texture = -1;
  if (version < 3) { /* Old maps do not have wallColors defined */
    for (i = 0; i < 4; i++) {
      wallColors[i][0] = 0.7;
      wallColors[i][1] = 0.2;
      wallColors[i][2] = 0.2;
      wallColors[i][3] = 1.0;
    }
  } else {
    for (i = 0; i < 4; i++) {
      if (version < 4) {
        // old maps do not have an alpha channel defined
        gzread(gp, data, sizeof(int32_t) * 3);
        for (j = 0; j < 3; j++) wallColors[i][j] = 0.01 * loadInt(data[j]);
        wallColors[i][3] = 1.0;
      } else {
        gzread(gp, data, sizeof(int32_t) * 4);
        for (j = 0; j < 4; j++) wallColors[i][j] = 0.01 * loadInt(data[j]);
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
    for (i = 0; i < 5; i++) waterHeights[i] = 0.0025 * loadInt(data[i]);
  } else {
    for (i = 0; i < 5; i++) waterHeights[i] = heights[i] - 0.5;
  }
  sunken = 0.0;

  if (version >= 7) {
    gzread(gp, data, sizeof(int32_t) * 8);
    for (i = 0; i < 4; i++) {
      float tx = 0.0025 * loadInt(data[i * 2]);
      float ty = 0.0025 * loadInt(data[i * 2 + 1]);
      if (tx || ty) {
        textureCoords[i][0] = tx;
        textureCoords[i][1] = ty;
      }
    }
  }
}

/* Returns the height at a specified (floating point) position */
/* now inlined
Real Map::getHeight(Real x,Real y) {
  int ix=(int) x;
  int iy=(int) y;
  Cell& c = cell(ix,iy);
  return c.getHeight(x-ix,y-iy);
}
*/
