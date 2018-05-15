/* sign.cc
   Show rotating text at a point

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

#include "sign.h"

#include "game.h"

#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>

#define SIGN_SCALE 0.007

Sign::Sign(const char *string, Real l, Real s, Real r, const Coord3d &pos)
    : Animated(Role_OtherAnimated, 1) {
  position = pos;
  if (l <= 0.0) l = 1e10;
  life = l;
  scale = s;
  rotation = r;
  tot_rot = 0.0;
  if (Game::current->isNight) {
    primaryColor = Color(1., 1., 1., 1.);
  } else if (Game::current->fogThickness) {
    primaryColor = Color(0., 0., 0., 1.);
  } else {
    primaryColor = Color(1., 1., 1., 1.);
  }

  textimg = 0;
  mkTexture(string);
}

Sign::~Sign() {
  if (textimg) glDeleteTextures(1, &textimg);
}

void Sign::mkTexture(const char *string) {
  SDL_Color fgColor = {255, 255, 255, 255};

  if (textimg) glDeleteTextures(1, &textimg);
  SDL_Surface *text = TTF_RenderUTF8_Blended(ingameFont, string, fgColor);
  textimg = LoadTexture(text, texcoord);
  width = text->w;
  height = text->h;
  SDL_FreeSurface(text);

  /* Note oversided bounding box since the sign rotates */
  boundingBox[0][0] = -std::sqrt(2) * SIGN_SCALE * scale * width;
  boundingBox[1][0] = +std::sqrt(2) * SIGN_SCALE * scale * width;
  boundingBox[0][1] = -std::sqrt(2) * SIGN_SCALE * scale * width;
  boundingBox[1][1] = +std::sqrt(2) * SIGN_SCALE * scale * width;
  boundingBox[0][2] = -SIGN_SCALE * scale * height;
  boundingBox[1][2] = SIGN_SCALE * scale * height;
}

void Sign::generateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                           const GLuint *vaolist, bool /*mustUpdate*/) const {
  GLfloat flat[3] = {0.f, 0.f, 0.f};

  GLfloat data[8 * 8];
  char *pos = (char *)data;
  GLfloat dx = std::cos(M_PI * (tot_rot - 45.) / 180.) * SIGN_SCALE * scale * width;
  GLfloat dy = std::sin(M_PI * (tot_rot - 45.) / 180.) * SIGN_SCALE * scale * width;
  GLfloat dz = SIGN_SCALE * scale * height;

  Color color = primaryColor.toOpaque();
  color.v[3] = std::min(1.0, life) * 65535;

  pos += packObjectVertex(pos, position[0] + dx, position[1] + dy, position[2] + dz,
                          texcoord[0], texcoord[1], color, flat);
  pos += packObjectVertex(pos, position[0] + dx, position[1] + dy, position[2] - dz,
                          texcoord[0], texcoord[1] + texcoord[3], color, flat);
  pos += packObjectVertex(pos, position[0] - dx, position[1] - dy, position[2] + dz,
                          texcoord[0] + texcoord[2], texcoord[1], color, flat);
  pos += packObjectVertex(pos, position[0] - dx, position[1] - dy, position[2] - dz,
                          texcoord[0] + texcoord[2], texcoord[1] + texcoord[3], color, flat);

  pos += packObjectVertex(pos, position[0] - dx, position[1] - dy, position[2] + dz,
                          texcoord[0], texcoord[1], color, flat);
  pos += packObjectVertex(pos, position[0] - dx, position[1] - dy, position[2] - dz,
                          texcoord[0], texcoord[1] + texcoord[3], color, flat);
  pos += packObjectVertex(pos, position[0] + dx, position[1] + dy, position[2] + dz,
                          texcoord[0] + texcoord[2], texcoord[1], color, flat);
  pos += packObjectVertex(pos, position[0] + dx, position[1] + dy, position[2] - dz,
                          texcoord[0] + texcoord[2], texcoord[1] + texcoord[3], color, flat);

  glBindVertexArray(vaolist[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 8 * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
  ushort idxs[4][3] = {{0, 1, 2}, {1, 3, 2}, {4, 5, 6}, {5, 7, 6}};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(ushort), idxs, GL_STATIC_DRAW);
  configureObjectAttributes();
}

void Sign::drawBuffers1(const GLuint * /*vaolist*/) const {}

void Sign::drawBuffers2(const GLuint *vaolist) const {
  if (activeView.calculating_shadows) return;

  // Keep the depth function on but trivial so as to record depth values
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_ALWAYS);

  // Transfer data
  setActiveProgramAndUniforms(shaderObject);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), 0., 0., 0., 1.);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 0.);
  glUniform1i(glGetUniformLocation(shaderObject, "use_lighting"), 0);
  glBindTexture(GL_TEXTURE_2D, textimg);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void *)0);

  glDepthFunc(GL_LEQUAL);
}

void Sign::tick(Real t) {
  life -= t;
  if (life <= 0) {
    /* time to die... */
    glDeleteTextures(1, &texture);
    texture = 0;
    remove();
    // delete this;
  }
  tot_rot += t * rotation;
  while (tot_rot > 360.0) tot_rot -= 360.0;
}
