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

#include "general.h"
#include "weather.h"
#include "game.h"
#include "player.h"
#include "settings.h"

using namespace std;

Weather::Weather() {
  if(Settings::settings->gfx_details <= 2) max_weather_particles=500;
  if(Settings::settings->gfx_details == 3) max_weather_particles=1000;
  if(Settings::settings->gfx_details == 4) max_weather_particles=2000;
  if(Settings::settings->gfx_details >= 5) max_weather_particles=3000;
  kind=WEATHER_SNOW;
  clear();  
}

void Weather::tick(Real td) {
  int i,j,k;
  static double count=0.0;  
  static double snowDrift=0.0;  

  if(Settings::settings->gfx_details <= 1) return;
  if(strength < 0.0) return;
  Player *player = Game::current->player1;

  if(kind == WEATHER_SNOW) {

	for(count+=td*300.0*strength;count>0.0;count-=1.0) {
	  Particle *p = &particles[next];
	  next=(next+1)%max_weather_particles;
	  p->size=frandom()*(0.5+0.5*strength);
	  p->position[0] = player->position[0] + (frandom()-0.5) * 20.0;
	  p->position[1] = player->position[1] + (frandom()-0.5) * 20.0;
	  p->position[2] = player->position[2] + 4.0;
	  p->velocity[0] = (frandom()-0.5) * 1.0;
	  p->velocity[1] = (frandom()-0.5) * 1.0;
	  p->velocity[2] = -1.0-frandom();
	  for(j=0;j<3;j++) for(k=0;k<3;k++)
		p->corners[j][k] = frandom() * 0.06;
	} 
	/* Make it look like the snow particles is drifting in the wind by changing 
	   their velocities randomly once every 2 seconds. */
	for(snowDrift+=(max_weather_particles*td)/2.0;snowDrift>0.0;snowDrift-=1.0) {
	  Particle *p = &particles[nextSnowDrift];
	  nextSnowDrift=(nextSnowDrift+1)%max_weather_particles;
	  p->velocity[0] += (frandom()-0.5) * 0.5;
	  p->velocity[1] += (frandom()-0.5) * 0.5;
	}

  } else if(kind == WEATHER_RAIN) {

	for(count+=td*500.0*strength;count>0.0;count-=1.0) {
	  Particle *p = &particles[next];
	  next=(next+1)%max_weather_particles;
	  p->size=frandom()*(0.5+0.5*strength);
	  p->position[0] = player->position[0] + (frandom()-0.5) * 15.0;
	  p->position[1] = player->position[1] + (frandom()-0.5) * 15.0;
	  p->position[2] = player->position[2] + 5.0;
	  p->velocity[0] = 0.0;
	  p->velocity[1] = 0.0;
	  p->velocity[2] = -4.0-p->size*4.0;
	}	
  }
  
  for(i=0;i<max_weather_particles;i++) {
	Particle *p = &particles[i];
	p->position[0] += p->velocity[0] * td;
	p->position[1] += p->velocity[1] * td;
	p->position[2] += p->velocity[2] * td;
  }
}

void Weather::draw2() {
  int i,j;

  if(Settings::settings->gfx_details <= 1) return;
  if(strength == -1.0) return;

  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_LIGHTING);

  if(kind == WEATHER_RAIN) {
	/** Draw RAIN particles **/
	if(Settings::settings->gfx_details >= 4) {
	  glColor4f(0.3,0.3,0.4,0.5);
	  glEnable(GL_BLEND);
	  glLineWidth(1.5);	
	} else {
	  glColor4f(0.3,0.3,0.4,1.0);
	  glDisable(GL_BLEND);
	  glLineWidth(1.0);	
	}
	glBegin(GL_LINES);

	double h=Game::current->player1->position[2]-6.0;
	for(i=0;i<max_weather_particles;i++) {
	  Particle *p = &particles[i];
	  if(p->position[2] < h) continue;
	  glVertex3f(p->position[0],p->position[1],p->position[2]+0.2*p->size);
	  glVertex3f(p->position[0],p->position[1],p->position[2]);
	}
	glEnd();
  }
  else if(kind == WEATHER_SNOW) {
	/** Draw SNOW particles **/
	glColor4f(0.8,0.8,0.85,1.5);
	glDisable(GL_CULL_FACE);
	if(Settings::settings->gfx_details >= GFX_DETAILS_EXTRA) {
	  glEnable(GL_BLEND);
	  bindTexture("glitter.png");
	  glEnable(GL_TEXTURE_2D);
	  glBegin(GL_TRIANGLES);
	  double h=Game::current->player1->position[2]-5.0;
	  for(i=0;i<max_weather_particles;i++) {
		Particle *p = &particles[i];
		if(p->position[2] < h) continue;
		//glNormal3f(0.0,0.0,1.0);
		glTexCoord2f(0.5, 0.0);
		glVertex3f(p->position[0]+p->corners[0][0]*1.3,p->position[1]+p->corners[0][1]*1.3,p->position[2]+p->corners[0][2]*1.3);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(p->position[0]+p->corners[1][0]*1.3,p->position[1]+p->corners[1][1]*1.3,p->position[2]+p->corners[1][2]*1.3);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(p->position[0]+p->corners[2][0]*1.3,p->position[1]+p->corners[2][1]*1.3,p->position[2]+p->corners[2][2]*1.3);
	  }
	  glEnd();
	  glDisable(GL_TEXTURE_2D);
	} else {
	  glDisable(GL_BLEND);
	  glBegin(GL_TRIANGLES);
	  double h=Game::current->player1->position[2]-5.0;
	  for(i=0;i<max_weather_particles;i++) {
		Particle *p = &particles[i];
		if(p->position[2] < h) continue;
		//glNormal3f(0.0,0.0,1.0);
		for(j=0;j<3;j++) 
		  glVertex3f(p->position[0]+p->corners[j][0],p->position[1]+p->corners[j][1],p->position[2]+p->corners[j][2]);
	  }
	  glEnd();
	}

  }
  glPopAttrib();
  glPopMatrix();
}

void Weather::clear() {
  int i;
  for(i=0;i<max_weather_particles;i++) {
	particles[i].position[2]=-10.0;
  }
  next=0;
  nextSnowDrift=0;
  strength=-1.0;
}

void Weather::snow(double s) {
  kind=WEATHER_SNOW;
  strength=s;
  if(strength<0.0) strength=0.0;
  if(strength>1.0) strength=1.0;
}
void Weather::rain(double s) {
  kind=WEATHER_RAIN;
  strength=s;
  if(strength<0.0) strength=0.0;
  if(strength>1.0) strength=1.0;
}
