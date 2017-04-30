/* sparkle2d.cc
   manage 2D sparkles (in 2D mode, of course)

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
#include "sparkle2d.h"
#include "sound.h"


/*  structure for */

typedef struct _glitter
{
  float pos[3];
  float speed[3];
  float color[4];
  float size;
  float age;
  struct _glitter *next, *prev;
}Glitter;



// initialize sparkle module
Sparkle2D::Sparkle2D() {
  // just be sure that this field is empty
  this->sparkle_first = NULL;
}

// destroy
Sparkle2D::~Sparkle2D() {
  // remove all remaining sparkle
  clear();
}

// remove all entries
void Sparkle2D::clear() {
  Sparkle *tmp = sparkle_first;
  Sparkle *tmp2;

  while(tmp != NULL) {
  	tmp2 = tmp->next;
	free(tmp);
   	tmp = tmp2;
  }
  sparkle_first = NULL;
}


// remove a particular sparkle from the list
void Sparkle2D::remove_sparkle(Sparkle *sparkle)
{
  if (sparkle == NULL)
    return;
  if (sparkle->next != NULL)
    sparkle->next->prev = sparkle->prev;
  if (sparkle->prev != NULL)
    sparkle->prev->next = sparkle->next;
  else
    sparkle_first = sparkle->next;
  free(sparkle);
}


/*
 * create and insert a glitter
 */
Sparkle *Sparkle2D::create_and_insert()
{
  Sparkle *tmp = NULL;

  tmp = (Sparkle*)malloc(sizeof(Sparkle));
  if (tmp == NULL)
    {
    return(NULL);
    }
  tmp->next = sparkle_first;
  tmp->prev = NULL;
  if (sparkle_first != NULL)
    {
    sparkle_first->prev = tmp;
    }
  sparkle_first = tmp;
  return(tmp);
}


/*
 * add a new sparkel
 */
int Sparkle2D::add(float px, float py, float vx, float vy, float ttl, float size, float r, float g, float b, float a) {
  Sparkle *sparkle;

  if ((sparkle = create_and_insert()) == NULL)
    return(0);
  
  sparkle->pos[0] = px;
  sparkle->pos[1] = py;
  sparkle->speed[0] = vx;
  sparkle->speed[1] = vy;
  sparkle->color[0] = r;
  sparkle->color[1] = g;
  sparkle->color[2] = b;
  sparkle->color[3] = a;
  sparkle->size = size;
  sparkle->ttl = ttl;
  sparkle->age = 0.;
  
  return(1);
}
int Sparkle2D::add(float pos[2], float speed[2], float ttl, float size, float color[4])
{
  Sparkle *sparkle;

  if ((sparkle = create_and_insert()) == NULL)
    return(0);
  
  sparkle->pos[0] = pos[0];
  sparkle->pos[1] = pos[1];
  sparkle->speed[0] = speed[0];
  sparkle->speed[1] = speed[1];
  sparkle->color[0] = color[0];
  sparkle->color[1] = color[1];
  sparkle->color[2] = color[2];
  sparkle->color[3] = color[3];
  sparkle->size = size;
  sparkle->ttl = ttl;
  sparkle->age = 0.;
  
  return(1);
}
int Sparkle2D::add(float pos[2], float speed[2], float ttl, float size) {
  float col[4] = {1., 0.9, 0.1, 0.9};
  return add(pos, speed, ttl, size, col);
}
int Sparkle2D::add(float pos[2], float speed[2], float ttl) {
  float col[4] = {1., 0.9, 0.1, 0.9};
  return add(pos, speed, ttl, 1., col);
}
int Sparkle2D::add(float pos[2], float ttl) {
  float col[4] = {1., 0.9, 0.1, 0.9};
  float speed[2];
  
  speed[0] = 2.*frandom()-1.;
  speed[1] = 2.*frandom()-1.;
  return add(pos, speed, ttl, 1., col);
}
int Sparkle2D::add(float pos[2]) {
  float col[4] = {1., 0.9, 0.1, 0.9};
  float speed[2];
  
  speed[0] = 2.*frandom()-1.;
  speed[1] = 2.*frandom()-1.;
  return add(pos, speed, 0.5+frandom()*2., 1., col);
}


/*
 * draw existing glitters
 */
void Sparkle2D::draw()
{
  float  age;
  float  ex, ey;
  float  alpha,tmp;
  float  *clr;
  float  *pos;
  Sparkle *skl;
  GLuint sparkleTex;

  Enter2DMode();
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  bindTexture("glitter.png");
  skl = sparkle_first;
  while(skl != NULL)
    {
    age = skl->age;
    if( age >= 0.)
      {
      if (age < 0.1) {
      	tmp = 1. - age*10.;
      } else
        tmp = age/skl->ttl;
      alpha = 1.0-tmp;
      clr = skl->color;
      glColor4f(clr[0], clr[1], clr[2], clr[3]*alpha);
      tmp = alpha*alpha;
      ex = tmp*skl->size*0.8;
      ey = tmp*skl->size*1.+(0.02*age);
      pos = skl->pos;
      glPushMatrix();
	  glTranslatef(pos[0], pos[1], pos[2]);
	  glBegin(GL_QUADS);
	  glTexCoord2f(0.0, 0.0);
	  glVertex3f( -ex,  ey, 0.0);
	  glTexCoord2f(0.0, 1.0);
	  glVertex3f( -ex, -ey, 0.0);
	  glTexCoord2f(1.0, 1.0);
	  glVertex3f(  ex, -ey, 0.0);
	  glTexCoord2f(1.0, 0.0);
	  glVertex3f(  ex,  ey, 0.0);
	  glEnd();
      glPopMatrix();
      }
    skl = skl->next;
    }
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  Leave2DMode();
}

/*
 * update existing glitters
 */
void Sparkle2D::tick(Real t)
{
  Sparkle *skl, *tmp;

  skl = sparkle_first;
  while(skl != NULL)
    {
    skl->age += t;
    
    if (skl->age > skl->ttl)
      {
      tmp = skl;
      skl = skl->next;
      remove_sparkle(tmp);
      continue;
      }
    /* update pos */
    skl->pos[0] += skl->speed[0]*t;
    skl->pos[1] += skl->speed[1]*t;
    skl->speed[1] += 200.0*t; // MB. fix for varying framerates

    skl = skl->next;
    }
}
