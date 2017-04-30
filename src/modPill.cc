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

#include "general.h"
#include "animated.h"
#include "modPill.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "sound.h"
#include "mainMode.h"
#include "settings.h"
#include "debris.h"
#include "sign.h"

using namespace std;

#define STATE_DEAD    0
#define STATE_ALIVE   1

int isGoodPill[NUM_MODS]={1,1,1,0,0,0,1,1};

/* Explanations of modpills shown after taking them. Note that only bad 
   ones have an exclamation mark after them */
char *modExplanations[NUM_MODS] = {"Speed ball","Extra jump","Spikes","Glass ball","Dizzy!","Freeze!",
								  "Floating","Extra life","Small ball","Large ball","Nitro"};

ModPill::ModPill(Coord3d position,int kind,int time,int resurrecting)
  :Ball(), kind(kind), time(time), resurrecting(resurrecting), 
   state(STATE_ALIVE) {
  assign(position,this->position);
  no_physics=1;
  realRadius=0.2; radius=realRadius;
  resolution=8;
  if(Settings::settings->gfx_details >= 4) resolution=9;
  if(Settings::settings->gfx_details >= 5) resolution=10;

  /* Change our color to red */  
  primaryColor[0] = 1.0;
  primaryColor[1] = 0.2;
  primaryColor[2] = 0.2;

  /* set bogus velocity for the rendering of speed mods */
  velocity[0] = 0.0;
  velocity[1] = -1.2;

  modTimeLeft[kind] = -1.0;
  clock=0.0;

  if(kind == MOD_EXTRA_LIFE) {
	resolution=9;
	if(Settings::settings->gfx_details >= 4) resolution=10;
	if(Settings::settings->gfx_details >= 5) resolution=12;
	realRadius=0.3; radius=realRadius;
	primaryColor[0] = 1.0;
	primaryColor[1] = 0.9;
	primaryColor[2] = 0.2;
	specularColor[0] = 1.0;
	specularColor[1] = 0.9;
	specularColor[2] = 0.2;
  }
}

ModPill::~ModPill() {
  this->Animated::~Animated();
}

void ModPill::draw() {
  if(state == STATE_ALIVE)
	Ball::draw();	
}
void ModPill::draw2() {
  if(state == STATE_ALIVE)
	Ball::draw2();	
}
void ModPill::tick(Real t) {
  Coord3d v;
  double dist;

  Player *player=Game::current->player1;
  if(state == STATE_DEAD) {
	timeLeft -= t;
	if(resurrecting > 0.0 && timeLeft < 0)
	  state = STATE_ALIVE;	  
  } 

  clock += t;
  if(kind == MOD_LARGE) {
	realRadius = 0.2 * (1.0 + fmod(clock,2.0) / 2.0);
	radius=realRadius;
  } else if(kind == MOD_SMALL) {
	realRadius = 0.2 / (1.0 + fmod(clock,2.0) / 2.0);
	radius=realRadius;
  }

  if(state == STATE_ALIVE && kind == MOD_NITRO) {
	nitroDebrisCount += t;
	while(nitroDebrisCount > 0.0) {
	  nitroDebrisCount -= 0.25;
	  Debris *d=new Debris(this,position,velocity,2.0+2.0*frandom());
	  d->position[0] += (frandom()-0.5)*radius;
	  d->position[1] += (frandom()-0.5)*radius;
	  d->position[2] += radius*1.0;
	  d->velocity[0] = 0.0;
	  d->velocity[1] = 0.0;
	  d->velocity[2] = 0.2;
	  d->gravity=-0.1;
	  d->modTimeLeft[MOD_GLASS]=-1.0;
	  d->primaryColor[0]=0.1;
	  d->primaryColor[1]=0.6;
	  d->primaryColor[2]=0.1;
	  d->no_physics=1;
	}
  }


  if(state == STATE_ALIVE) {	
	position[2] = Game::current->map->getHeight(position[0],position[1]) + radius;
	sub(player->position,position,v);
	dist = length(v);
	if(dist < radius + player->radius) {

	  Coord3d signPos;
	  assign(position,signPos);
	  signPos[2] += 1.0;
	  new Sign(modExplanations[kind],6.0,1.0,60.0,signPos);

	  if(kind == MOD_EXTRA_LIFE) {
		player->lives=min(4,player->lives+1);
	  } else {
		if(time >= 0.0)
		  player->modTimeLeft[kind] += time;
		else
		  player->modTimeLeft[kind] = -1.0;
	  }

	  if(kind == MOD_FROZEN) 
		MainMode::mainMode->flash=2.0;
	  
	  if(kind == MOD_EXTRA_LIFE) 
	    playEffect(SFX_GOT_LIFE);
	  else if(isGoodPill[kind])
	    playEffect(SFX_GOT_GOODPILL);
	  else
	    playEffect(SFX_GOT_BADPILL);
	  
	  state=STATE_DEAD;	  
	  if(!resurrecting) remove();
	  timeLeft = (Real) resurrecting;
	}
  }
}
void ModPill::playerRestarted() {
}
void ModPill::die(int how) { 
  state=STATE_DEAD; 
  if(!resurrecting) delete this;
}
