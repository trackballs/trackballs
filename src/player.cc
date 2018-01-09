/* player.cc
   Represent The(/A?) player.

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

#include "player.h"

#include "animatedCollection.h"
#include "debris.h"
#include "game.h"
#include "mainMode.h"
#include "map.h"
#include "settings.h"
#include "sound.h"
#include "splash.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

Player::Player() : Ball() {
  inTheAir = 0;
  inPipe = 0;
  lives = 3;
  realRadius = 0.3;
  radius = realRadius;

  ballResolution = BALL_HIRES;

  /* Change our color to red */
  primaryColor[0] = 1.0;
  primaryColor[1] = 0.2;
  primaryColor[2] = 0.2;

  specularColor[0] = 1.0;
  specularColor[1] = 1.0;
  specularColor[2] = 1.0;

  timeLeft = 5;
  bounceFactor = .7;
  crashTolerance = 7;
  playing = false;
  score = 0;
  hasWon = 0;
  health = 1.0;
  oxygen = 1.0;
  moveBurst = 0.0;

  lastJoyX = lastJoyY = 0;
  setReflectivity(0.4, 0);

  scoreOnDeath = Game::defaultScores[SCORE_PLAYER][0];
  timeOnDeath = Game::defaultScores[SCORE_PLAYER][1];
}

