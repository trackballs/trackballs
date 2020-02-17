/* weather.h
   Adds purely decorative snow and rain

   Copyright (C) 2003-2004  Mathias Broxvall

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

#include "weather.h"

#include "game.h"
#include "player.h"
#include "settings.h"

#include <cstring>

Weather::Weather() {
  if (Settings::settings->gfx_details <= 2) max_weather_particles = 500;
  if (Settings::settings->gfx_details == 3) max_weather_particles = 1000;
  if (Settings::settings->gfx_details == 4) max_weather_particles = 2000;
  if (Settings::settings->gfx_details >= 5) max_weather_particles = 3000;
  kind = WEATHER_SNOW;
  clear();

  bufs[0] = 0;
  bufs[1] = 0;
  vao = 0;
}
Weather::~Weather() {
  if (bufs[0] == bufs[1]) {
    glDeleteBuffers(2, bufs);
    glDeleteVertexArrays(1, &vao);
  }
}

void Weather::tick(Real td) {
  static double count = 0.0;
  static double snowDrift = 0.0;

  if (Settings::settings->gfx_details <= 1) return;
  if (strength < 0.0) return;
  Player *player = Game::current->player1;

  if (kind == WEATHER_SNOW) {
    for (count += td * 300.0 * strength; count > 0.0; count -= 1.0) {
      Particle *p = &particles[next];
      next = (next + 1) % max_weather_particles;
      p->size = frandom() * (0.5 + 0.5 * strength);
      p->position[0] = player->position[0] + (frandom() - 0.5) * 20.0;
      p->position[1] = player->position[1] + (frandom() - 0.5) * 20.0;
      p->position[2] = player->position[2] + 4.0;
      p->velocity[0] = (frandom() - 0.5) * 1.0;
      p->velocity[1] = (frandom() - 0.5) * 1.0;
      p->velocity[2] = -1.0 - frandom();
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) p->corners[j][k] = frandom() * 0.06;
    }
    /* Make it look like the snow particles is drifting in the wind by changing
       their velocities randomly once every 2 seconds. */
    for (snowDrift += (max_weather_particles * td) / 2.0; snowDrift > 0.0; snowDrift -= 1.0) {
      Particle *p = &particles[nextSnowDrift];
      nextSnowDrift = (nextSnowDrift + 1) % max_weather_particles;
      p->velocity[0] += (frandom() - 0.5) * 0.5;
      p->velocity[1] += (frandom() - 0.5) * 0.5;
    }

  } else if (kind == WEATHER_RAIN) {
    for (count += td * 500.0 * strength; count > 0.0; count -= 1.0) {
      Particle *p = &particles[next];
      next = (next + 1) % max_weather_particles;
      p->size = frandom() * (0.5 + 0.5 * strength);
      p->position[0] = player->position[0] + (frandom() - 0.5) * 15.0;
      p->position[1] = player->position[1] + (frandom() - 0.5) * 15.0;
      p->position[2] = player->position[2] + 5.0;
      p->velocity[0] = 0.0;
      p->velocity[1] = 0.0;
      p->velocity[2] = -4.0 - p->size * 4.0;
    }
  }

  for (int i = 0; i < max_weather_particles; i++) {
    Particle *p = &particles[i];
    p->position[0] += p->velocity[0] * td;
    p->position[1] += p->velocity[1] * td;
    p->position[2] += p->velocity[2] * td;
  }
}

