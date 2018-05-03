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
    : Ball(Role_OtherAnimated), kind(kind), resurrecting(resurrecting), time(time) {
  no_physics = true;
  realRadius = 0.2;
  radius = realRadius;
  is_on = true;

  /* Change our color to red */
  primaryColor[0] = 1.0;
  primaryColor[1] = 0.2;
  primaryColor[2] = 0.2;

  /* set bogus velocity for the rendering of speed mods */
  if (kind == MOD_SPEED) {
    velocity[0] = 0.0;
    velocity[1] = -1.2;
  } else {
    velocity = Coord3d();
  }

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

  timeLeft = 0.;
}

ModPill::~ModPill() { this->Animated::~Animated(); }

void ModPill::tick(Real t) {
  Player *player = Game::current->player1;
  if (!is_on) {
    timeLeft -= t;
    if (resurrecting > 0.0 && timeLeft < 0) is_on = true;
  }

  clock += t;
  if (kind == MOD_LARGE) {
    realRadius = 0.2 * (1.0 + fmod(clock, 2.0) / 2.0);
    radius = realRadius;
  } else if (kind == MOD_SMALL) {
    realRadius = 0.2 / (1.0 + fmod(clock, 2.0) / 2.0);
    radius = realRadius;
  }

  if (is_on && kind == MOD_NITRO) Ball::generateNitroDebris(t);

  if (is_on) {
    position[2] = Game::current->map->getHeight(position[0], position[1]) + radius;
    Coord3d v = player->position - position;
    double dist = length(v);
    if (dist < radius + player->radius) {
      Coord3d signPos = position;
      signPos[2] += 1.0;

      /* Explanations of modpills shown after taking them. Note that only bad
         ones have an exclamation mark after them */
      const char *modExplanations[NUM_MODS] = {
          _("Speed ball"), _("Extra jump"), _("Spikes"),   _("Glass ball"),
          _("Dizzy!"),     _("Freeze!"),    _("Floating"), _("Extra life"),
          _("Small ball"), _("Large ball"), _("Nitro")};
      Game::current->add(new Sign(modExplanations[kind], 6.0, 1.0, 60.0, signPos));

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

      is_on = false;
      if (!resurrecting) remove();
      timeLeft = (Real)resurrecting;
    }
  }
}

void ModPill::die(int /*how*/) {
  is_on = false;
  if (!resurrecting) remove();
}
