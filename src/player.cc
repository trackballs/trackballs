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

static Uint32 getFilteredRelativeMouse(double *xrate, double *yrate) {
  /* Finite impulse response filter to smooth mouse input */
  const int N = 1000;
  const double T = 0.2;
  static int fir[N][2];
  static double ts[N];
  static int first = true;
  if (first) {
    for (int i = 0; i < N; i++) {
      fir[i][0] = 0;
      fir[i][1] = 0;
      ts[i] = -1. - T;
    }
    first = false;
  }

  double now = Game::current->gameTime;

  int mouseX, mouseY;
  Uint32 mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
  if (mouseX || mouseY) {
    /* Add value to filter */

    int cx = mouseX, cy = mouseY;
    double ct = now;
    int i = 0;
    for (; ts[i] > now - T; i++) {
      std::swap(fir[i][0], cx);
      std::swap(fir[i][1], cy);
      std::swap(ts[i], ct);
    }
    std::swap(fir[i][0], cx);
    std::swap(fir[i][1], cy);
    std::swap(ts[i], ct);
  }

  int sx = 0, sy = 0;
  for (int i = 0; ts[i] > now - T; i++) {
    if (ts[i] < now - T / 2) {
      sx += fir[i][0];
      sy += fir[i][1];
    } else {
      sx += 2 * fir[i][0];
      sy += 2 * fir[i][1];
    }
  }
  *xrate = sx / T / 2;
  *yrate = sy / T / 2;
  return mouseState;
}

Player::Player() : Ball(Role_Player) {
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
void Player::tick(Real t) {
  static Real timeFraction = 0.;

  /* Never let us drop below 0 points, it just looks silly */
  if (score < 0) score = 0;

  if (!Game::current) return;
  if (!playing) return;

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

  double dx = 0., dy = 0.;
  /* Joysticks */
  if (Settings::settings->hasJoystick()) {
    if (Settings::settings->joystickButton(0)) key(' ');

    double joyX = Settings::settings->joystickX();
    double joyY = Settings::settings->joystickY();
    /* Help keep the joysticks centered if neccessary */
    if (joyX < 0.1 && joyX > -0.1) joyX = 0.0;
    if (joyY < 0.1 && joyY > -0.1) joyY = 0.0;

    dx = ((double)joyX) * Settings::settings->mouseSensitivity;
    dy = ((double)joyY) * Settings::settings->mouseSensitivity;
  }

  /* Give only *relative* mouse movements */
  if (!Settings::settings->ignoreMouse && !(SDL_GetModState() & KMOD_CAPS)) {
    double sx, sy;
    getFilteredRelativeMouse(&sx, &sy);
    if (sx || sy) {
      dx = sx * Settings::settings->mouseSensitivity * 0.0005;
      dy = sy * Settings::settings->mouseSensitivity * 0.0005;
    }
  }

  /* Handle keyboard steering */
  const Uint8 *keystate = SDL_GetKeyboardState(NULL);
  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);

  int ix = 0, iy = 0;
  if (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_KP_8]) iy--;
  if (keystate[SDL_SCANCODE_DOWN] || keystate[SDL_SCANCODE_KP_2]) iy++;
  if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_KP_4]) ix--;
  if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_KP_6]) ix++;
  if (keystate[SDL_SCANCODE_KP_7] || keystate[SDL_SCANCODE_Q]) {
    ix--;
    iy--;
  }
  if (keystate[SDL_SCANCODE_KP_9] || keystate[SDL_SCANCODE_W]) {
    ix++;
    iy--;
  }
  if (keystate[SDL_SCANCODE_KP_1] || keystate[SDL_SCANCODE_A]) {
    ix--;
    iy++;
  }
  if (keystate[SDL_SCANCODE_KP_3] || keystate[SDL_SCANCODE_S]) {
    ix++;
    iy++;
  }
  if (ix || iy) {
    double len = std::sqrt(ix * ix + iy * iy);
    dx = ix / len;
    dy = iy / len;
  }

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

  /* Cap dx/dy to have total radius 1. */
  double len = std::sqrt(dx * dx + dy * dy);
  if (len > 1.) {
    dx /= len;
    dy /= len;
  }

  // Rotate by 45 degrees, introduce sqrt2 scale factor
  double tx = -(dy - dx);
  double ty = -(dy + dx);
  dx = tx;
  dy = ty;

  if (shift) {
    dx *= 0.5;
    dy *= 0.5;
  }

  /* set inherent base acceleration*/
  acceleration = 3.5 - 0.25 * Settings::settings->difficulty;

  if (!hasWon && is_on) {
    Ball::drive(dx, dy);
  } else {
    Ball::drive(0., 0.);
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
  Ball::queueJump(jumpStrength);
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
    Game::current->add(new Splash(center, vel, acidColor, 32.0, radius));
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
        Game::current->add(new Debris(this, pos, vel, 2.0 + 8.0 * frandom()));
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
  is_on = false;
}

void Player::setStartVariables() {
  sink = 0.0;
  rotation[0] = rotation[1] = 0.;
  velocity = Coord3d();
  playing = true;
  hasWon = 0;
  health = 1.0;
  oxygen = 1.0;
  inTheAir = 0;
  inPipe = 0;
  moveBurst = 0.0;
  is_on = true;
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
void Player::newLevel() {}
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