Player::~Player() {}
void Player::draw() {
  if (!playing) return;
  Ball::draw();
}
void Player::tick(Real t) {
  double dx, dy;
  int superAccelerate = 0;
  static Real timeFraction = 0.;

  /* Never let us drop below 0 points, it just looks silly */
  if (score < 0) score = 0;

  if (!Game::current) return;
  if (!playing) return;

  dx = 0.0;
  dy = 0.0;
  Map *map = Game::current->map;
  health += t * 0.4;
  if (health > 1.0) health = 1.0;

  timeFraction += t;
  if (timeFraction > 1.0) {
    timeFraction -= 1.0;
    if (!hasWon) {
      timeLeft--;
      if (timeLeft < 15) playEffect(SFX_TIME_WARNING);
    }
  }
  if (timeLeft < 1) {
    /* Only die from running out of time if we are not running
       in sandbox mode. */
    if (Settings::settings->sandbox == 0)
      die(DIE_TIMEOUT);
    else {
      /* DEPRACATED: This is instead handled by animated::die and the
         scoreOnDeath/timeOnDeath variables */
      /*
      score -= 100;
      if(score < 0) score=0;
      timeLeft += 60;*/
    }
    return;
  }

  /* Check for oxygen by seeing if we are below water level. However:
     when we are in a pipe we might be below ground and thus below the water level even though
     it's not supposed to be water here.
  */
  if (map->getWaterHeight(position[0], position[1]) > position[2] + radius * 0.75 &&
      (!(inPipe && position[2] < map->getHeight(position[0], position[1])))) {
    oxygen -=
        (t / (12.0 - 2.0 * Settings::settings->difficulty)) / Game::current->oxygenFactor;
    if (oxygen <= 0.0) die(DIE_OTHER);
  } else
    oxygen = std::min(1.0, oxygen + (t / 4.0) / Game::current->oxygenFactor);

  /* Joysticks */
  if (Settings::settings->hasJoystick()) {
    if (Settings::settings->joystickButton(0)) key(' ');
    if (Settings::settings->joystickButton(1)) superAccelerate = 1;

    double joyX = Settings::settings->joystickX();
    double joyY = Settings::settings->joystickY();
    /* Help keep the joysticks centered if neccessary */
    if (joyX < 0.1 && joyX > -0.1) joyX = 0.0;
    if (joyY < 0.1 && joyY > -0.1) joyY = 0.0;

    dx = ((double)joyX) * Settings::settings->mouseSensitivity * t * 200.0;
    dy = ((double)joyY) * Settings::settings->mouseSensitivity * t * 200.0;
  }

  /* Give only *relative* mouse movements */
  if (!Settings::settings->ignoreMouse && !(SDL_GetModState() & KMOD_CAPS)) {
    int mouseX, mouseY;

    Uint32 mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

    if (mouseState & SDL_BUTTON_RMASK) superAccelerate = 1;
    if ((mouseX || mouseY)) {
      dx = mouseX * Settings::settings->mouseSensitivity * 0.1;
      dy = mouseY * Settings::settings->mouseSensitivity * 0.1;
    }
  }

  /* Handle keyboard steering */
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);
  double kscale = shift ? 200.0 : 100.0;

  if (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_KP_8]) { dy = -kscale * t; }
  if (keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_KP_2]) { dy = kscale * t; }
  if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_KP_4]) { dx = -kscale * t; }
  if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_KP_6]) { dx = kscale * t; }
  if (keystate[SDL_SCANCODE_KP_7] || keystate[SDL_SCANCODE_Q]) {
    dx = -kscale * t;
    dy = -kscale * t;
  }
  if (keystate[SDL_SCANCODE_KP_9] || keystate[SDL_SCANCODE_W]) {
    dx = kscale * t;
    dy = -kscale * t;
  }
  if (keystate[SDL_SCANCODE_KP_1] || keystate[SDL_SCANCODE_A]) {
    dx = -kscale * t;
    dy = kscale * t;
  }
  if (keystate[SDL_SCANCODE_KP_3] || keystate[SDL_SCANCODE_S]) {
    dx = kscale * t;
    dy = kscale * t;
  }
  if (keystate[SDL_SCANCODE_KP_ENTER] || keystate[SDL_SCANCODE_RETURN]) superAccelerate = 1;

  /* rotate control as by settings->rotateArrows */
  {
    double angle = Settings::settings->rotateSteering * M_PI / 4.0;
    double tmp = dx * cos(angle) - dy * sin(angle);
    dy = dy * cos(angle) + dx * sin(angle);
    dx = tmp;
  }

  /* rotate controls if the camera perspective is rotated */
  double angle = ((MainMode *)GameMode::current)->xyAngle * M_PI / 2.;
  if (angle) {
    double tmp = dx * cos(angle) - dy * sin(angle);
    dy = dy * cos(angle) + dx * sin(angle);
    dx = tmp;
  }

  /* Cap is the maximum normal acceleration we are allowed to perform (on average) per second
   */
  double cap = 140.0 - 10.0 * Settings::settings->difficulty;
  if (modTimeLeft[MOD_SPEED]) {
    dx *= 1.5;
    dy *= 1.5;
    cap = 200.0;
  }
  if (modTimeLeft[MOD_DIZZY]) cap = cap / 2.0;

  /* Limit size of actual movement according to "cap" */
  /* Uses a trick to avoid capping away movements from integer only devices (eg. mouse) while
   * having a high FPS */
  moveBurst = std::min(cap * 0.2, moveBurst + cap * t);
  double len = sqrt(dx * dx + dy * dy);
  if (len > moveBurst) {
    dx = dx * moveBurst / len;
    dy = dy * moveBurst / len;
    len = moveBurst;
  }
  moveBurst -= len;

  /*
  if(modTimeLeft[MOD_DIZZY]) {
        double tmp = dx * sin(Game::current->gameTime*M_PI*0.1) + dy *
  cos(Game::current->gameTime*M_PI*0.1);
        dy = dx * cos(Game::current->gameTime*M_PI*0.1) - dy *
  sin(Game::current->gameTime*M_PI*0.1);
        dx = tmp;
        }*/

  /* Do the movement. Also, if nitro is active then create debris after the ball */

  if (!modTimeLeft[MOD_FROZEN] && !hasWon && is_on) {
    rotation[0] -= 0.025 * (dy - dx);
    rotation[1] -= 0.025 * (dx + dy);

    if (superAccelerate && modTimeLeft[MOD_NITRO]) {
      double dirX = rotation[0];
      double dirY = rotation[1];
      double len = sqrt(dirX * dirX + dirY * dirY);
      static double nitroDebris = 0.0;
      nitroDebris += t;
      while (nitroDebris > 0.1) {
        nitroDebris -= 0.1;
        Debris *d = new Debris(this, position, velocity, 1.0 + frandom() * 2.0);
        d->position[0] += (frandom() - 0.5) * radius - velocity[0] * 0.1;
        d->position[1] += (frandom() - 0.5) * radius - velocity[1] * 0.1;
        d->position[2] += (frandom() - 0.5) * radius;
        d->modTimeLeft[MOD_GLASS] = -1.0;
        d->primaryColor[0] = 0.1;
        d->primaryColor[1] = 0.6;
        d->primaryColor[2] = 0.1;
      }
      if (len > 0.0) {
        dirX /= len;
        dirY /= len;
        rotation[0] += dirX * t * 10.0;
        rotation[1] += dirY * t * 10.0;
      }
    }
  }

  if (!hasWon)
    friction = 1.0;
  else
    friction = 10.0;
  Ball::tick(t);
}
void Player::key(int c) {
  switch (c) {
  case ' ':
    jump();
    break;
  case 'k':
    die(DIE_OTHER);
    break;
  }
}
void Player::jump() {
  double jumpStrength =
      Game::current->jumpFactor * (1.2 - 0.1 * Settings::settings->difficulty);
  jumpStrength *= modTimeLeft[MOD_JUMP] ? 5.0 : 3.0;
  Ball::jump(jumpStrength);
}

