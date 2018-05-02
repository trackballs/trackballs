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

Diamond::Diamond(Coord3d pos) : Animated(Role_OtherAnimated) {
  position = pos;

  specularColor[0] = specularColor[1] = specularColor[2] = 1.0;
  primaryColor[0] = 0.7;
  primaryColor[1] = 0.7;
  primaryColor[2] = 0.9;
  primaryColor[3] = 0.7;
  fade = 1.0;
  taken = 0;
}

int Diamond::generateBuffers(GLuint *&idxbufs, GLuint *&databufs) {
  if (fade <= 0.) { return 0; }
  allocateBuffers(1, idxbufs, databufs);

  GLfloat color[4];
  for (int i = 0; i < 4; i++) color[i] = primaryColor[i];
  color[3] *= fade;

  GLfloat flat[3] = {0.f, 0.f, 0.f};

  GLfloat data[8 * 8];
  packObjectVertex(&data[0], position[0], position[1], position[2] - .4, 0., 0., color, flat);
  for (int i = 0; i < 6; i++) {
    float v = i * 2.0 * M_PI / 6.0 + Game::current->gameTime;
    packObjectVertex(&data[(i + 1) * 8], position[0] + std::sin(v) * 0.25,
                     position[1] + std::cos(v) * 0.25, position[2], 0., 0., color, flat);
  }
  packObjectVertex(&data[7 * 8], position[0], position[1], position[2] + .4, 0., 0., color,
                   flat);

  ushort idxs[12][3] = {{0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 5}, {0, 5, 6}, {0, 6, 1},
                        {7, 2, 1}, {7, 3, 2}, {7, 4, 3}, {7, 5, 4}, {7, 6, 5}, {7, 1, 6}};

  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  glBufferData(GL_ARRAY_BUFFER, 8 * 8 * sizeof(GLfloat), data, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(ushort), idxs, GL_STATIC_DRAW);

  return 1;
}

void Diamond::drawBuffers1(GLuint * /*idxbufs*/, GLuint * /*databufs*/) {}

void Diamond::drawBuffers2(GLuint *idxbufs, GLuint *databufs) {
  if (fade <= 0.) { return; }

  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  setActiveProgramAndUniforms(shaderObject);
  glUniform4f(glGetUniformLocation(shaderObject, "specular"), specularColor[0],
              specularColor[1], specularColor[2], specularColor[3]);
  glUniform1f(glGetUniformLocation(shaderObject, "shininess"), 100.f);
  glBindTexture(GL_TEXTURE_2D, textures[loadTexture("blank.png")]);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxbufs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, databufs[0]);
  configureObjectAttributes();
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void *)0);
}

void Diamond::tick(Real t) {
  Coord3d v0;
  if (fade <= 0.0) return;
  v0 = Game::current->player1->position - position;
  if (length(v0) < 0.3 + Game::current->player1->radius) {
    if (!taken) onGet();
    taken = 1;
  } else {
    taken = 0;
    fade = std::min(1.0, fade + 0.5 * t);
  }
}
void Diamond::onGet() {
  Game::current->map->startPosition = position;
  playEffect(SFX_GOT_FLAG);
  fade = -14.0;

  Coord3d signPos;
  signPos = position;
  signPos[2] += 1.0;
  Game::current->addEntity(new Sign(_("Save point"), 6.0, 1.0, 60.0, signPos));
}