void Weather::draw2() {
  if (Settings::settings->gfx_details <= 1) return;
  if (strength == -1.0) return;
  if (activeView.calculating_shadows) return;

  if (bufs[0] == bufs[1]) {
    glGenBuffers(2, bufs);
    glGenVertexArrays(1, &vao);
    /* Create a fixed linear index */
    ushort *idxs = new ushort[3 * 3000];
    for (int i = 0; i < 3 * 3000; i++) { idxs[i] = i; }
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 3000 * sizeof(ushort), idxs, GL_STATIC_DRAW);
    delete[] idxs;
  }

  if (kind == WEATHER_RAIN) {
    /** Draw RAIN particles **/
    glEnable(GL_BLEND);
    glLineWidth(1.5);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    GLfloat h = Game::current->player1->position[2] - 6.0;
    int nactive = 0;
    for (int i = 0; i < max_weather_particles; i++) {
      if (particles[i].position[2] < h) continue;
      nactive++;
    }
    if (nactive <= 0) { return; }

    GLfloat *data = new GLfloat[2 * 3 * nactive];
    int j = 0;
    for (int i = 0; i < max_weather_particles; i++) {
      const Particle &p = particles[i];
      if (p.position[2] < h) continue;
      data[6 * j + 0] = p.position[0];
      data[6 * j + 1] = p.position[1];
      data[6 * j + 2] = p.position[2] + 0.2 * p.size;
      data[6 * j + 3] = p.position[0];
      data[6 * j + 4] = p.position[1];
      data[6 * j + 5] = p.position[2];
      j++;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, 2 * 3 * nactive * sizeof(GLfloat), data, GL_STATIC_DRAW);
    delete[] data;

    // Transfer data
    setActiveProgramAndUniforms(Shader_Line);
    glUniform4f(uniformLocations.line_color, 0.3, 0.3, 0.4, 0.7);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);
    glDrawElements(GL_LINES, (2 * nactive), GL_UNSIGNED_SHORT, (void *)0);
  } else if (kind == WEATHER_SNOW) {
    /** Draw SNOW particles **/
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    int nactive = 0;
    GLfloat h = Game::current->player1->position[2] - 5.0;
    for (int i = 0; i < max_weather_particles; i++) {
      // Note that < and >= or not opposite for nan
      if (particles[i].position[2] < h) continue;
      nactive++;
    }
    if (nactive <= 0) { return; }

    GLfloat *data = new GLfloat[3 * 8 * nactive];

    Color color(0.8, 0.8, 0.85, 1.);
    GLfloat flat[3] = {0.f, 0.f, 0.f};
    GLfloat txc[3][2] = {{0.5f, 0.f}, {0.f, 1.f}, {1.f, 1.f}};
    GLfloat size = 1.3f;

    char *pos = (char *)data;
    for (int i = 0; i < max_weather_particles; i++) {
      const Particle &p = particles[i];
      if (p.position[2] < h) continue;
      for (int k = 0; k < 3; k++) {
        pos += packObjectVertex(pos, p.position[0] + p.corners[k][0] * size,
                                p.position[1] + p.corners[k][1] * size,
                                p.position[2] + p.corners[k][2] * size, txc[k][0], txc[k][1],
                                color, flat);
      }
    }

    // Transfer data
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * 8 * nactive * sizeof(GLfloat), data, GL_STATIC_DRAW);
    delete[] data;

    setActiveProgramAndUniforms(Shader_Object);
    setObjectUniforms(Color(0., 0., 0., 1.), 1., Lighting_None);
    glBindTexture(GL_TEXTURE_2D, textureGlitter);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);

    configureObjectAttributes();
    glDrawElements(GL_TRIANGLES, 3 * nactive, GL_UNSIGNED_SHORT, (void *)0);
  }
}

void Weather::clear() {
  // Zero in case of changes
  memset(particles, 0, sizeof(particles));
  for (int i = 0; i < max_weather_particles; i++) { particles[i].position[2] = -10.0; }
  next = 0;
  nextSnowDrift = 0;
  strength = -1.0;
}

void Weather::snow(double s) {
  kind = WEATHER_SNOW;
  strength = s;
  if (strength < 0.0) strength = 0.0;
  if (strength > 1.0) strength = 1.0;
}
void Weather::rain(double s) {
  kind = WEATHER_RAIN;
  strength = s;
  if (strength < 0.0) strength = 0.0;
  if (strength > 1.0) strength = 1.0;
}
