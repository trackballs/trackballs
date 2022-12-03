/* diamond.cc
   Gives points when captured

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

#include "diamond.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "sign.h"
#include "sound.h"

Diamond::Diamond(Game &g, const Coord3d &pos) : Animated(g, Role_OtherAnimated, 1) {
  position = pos;

  specularColor = Color(1., 1., 1., 1.);
  primaryColor = Color(SRGBColor(0.7, 0.7, 0.9, 0.7));
  fade = 1.0;
  taken = 0;
}

void Diamond::updateBuffers(const GLuint *idxbufs, const GLuint *databufs,
                            const GLuint *vaolist, bool firstCall) {
  if (fade <= 0.) return;

  Color color = primaryColor;
  color.v[3] *= fade;

  if (firstCall || bufferColor != color) {
    GLfloat flat[3] = {0.f, 0.f, 0.f};

    GLfloat data[8 * 8];
    packObjectVertex(&data[0], 0., 0., -.4, 0., 0., color, flat);
    for (int i = 0; i < 6; i++) {
      float v = i * 2.0 * M_PI / 6.0;
      packObjectVertex(&data[(i + 1) * 8], std::sin(v) * 0.25, std::cos(v) * 0.25, 0., 0., 0.,
                       color, flat);
    }
    packObjectVertex(&data[7 * 8], 0., 0., .4, 0., 0., color, flat);

    ushort idxs[12][3] = {{0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 6}, {0, 6, 1},
                          {7, 2, 1}, {7, 3, 2}, {7, 4, 3}, {7, 5, 4}, {7, 6, 5}, {7, 1, 6}};

    glBindVertexArray(vaolist[0]);
    glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
    glBufferData(GL_ARRAY_BUFFER, 8 * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(ushort), idxs, GL_STATIC_DRAW);
    configureObjectAttributes();

    bufferColor = color;
  }
}

void Diamond::drawBuffers1(const GLuint * /*vaolist*/) const {}

void Diamond::drawBuffers2(const GLuint *vaolist) const {
  if (fade <= 0.) { return; }

  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  Matrix4d transform;
  affineMatrix(transform, identity3, position);
  rotateZ(game.gameTime, transform);
  const UniformLocations *uloc = setActiveProgramAndUniforms(Shader_Object);
  setObjectUniforms(uloc, transform, specularColor, 50.f, Lighting_Regular);
  glBindTexture(GL_TEXTURE_2D, textureBlank);

  glBindVertexArray(vaolist[0]);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void *)0);
}

void Diamond::tick(Real t) {
  Animated::tick(t);

  Coord3d v0;
  if (fade <= 0.0) return;
  v0 = game.player1->position - position;
  if (length(v0) < 0.3 + game.player1->radius) {
    if (!taken) onGet();
    taken = 1;
  } else {
    taken = 0;
    fade = std::min(1.0, fade + 0.5 * t);
  }
}
void Diamond::onGet() {
  game.map->startPosition = position;
  playEffect(SFX_GOT_FLAG);
  fade = -14.0;

  Coord3d signPos;
  signPos = position;
  signPos[2] += 1.0;
  game.add(new Sign(game, _("Save point"), 6.0, 1.0, 60.0, signPos));
}
