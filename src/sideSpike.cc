/* sideSpikes.cc
   Dangerous spikes shooting from the sides

   Copyright (C) 2000  Mathias Broxvall
   		       Yannick Perret

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
#include "glHelp.h"
#include "animated.h"
#include "map.h"
#include "game.h"
#include "player.h"
#include "ball.h"
#include "sound.h"
#include "sideSpike.h"

using namespace std;

SideSpike::SideSpike(Coord3d position,Real speed,Real phase, int side) {
  assign(position,this->position);
  this->position[2] = Game::current->map->getHeight(position[0],position[1]) + 0.0;
  this->speed=speed;
  while(phase < 0.0) phase += 1.0;
  this->phase=phase;
  if ((side < 1)||(side > 4))
    {
    fprintf(stderr, "Warning: SideSpike (%f,%f) got invalide side (%d). Treated as '1'.\n",
                position[0], position[1], side);
    this->side = 1;
    }
  else
    this->side=side;
  this->soundDone = 0;
  this->offset = 0.;

  primaryColor[0] = 0.9;
  primaryColor[1] = 0.5;
  primaryColor[2] = 0.4;
  
  secondaryColor[0] = 0.9;
  secondaryColor[1] = 0.8;
  secondaryColor[2] = 0.5;
}

void SideSpike::draw() {
  int i;

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  // setup the local axes regards to 'side'
  glTranslatef(position[0], position[1], position[2] + 0.25);
  switch(side)
    {
    case 1:
      glRotatef(90., 0., 1., 0.);
      break;
    case 2:
      glRotatef(-90., 0., 1., 0.);
      break;
    case 3:
      glRotatef(-90., 1., 0., 0.);
      break;
    case 4:
      glRotatef(90., 1., 0., 0.);
      break;
    }
  glTranslatef(0., 0., -0.5);

  GLfloat white[4] = {0.0, 0.0, 0.0, 0.0};
  glMaterialfv(GL_FRONT,GL_SPECULAR,white);
  glMaterialf(GL_FRONT,GL_SHININESS,0.5);
  glShadeModel(GL_SMOOTH);
  glColor3f(1.0,1.0,1.0);


  GLfloat colorBase[4];
  for(i=0;i<3;i++)
    colorBase[i] = primaryColor[i];
  colorBase[3] = 0.0;
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colorBase);
  
  double x=0.,y=0.,z=offset;
  glBegin(GL_TRIANGLE_STRIP);
  for(i=0;i<=6;i++) {
	glNormal3f(sin(i/3.0*M_PI),cos(i/3.0*M_PI),0.0);
	glVertex3f(x+0.1*sin(i/3.0*M_PI),y+0.1*cos(i/3.0*M_PI),z-0.70);
	glVertex3f(x+0.1*sin(i/3.0*M_PI),y+0.1*cos(i/3.0*M_PI),z+0.0);
  }
  glEnd();

  GLfloat colorTop[4]; 
  for(i=0;i<3;i++)
    colorTop[i] = secondaryColor[i];
  colorTop[3] = 0.0;
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colorTop);
  
  double d1=1 / sqrt(10.0), d2=3 / sqrt(10.0);
  glBegin(GL_TRIANGLES);
  for(i=0;i<6;i++) {
	glNormal3f(d2*sin((i+0.0)/3.0*M_PI),d2*cos((i+0.0)/3.0*M_PI),d1);
	glVertex3f(x+0.1*sin(i/3.0*M_PI),y+0.1*cos(i/3.0*M_PI),z+0.0);
	glNormal3f(d2*sin((i+1.0)/3.0*M_PI),d2*cos((i+1.0)/3.0*M_PI),d1);
	glVertex3f(x+0.1*sin((i+1)/3.0*M_PI),y+0.1*cos((i+1)/3.0*M_PI),z+0.0);
	glNormal3f(d2*sin((i+0.5)/3.0*M_PI),d2*cos((i+0.5)/3.0*M_PI),d1);
	glVertex3f(x+0.0,y+0.0,z+0.3);
  }
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

void SideSpike::tick(Real t) {
  set<Ball*>::iterator iter = Ball::balls->begin();
  set<Ball*>::iterator end = Ball::balls->end();
  Ball *ball;
  double dist,dx,dy,speed,h;
  int is_sinking=0;

  if(is_on)
	phase += t * this->speed;

  double x=position[0],y=position[1],z=position[2];
  while(phase > 1.0) phase -= 1.0;
  h = - 0.3;
  if(phase < 0.4) {
	/* spike is low */
	z = h; //
  } else if(phase < 0.5) {
	/* spike rising */
	z = h + (phase - 0.4) / 0.1;
  } else if(phase < 0.9) {
	/* spike is high */
	z = h + 1.0;
	soundDone = 0;
  } else {
	/* spike is sinking */
	z = h + 1.0 - (phase - 0.9) / 0.1;
	is_sinking=1;
  }
  offset = z;

  // play a 'rising' sound if the ball is in the round
  Player *player = Game::current->player1;
  dist = sqrt((position[0]-player->position[0])*(position[0]-player->position[0]) +
              (position[1]-player->position[1])*(position[1]-player->position[1]));
  if ((dist < 9.)&&(!soundDone)&&(phase >= 0.4)&&(phase < 0.5))   {
    if (dist < 6.)
      playEffect(SFX_SPIKE, 0.66);
    else
      playEffect(SFX_SPIKE, 0.66*(9.-dist)/3.);
    soundDone = 1;

    }

  if (offset <= 0.05)
    return;  // no collision possible

  double sx,sy,bx,by;
  double od, aod;  // lateral distance to the spike
  int ol;  // true if possible bounce


  for(;iter!=end;iter++) {
	ball = *iter;
	
	if (! ball->alive)
	  continue;
	
	switch(side)
	  {
	  case 1:
	    sx = position[0] - 0.5 + (offset+0.29);
	    sy = position[1];
	    bx = position[0] - 0.5;
	    by = sy;

	    od = ball->position[1] - position[1];

	    if ( (ball->position[0] - (position[0] - 0.5) > offset+0.29) ||
        	 (ball->position[0] - (position[0] - 0.5) < 0.) )
	      ol = 0;
	    else
  	      ol = 1;
	    break;
	  case 2:
	    sx = position[0] + 0.5 - (offset+0.29);
	    sy = position[1];
	    bx = position[0] + 0.5;
	    by = sy;

	    od = ball->position[1] - position[1];

	    if ( (position[0] + 0.5 - ball->position[0] > offset+0.29) ||
        	 (position[0] + 0.5 - ball->position[0] < 0.) )
	      ol = 0;
	    else
  	      ol = 1;
	    break;
	  case 3:
	    sx = position[0];
	    sy = position[1] - 0.5 + (offset+0.29);
	    bx = sx;
	    by = position[1] - 0.5;

	    od = ball->position[0] - position[0];

	    if ( (ball->position[1] - (position[1] - 0.5) > offset+0.29) ||
        	 (ball->position[1] - (position[1] - 0.5) < 0.) )
	      ol = 0;
	    else
  	      ol = 1;
	    break;
	  case 4:
	    sx = position[0];
	    sy = position[1] + 0.5 - (offset+0.29);
	    bx = sx;
	    by = position[1] + 0.5;

	    od = ball->position[0] - position[0];

	    if ( (position[1] + 0.5 - ball->position[1] > offset+0.29) ||
        	 (position[1] + 0.5 - ball->position[1] < 0.) )
	      ol = 0;
	    else
  	      ol = 1;
	    break;
	  }
	dist= sqrt((sx-ball->position[0])*(sx-ball->position[0]) +
        	   (sy-ball->position[1])*(sy-ball->position[1]));

	// too far
	if (dist > ball->radius + 1. + 0.3)
	  continue;

        // killed
	if (dist < ball->radius)  {
		ball->die(DIE_CRASH);
	}
	
	if (od < 0.)
	  aod = -od;
	else
	  aod = od;
	// bounce ?
	if (ol && (aod < ball->radius + 0.1)) {
		switch(side)
		  {
		  case 1:
		  case 2:
		    ball->velocity[1] = -ball->velocity[1]*1.15;
		    if (od < 0.)
		      {
		      ball->position[1] -= 0.015;
		      }
		    else
		      {
		      ball->position[1] += 0.015;
		      }
		    break;
		  case 3:
		  case 4:
		    ball->velocity[0] = -ball->velocity[0]*1.15;
		    if (od < 0.)
		      {
		      ball->position[0] -= 0.015;
		      }
		    else
		      {
		      ball->position[0] += 0.015;
		      }
		    break;
		  }
	}
	
  }
}
