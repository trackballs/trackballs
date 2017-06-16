/* modPill.cc
   A "pill" which gives the player mods when taken

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

#include "modPill.h"

#include "debris.h"
#include "game.h"
#include "mainMode.h"
#include "map.h"
#include "player.h"
#include "sign.h"
#include "sound.h"

int isGoodPill[NUM_MODS] = {1, 1, 1, 0, 0, 0, 1, 1};

ModPill::ModPill(Real x, Real y, int kind, int time, int resurrecting)
    : Ball(), kind(kind), resurrecting(resurrecting), time(time) {
  no_physics = 1;
  realRadius = 0.2;
  radius = realRadius;

  /* Change our color to red */
  primaryColor[0] = 1.0;
  primaryColor[1] = 0.2;
  primaryColor[2] = 0.2;

  /* set bogus velocity for the rendering of speed mods */
  velocity[0] = 0.0;
  velocity[1] = -1.2;

  modTimeLeft[kind] = -1.0;
  clock = 0.0;

  if (kind == MOD_EXTRA_LIFE) {
    realRadius = 0.3;
    radius = realRadius;
    primaryColor[0] = 1.0;
    primaryColor[1] = 0.9;
    primaryColor[2] = 0.2;
    specularColor[0] = 1.0;
    specularColor[1] = 0.9;
    specularColor[2] = 0.2;
  }

  position[0] = x;
  position[1] = y;
  position[2] = Game::current->map->getHeight(position[0], position[1]) + radius;

  alive = 1;
}

ModPill::~ModPill() { this->Animated::~Animated(); }

void ModPill::tick(Real t) {
  Coord3d v;
  double dist;

  Player *player = Game::current->player1;
  if (!alive) {
    timeLeft -= t;
    if (resurrecting > 0.0 && timeLeft < 0) alive = 1;
  }

  clock += t;
  if (kind == MOD_LARGE) {
    realRadius = 0.2 * (1.0 + fmod(clock, 2.0) / 2.0);
    radius = realRadius;
  } else if (kind == MOD_SMALL) {
    realRadius = 0.2 / (1.0 + fmod(clock, 2.0) / 2.0);
    radius = realRadius;
  }

  if (alive && kind == MOD_NITRO) {
    nitroDebrisCount += t;
    while (nitroDebrisCount > 0.0) {
      nitroDebrisCount -= 0.25;
      Debris *d = new Debris(this, position, velocity, 2.0 + 2.0 * frandom());
      d->position[0] += (frandom() - 0.5) * radius;
      d->position[1] += (frandom() - 0.5) * radius;
      d->position[2] += radius * 1.0;
      d->velocity[0] = 0.0;
      d->velocity[1] = 0.0;
      d->velocity[2] = 0.2;
      d->gravity = -0.1;
      d->modTimeLeft[MOD_GLASS] = -1.0;
      d->primaryColor[0] = 0.1;
      d->primaryColor[1] = 0.6;
      d->primaryColor[2] = 0.1;
      d->no_physics = 1;
    }
  }

  if (alive) {
    position[2] = Game::current->map->getHeight(position[0], position[1]) + radius;
    sub(player->position, position, v);
    dist = length(v);
    if (dist < radius + player->radius) {
      Coord3d signPos;
      assign(position, signPos);
      signPos[2] += 1.0;

      /* Explanations of modpills shown after taking them. Note that only bad
         ones have an exclamation mark after them */
      const char *modExplanations[NUM_MODS] = {
          _("Speed ball"), _("Extra jump"), _("Spikes"),   _("Glass ball"),
          _("Dizzy!"),     _("Freeze!"),    _("Floating"), _("Extra life"),
          _("Small ball"), _("Large ball"), _("Nitro")};
      new Sign(modExplanations[kind], 6.0, 1.0, 60.0, signPos);

      if (kind == MOD_EXTRA_LIFE) {
        player->lives = std::min(4, player->lives + 1);
      } else {
        if (time >= 0.0)
          player->modTimeLeft[kind] += time;
        else
          player->modTimeLeft[kind] = -1.0;
      }

      if (kind == MOD_FROZEN) MainMode::mainMode->flash = 2.0;

      if (kind == MOD_EXTRA_LIFE)
        playEffect(SFX_GOT_LIFE);
      else if (isGoodPill[kind])
        playEffect(SFX_GOT_GOODPILL);
      else
        playEffect(SFX_GOT_BADPILL);

      alive = 0;
      if (!resurrecting) remove();
      timeLeft = (Real)resurrecting;
    }
  }
}
void ModPill::playerRestarted() {}
void ModPill::die(int /*how*/) {
  alive = 0;
  if (!resurrecting) delete this;
}