void Player::die(int how) {
  Map *map = Game::current->map;
  /* immortal when not playing (i.e., at level finish) */
  if (hasWon) return;
  if (!playing) return;

  Ball::die(how);
  /* death hooks may have altered `hasWon` and `playing` */
  if (hasWon) return;
  if (!playing) return;
  if (Game::current->map->isBonus) {
    MainMode::mainMode->bonusLevelComplete();
    return;
  }
  /* Only remove lives if we are not running in sandbox mode */
  if (Settings::settings->sandbox == 0)
    lives--;
  else {
    score -= 100;
    if (score < 0) score = 0;
  }
  playing = false;
  health = 0.0;

  Coord3d pos, vel;
  if (how == DIE_ACID) {
    GLfloat acidColor[4] = {0.1, 0.8, 0.1, 0.5};
    Coord3d vel;
    Coord3d center(position[0], position[1], map->getHeight(position[0], position[1]));
    new Splash(center, vel, acidColor, 32.0, radius);
  } else
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++) {
        Real a = i / 4.0 * M_PI2;
        Real b = (j + 0.5) / 4.0 * M_PI;
        pos[0] = position[0] + cos(a) * 0.25 * sin(b) * 2.0;
        pos[1] = position[1] + sin(a) * 0.25 * sin(b) * 2.0;
        pos[2] = position[2] + 0.25 * cos(b) + 0.5;
        vel[0] = velocity[0] + (sink ? 0.1 : 0.5) * 1 / 2048.0 * ((rand() % 2048) - 1024);
        vel[1] = velocity[1] + (sink ? 0.1 : 0.5) * 1 / 2048.0 * ((rand() % 2048) - 1024);
        vel[2] = velocity[2] + (sink ? 0.01 : 0.5) * 1 / 2048.0 * ((rand() % 2048) - 1024);
        new Debris(this, pos, vel, 2.0 + 8.0 * frandom());
      }

  if (how == DIE_CRASH)
    playEffect(SFX_PLAYER_DIES);
  else if (how == DIE_TIMEOUT)
    playEffect(SFX_TIMEOUT);
  else if (how == DIE_FF)
    playEffect(SFX_FF_DEATH);

  if (lives <= 0)
    ((MainMode *)GameMode::current)->playerLose();
  else
    ((MainMode *)GameMode::current)->playerDie();
  alive = 0;
}

void Player::setStartVariables() {
  sink = 0.0;
  rotation[0] = rotation[1] = 0.;
  velocity = Coord3d();
  playing = true;
  alive = 1;
  hasWon = 0;
  health = 1.0;
  oxygen = 1.0;
  inTheAir = 0;
  inPipe = 0;
  moveBurst = 0.0;
}

void Player::restart(Coord3d pos) {
  setStartVariables();
  position = pos;
  position[2] = Game::current->map->getHeight(position[0], position[1]) + radius;
  modTimeLeft[MOD_DIZZY] = 0.0;

  /* reset all mods */
  /*for(i=0;i<NUM_MODS;i++)
    modTimeLeft[i] = 0.0;*/
}
void Player::mouse(int /*state*/, int /*x*/, int /*y*/) {}
void Player::newLevel() { Game::current->balls->insert(this); }
void Player::setHealth(Real d) {
  if (d < health) health = d;
  if (health < 0.0) health = 0.0;
}
bool Player::crash(Real speed) {
  double espeed =
      modTimeLeft[MOD_GLASS] ? (1.5 * speed) / crashTolerance : speed / crashTolerance;
  setHealth(1.0 - espeed);
  return this->Ball::crash(speed);
}
