/* ball.cc
   The class ball is used by all animated objects representing balls of different radiuses

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
#include "ball.h"
#include "map.h"
#include "gameMode.h"
#include "mainMode.h"
#include "game.h"
#include "debris.h"
#include "trampoline.h"
#include "forcefield.h"
#include "sound.h"
#include "settings.h"
#include "font.h"
#include "pipe.h"
#include "pipeConnector.h"
#include "splash.h"

using namespace std;

const Real Ball::physicsResolution = 0.002;
class set<Ball*> *Ball::balls;
GLuint Ball::dizzyTexture;
GLfloat Ball::dizzyTexMinX, Ball::dizzyTexMinY, Ball::dizzyTexMaxX, Ball::dizzyTexMaxY;
extern GLuint hiresSphere;

void Ball::init() {
  balls = new set<Ball *>();
  SDL_Surface *text;
  SDL_Color fgColor={255,255,255};
  GLfloat texcoord[4];

  text = TTF_RenderText_Blended(ingameFont, " ? ", fgColor);
  //dizzyTexture = LoadTexture(text, texcoord);
  loadTexture("dizzy.png");
  SDL_FreeSurface(text);
  dizzyTexMinX = 0.0;
  dizzyTexMinY = 0.0;
  dizzyTexMaxX = 1.0;
  dizzyTexMaxY = 1.0;    
}
void Ball::reset() {
  delete balls;
  balls = new set<Ball *>();
}
void Ball::onRemove() {Animated::onRemove(); balls->erase(this);}

Ball::Ball() :Animated() {
  int i;
  
  sink=0.0;

  ballResolution=BALL_LORES;
  zero(position);
  zero(velocity);
  friction = 1.0;
  rotation[0] = rotation[1] = 0.0;

  primaryColor[0] = 0.8;
  primaryColor[1] = 0.8;
  primaryColor[2] = 0.8;
  secondaryColor[0] = 0.5;
  secondaryColor[1] = 0.5;
  secondaryColor[2] = 0.5;

  gravity = 8.0;
  bounceFactor = 0.8;
  crashTolerance = 7;
  balls->insert(this);
  no_physics=0;
  inPipe=0;

  for(i=0;i<NUM_MODS;i++) {
	modTimeLeft[i] = 0.0;
	modTimePhaseIn[i] = 0.0;
  }

  identityMatrix(rotations);
  texture = loadTexture("ice.png");
  nitroDebrisCount=0.0;

  environmentTexture=0;
  reflectivity=0.0;
  metallic=0;
  dontReflectSelf=0;
}
Ball::~Ball() { 
  setReflectivity(0.0,0); // This effectivly deallocates all environment maps */
}

/*********************************************
 draw() - Draws all the opaque parts of ball 
**********************************************/
void Ball::draw() {
  int i,u,v;
  GLfloat color[4];
  GLfloat specular[4];
  double blend;
  double phase;

  // Fix when rendering environment map to not reflect oneself
  if(dontReflectSelf) return;

  for(i=0;i<3;i++) {
	color[i] = primaryColor[i];
	specular[i] = specularColor[i];
  }
  color[3] = 1.0;
  specular[3] = 1.0;
  double shininess = 10.0;

  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]-sink);

  
  glTranslatef(0., 0., 2.);
  glColor4f(1.,1.,1.,1.);
  //Font::drawSimpleText3D(0, "test#f", .2, .2);
  glTranslatef(0., 0., -2.);

  //glScalef(radius,radius,radius);
  //glCallList(loresSphere);
  if(modTimeLeft[MOD_GLASS]) {

	phase=min(modTimePhaseIn[MOD_GLASS]/2.0,1.0);
    if(modTimeLeft[MOD_GLASS] > 0)
	  phase=min(modTimeLeft[MOD_GLASS]/2.0,phase);
	else
	  phase=1.0;
	blend = phase;

    
    color[0] = 0.8 * blend + color[0] * (1.0 - blend); 
    color[1] = 0.8 * blend + color[1] * (1.0 - blend); 
    color[2] = 0.8 * blend + color[2] * (1.0 - blend);
    color[3] = 0.5 * blend + 1.0 * (1.0 - blend); 
    specular[0] = 2.5 * blend + specular[0] * (1.0 - blend);
    specular[1] = 2.5 * blend + specular[1] * (1.0 - blend);
    specular[2] = 2.5 * blend + specular[2] * (1.0 - blend);
    shininess = 50.0 * blend + shininess * (1.0 - blend);
    glEnable(GL_BLEND);
  } else if(modTimeLeft[MOD_FROZEN]) {

	phase=min(modTimePhaseIn[MOD_FROZEN]/2.0,1.0);
    if(modTimeLeft[MOD_FROZEN] > 0)
	  phase=min(modTimeLeft[MOD_FROZEN]/2.0,phase);
	else
	  phase=1.0;
	blend=phase;

    color[0] = 0.4 * blend + color[0] * (1.0 - blend); 
    color[1] = 0.4 * blend + color[1] * (1.0 - blend); 
    color[2] = 0.9 * blend + color[2] * (1.0 - blend);
    color[3] = 0.6 * blend + 1.0 * (1.0 - blend); 
    specular[0] = 2.0 * blend + specular[0] * (1.0 - blend);
    specular[1] = 2.0 * blend + specular[1] * (1.0 - blend);
    specular[2] = 4.0 * blend + specular[2] * (1.0 - blend);
    shininess = 50.0 * blend + shininess * (1.0 - blend);
    glEnable(GL_BLEND);
  }
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMaterialf(GL_FRONT,GL_SHININESS,shininess);	

  glPushMatrix();
  glMultMatrixd((GLdouble*)rotations);
  if(Settings::settings->gfx_details > 0) {	
	if(modTimeLeft[MOD_EXTRA_LIFE]) {
	  bindTexture("track.png");
	  //glBindTexture(GL_TEXTURE_2D, textures[3]);
	  glEnable(GL_TEXTURE_2D);
	  glRotated(Game::current->gameTime*60., 0.0,0.0,1.0);
	} else {
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, texture);  
	}
  }

  /*
  specular[0] = 1.0;
  specular[1] = 1.0;
  specular[2] = 1.0;
  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMaterialf(GL_FRONT,GL_SHININESS,10.0);	
  */



  //gluSphere(qball, radius, resolution, resolution);
  glPushMatrix();
  glEnable(GL_NORMALIZE);
  glScalef(radius,radius,radius);
  glCallList(sphereDisplayLists[ballResolution]);
  glDisable(GL_NORMALIZE);
  glPopMatrix();


  
  /* Draw reflection of environment */
  if(Settings::settings->doReflections && reflectivity > 0.0 && environmentTexture) {
	 //1 || (Game::current && (Ball*)Game::current->player1 == this && Settings::settings->gfx_details > 3)) {	
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	GLfloat c[4];
	if(metallic) { c[0]=color[0]; c[1]=color[1]; c[2]=color[2]; c[3]=reflectivity; }
	else { c[0]=1.0; c[1]=1.0; c[2]=1.0; c[3]=reflectivity; }
	glColor4fv(c);
	/*
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c);  
	// TODO. Apply specular highlighting after environment textures
	c[0]=1.0; c[1]=1.0; c[2]=1.0; c[3]=0.5;
	glMaterialfv(GL_FRONT,GL_SPECULAR,c);
	glMaterialf(GL_FRONT,GL_SHININESS,20.0);*/
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	//bindTexture("clouds.png");
	glBindTexture(GL_TEXTURE_2D,environmentTexture);

	//gluSphere(qball, radius, resolution, resolution);
	glPushMatrix();
	glEnable(GL_NORMALIZE);
	glScalef(radius,radius,radius);
	glCallList(sphereDisplayLists[ballResolution]);
	glDisable(GL_NORMALIZE);
	glPopMatrix();
	glPopAttrib();
  }

  if(modTimeLeft[MOD_SPIKE]) {
    glPushAttrib(GL_ENABLE_BIT);
    glColor4f(1.0,1.0,1.0,1.0);

	phase=min(modTimePhaseIn[MOD_SPIKE]/2.0,1.0);
    if(modTimeLeft[MOD_SPIKE] > 0)
	  phase=min(modTimeLeft[MOD_SPIKE]/2.0,phase);
	else
	  phase=1.0;
	glScalef(0.5+0.5*phase,0.5+0.5*phase,0.5+0.5*phase);

	for(i=0;i<3;i++)
	  color[i] = secondaryColor[i];
	glColor3f(1.0,1.0,1.0);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	color[0] = 0.2; color[1] = 0.2; color[2] = 0.2;
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,color);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,10.0);	

    for(u=0;u<6;u++)
      for(v=0;v<3;v++) {
		glPushMatrix();
		glRotated(v*60., 1.0,0.0,0.0);
		glRotated(u*60., 0.0,1.0,0.0);
		
		Coord3d a, b, c, d;
		
		// YP: only calculate the 4 pts, and use the 'drawSpike'
		//     new function, which draw the 3 triangles at once
		a[0] = radius*1.3; a[1] = 0.0; a[2] = 0.0;
		b[0] = 0.0; b[1] = +0.5 * radius; b[2] = -0.3*radius;
		c[0] = 0.0; c[1] = -0.5 * radius; c[2] = -0.3*radius;
		d[0] = 0.0; d[1] = 0.0; d[2] = +0.5*radius;

		drawSpike(a, b, c, d);
		glPopMatrix();
      }
    glPopAttrib();
  }

  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
  if(modTimeLeft[MOD_GLASS] || modTimeLeft[MOD_FROZEN])
	glDisable(GL_BLEND);

  if(modTimeLeft[MOD_SPEED]) {

	GLfloat stripes[4] = {5.0, 5.0, 5.0, 0.5};
	glEnable(GL_BLEND);
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,stripes);
	glMaterialf(GL_FRONT,GL_SHININESS,0.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);

	for(i=1;i<8;i++) {
	  double angle=M_PI/9.0*(i+0.5) - M_PI/2.0;
	  Coord3d v;
	  assign(velocity,v);
	  v[2]=0.0;
	  if(length(v) > 0.8) {
		normalize(v);
		double z = (rand() % 1000) / 1000.0;
		glVertex3f(sin(angle)*radius*v[1],sin(angle)*radius*-v[0],cos(angle)*radius*z);
		glVertex3f(sin(angle)*radius*v[1]-velocity[0]*0.4*cos(angle),sin(angle)*radius*-v[0]-velocity[1]*0.4*cos(angle),
				   cos(angle)*radius*z-velocity[2]*0.2);
	  }
	}
	glEnd();
	glDisable(GL_BLEND);
  }

  if(modTimeLeft[MOD_FLOAT]) {
	
	/*	if(Settings::settings->gfx_details >= 4) {
	  glEnable(GL_BLEND);
	  glEnable(GL_LINE_SMOOTH);
	  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	}

	if(screenWidth == 640)
	  glLineWidth(3.0);
	else
	  glLineWidth(5.0);
	*/

	//glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	for(i=0;i<10;i++) {
	  glNormal3f(0.0,0.0,1.0);
	  const GLfloat stripeA[4]={1.0,1.0,1.0,1.0};
	  const GLfloat stripeB[4]={1.0,0.2,0.2,1.0};
	  if(i%2 == 0) glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,stripeA);
	  else  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,stripeB);
	  double a0=(i/10.0)*2.*M_PI,a1=((i+1)/10.0)*2.*M_PI;

	  glBegin(GL_TRIANGLE_STRIP);
	  glNormal3f(-sin(a0),-cos(a0),0.0);
	  glVertex3f(sin(a0)*radius*1.0,cos(a0)*radius*1.0,0.0);
	  glNormal3f(-sin(a1),-cos(a1),0.0);
	  glVertex3f(sin(a1)*radius*1.0,cos(a1)*radius*1.0,0.0);
	  glNormal3f(0.0,0.0,1.0);
	  glVertex3f(sin(a0)*radius*1.2,cos(a0)*radius*1.2,0.2*radius);
	  glNormal3f(0.0,0.0,1.0);
	  glVertex3f(sin(a1)*radius*1.2,cos(a1)*radius*1.2,0.2*radius);
	  glNormal3f(sin(a0),cos(a0),0.0);
	  glVertex3f(sin(a0)*radius*1.4,cos(a0)*radius*1.4,0.0);
	  glNormal3f(sin(a1),cos(a1),0.0);
	  glVertex3f(sin(a1)*radius*1.4,cos(a1)*radius*1.4,0.0);
	  glEnd();
	}
  }

  glPopAttrib();
  glPopMatrix();
}
/***************************************************
 draw2() - Draws all the translucent parts of ball 
***************************************************/
void Ball::draw2() {
  int i;

  // Fix when rendering environment map to not reflect oneself
  if(dontReflectSelf) return;

  if(!(modTimeLeft[MOD_EXTRA_LIFE] || modTimeLeft[MOD_JUMP] || modTimeLeft[MOD_DIZZY])) return;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]-sink);
  glPushAttrib(GL_ENABLE_BIT);
  glEnable(GL_BLEND);

  if(modTimeLeft[MOD_EXTRA_LIFE]) {
	GLfloat specular[4]={0.0,0.0,0.0,0.0};
	GLfloat primary[4]; for(i=0;i<3;i++) primary[i]=primaryColor[i]; primary[3] = 0.4;
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,primary);
	glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
	glMaterialf(GL_FRONT,GL_SHININESS,0.0);		
	glPushMatrix(); // TODO: MB This looks unneccesary!
	gluSphere(qball, radius*1.25, resolution, resolution);
	glPopMatrix();
  }
  if(modTimeLeft[MOD_JUMP]) {
	glPushMatrix();
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glColor4f(1.0,1.0,1.0,0.5);
	
	glRotatef(Game::current->gameTime*50.0,0.0,0.0,1.0);
	glBegin(GL_TRIANGLES);
	glNormal3f(-1.0,0.0,0.0);
	double z = 1.1 + 1.0 * fmod(Game::current->gameTime,1.0);
	glVertex3f(0.0,radius*-.3,radius*z);
	glVertex3f(0.0,radius*.3,radius*(z+.9));
	glVertex3f(0.0,radius*.3,radius*z);
	
	glVertex3f(0.0,radius*.3,radius*(z+.9));
	glVertex3f(0.0,radius*-.3,radius*z);
	glVertex3f(0.0,radius*-.3,radius*(z+.9));
	
	glVertex3f(0.0,radius*-.6,radius*(z+.9));
	glVertex3f(0.0,0.0,radius*(z+2.1));
	glVertex3f(0.0,radius*.6,radius*(z+.9));
	
	glEnd();
	glPopAttrib();
	glPopMatrix();
  }
  if(modTimeLeft[MOD_DIZZY]) {
	glPushMatrix();
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	bindTexture("dizzy.png");
	glColor4f(1.0,1.0,1.0,0.5);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glScalef(radius,radius,radius);

	glRotatef(Game::current->gameTime*90.0,0.0,0.0,1.0);
	for(i=0;i<3;i++) {
	  glRotatef(360/3.0,0.0,0.0,1.0);
	  glNormal3f(-1.0,0.0,0.0);
	  glBegin(GL_QUADS);	
	  glTexCoord2f(dizzyTexMaxY, dizzyTexMaxX);
	  glVertex3f(-0.6,1.5,0.5-0.6);
	  glTexCoord2f(dizzyTexMinY, dizzyTexMaxX);
	  glVertex3f(+0.6,1.5,0.5-0.6);
	  glTexCoord2f(dizzyTexMinY, dizzyTexMinX);
	  glVertex3f(+0.6,1.5,0.5+0.6);
	  glTexCoord2f(dizzyTexMaxY, dizzyTexMinX);
	  glVertex3f(-0.6,1.5,0.5+0.6);
	  glEnd();
	}
	glPopAttrib();
	glPopMatrix();
  }
  glPopAttrib();
  glPopMatrix();
}

extern float fps;
void Ball::doExpensiveComputations() {
  /* TODO. Better test for when to render environment. Eg, only if ball is
     visible etc. */
  if(reflectivity > 0.0 && Settings::settings->doReflections) {
    double dx=position[0] - MainMode::mainMode->camFocus[0];
    double dy=position[1] - MainMode::mainMode->camFocus[1];
    double dist=sqrt(dx*dx+dy*dy);
    if(dist < 10) {
      int update=20; /* How often to update environment map (on average) */
      if(Settings::settings->gfx_details == 4) update = 10;
      if(Settings::settings->gfx_details == 5) update = 5;   
      if(((rand()>>7) % update) == 0 || 1) {
	if(environmentTexture == 0) glGenTextures(1, &environmentTexture);
	dontReflectSelf=1;
		MainMode::mainMode->renderEnvironmentTexture(environmentTexture,position);
		dontReflectSelf=0;
      }
    }
  }
}

void Ball::tick(Real time) {
  int i;
  double phase;

  for(i=0;i<NUM_MODS;i++) {
	if(!modTimeLeft[i]) modTimePhaseIn[i]=0.0;
	else modTimePhaseIn[i]+=time;

	if(modTimeLeft[i] > 0.0) { 
	  modTimeLeft[i] -= time;
	  if(modTimeLeft[i] < 0) modTimeLeft[i] = 0.0;
	}  
  }

  /* Nitro balls create "debris" clouds constantly */
  if(modTimeLeft[MOD_NITRO]) {
	nitroDebrisCount += time;
	while(nitroDebrisCount > 0.0) {
	  nitroDebrisCount -= 0.25;
	  Debris *d=new Debris(this,position,velocity,2.0+2.0*frandom());
	  d->position[0] += (frandom()-0.5)*radius;
	  d->position[1] += (frandom()-0.5)*radius;
	  d->position[2] += radius*1.0;
	  d->velocity[2] += 0.2;
	  d->gravity=-0.1;
	  d->modTimeLeft[MOD_GLASS]=-1.0;
	  d->primaryColor[0]=0.1;
	  d->primaryColor[1]=0.6;
	  d->primaryColor[2]=0.1;
	  d->no_physics=1;
	}
  }

  radius=realRadius;
  if(modTimeLeft[MOD_LARGE]) {
	phase=min(modTimePhaseIn[MOD_LARGE]/5.0,1.0);
    if(modTimeLeft[MOD_LARGE] > 0)
	  phase=min(modTimeLeft[MOD_LARGE]/3.0,phase);
	else
	  phase=1.0;
	radius *= 1.0 + phase;
  }
  if(modTimeLeft[MOD_SMALL]) {
	phase=min(modTimePhaseIn[MOD_SMALL]/5.0,1.0);
    if(modTimeLeft[MOD_SMALL] > 0)
	  phase=min(modTimeLeft[MOD_SMALL]/3.0,phase);
	else
	  phase=1.0;
	radius /= 1.0 + phase;
  }
  physics(time);
}

void Ball::setReflectivity(double reflectivity,int metallic) {
  this->reflectivity=reflectivity;
  this->metallic=metallic;
  if(reflectivity <= 0.0 && environmentTexture != 0) {
	/* Deallocate environment texture */
	glDeleteTextures(1,&environmentTexture);
	environmentTexture=0;	
  }  
}

Boolean Ball::physics(Real time) {
  Real x,y,h;

  if(!Game::current) return true;  
  Map *map = Game::current->map;

  if(modTimeLeft[MOD_DIZZY]) {
    /* Explanation. We cast the address of the ball into an integer in order to get
       a unique random seed for every ball. Could possibly cause problems on 64 bit platforms, have to
       look into it. */
    rotation[0] += time * 7.0 * (frand((long) Game::current->gameTime + (long) this)-0.5); 
    rotation[1] += time * 7.0 * (frand(47 + (long) Game::current->gameTime + (long) this)-0.5); 
  }

  /*  rotateX(-rotation[1]*time*2.0*M_PI*radius,rotations);
	  rotateY(-rotation[0]*time*2.0*M_PI*radius,rotations);*/
  rotateX(-rotation[1]*time*2.0*M_PI*0.3*0.3/radius,rotations);
  rotateY(-rotation[0]*time*2.0*M_PI*0.3*0.3/radius,rotations);


  int i;
  Real t=0;
  do {
	
    Real partialX = position[0] - (Real) ((int) position[0]);
    Real partialY = position[1] - (Real) ((int) position[1]);
    Cell& c = map->cell((int) position[0],(int) position[1]);
    Coord3d normal;
    c.getNormal(normal,Cell::CENTER);

	Real mapHeight=map->getHeight(position[0],position[1]);

    /* All effects of gravity */
	if(inTheAir)
	  velocity[2] = velocity[2] - gravity * physicsResolution;
    else if(!inPipe) {
	  double scale = gravity * physicsResolution / normal[2];
      velocity[0] += normal[0] * scale; //gravity * physicsResolution;
	  velocity[1] += normal[1] * scale; //gravity * physicsResolution;
	}

	/* Sand - generate debris */
	if(!inPipe && c.flags & CELL_SAND && !inTheAir && radius > 0.2) {
	  double speed = velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
	  if(frandom() < (speed - 0.3) * 0.08) {
		/* lots of friction when we crash into sand */
		velocity[0] *= 0.9;
		velocity[1] *= 0.9;
		velocity[2] *= 0.9;
		generateSandDebris();
	  }
	} else if(Settings::settings->difficulty > 0 && !inPipe && modTimeLeft[MOD_SPIKE] && !inTheAir && radius > 0.2) {
	  double speed = velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
	  if(frandom() < (speed - 0.3) * 0.0007*Settings::settings->difficulty) {
		velocity[0] *= 0.9;
		velocity[1] *= 0.9;
		velocity[2] *= 0.9;
		generateDebris(c.colors[Cell::CENTER]);
	  }
	}

	/* All effects of water */
	{
	  double waterHeight = map->getWaterHeight(position[0],position[1]);


	  /* Floating */
	  if(modTimeLeft[MOD_FLOAT] && !inPipe && waterHeight > position[2] && waterHeight > mapHeight + radius + 0.025) {
		velocity[2] += gravity * physicsResolution * 1.5;
		velocity[2] *= 0.99;
		// avoid sticking to the ground
		double delta=position[2] - radius - map->getHeight(position[0],position[1]);
		if(delta < 0.025) {
		  position[2] += 0.025 - delta;
		  if(velocity[2] < 0.0) velocity[2] = 0.0;
		}
	  }

	  if(position[2] - radius < waterHeight) {
		double depth=waterHeight - (position[2]-radius);
		// splashes caused by speed
		double speed = velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2] * 5.;
		if(frandom() < speed * 0.001 * (depth<0.5?depth:1.0-depth) * radius/0.3) {
		  GLfloat waterColor[4]={0.4,0.4,0.8,0.5};
		  Coord3d center; assign(position,center);
		  center[2] = map->getWaterHeight(center[0],center[1]);
		  Splash *s = new Splash(center,velocity,waterColor,30*radius/0.3,radius); //speed*radius*(depth<0.5?depth:1.0-depth)*2.0,radius);
		}
		// splashes caused by rotation. eg "swimming"
		speed = rotation[0]*rotation[0]+rotation[1]*rotation[1];
		if(frandom() < speed * 0.001) {
		  GLfloat waterColor[4]={0.4,0.4,0.8,0.5};
		  Coord3d center; assign(position,center);
		  center[2] = map->getWaterHeight(center[0],center[1]);
		  Coord3d vel;
		  vel[0]=-rotation[0] * radius; //0.3;
		  vel[1]=-rotation[1] * radius; //0.3;
		  vel[2]=0.2;
		  rotation[0] *= 0.9;
		  rotation[1] *= 0.9;
		  velocity[0] += 0.01 * rotation[0];
		  velocity[1] += 0.01 * rotation[1];
		  Splash *s = new Splash(center,vel,waterColor,(int) speed*0.5,radius);
		}
		double fric=0.004*min(1.0,depth / (2.*radius)); // an extra water friction
		velocity[0]=velocity[0]*(1.-fric)+c.velocity[0]*fric;
		velocity[1]=velocity[1]*(1.-fric)+c.velocity[1]*fric;		
		fric=0.008;
		velocity[2]=velocity[2]*(1.-fric)+c.velocity[1]*fric;
	  }
	}

	if(!inPipe && !inTheAir && c.flags & CELL_KILL) {
	  die(DIE_OTHER);
	  return false;
	}

	/* Sinking into floor material */
	if(c.flags & CELL_SAND && !inPipe) {
	  sink += 0.8 * physicsResolution;
	  if(sink > radius * 0.5) sink = radius * 0.5;
	} else	if(c.flags & CELL_ACID && !inPipe && !inTheAir) {
	  sink += 0.8 * physicsResolution;
	  double speed = velocity[0] * velocity[0] + velocity[1] * velocity[1] + velocity[2] * velocity[2];
	  if(frandom() < (speed - 0.2) * 0.05) {
		GLfloat acidColor[4]={0.1,0.5,0.1,0.5};
		Coord3d center; assign(position,center);
		center[2] = map->getHeight(center[0],center[1]);
		Splash *s = new Splash(center,velocity,acidColor,speed*radius,radius);
	  }
	  if(modTimeLeft[MOD_GLASS]) sink = min(sink,0.3);
	  if(sink > radius * 2.0) { die(DIE_ACID); return false; }
	} else
	  sink=max(0.0,sink-2.0*physicsResolution);
	
	/*                                      */
	/* Ground "grip" - Also works in pipes! */
	/*                                      */
	{ 
	  double v_fric=0.08;
	  double r_fric=0.10;

	  if(c.flags & CELL_ACID) {
	    v_fric = 0.008;
	    r_fric = 0.010;
	  }
	  
	  if(c.flags & CELL_ICE) {
	    if(modTimeLeft[MOD_SPIKE]) {
	      v_fric = 0.008; r_fric = 0.010;
	    } else {
	      v_fric = 0.0008; r_fric = 0.0010;
	    }
	  } 
	  

	  if(inTheAir) {
		if(map->getWaterHeight(position[0],position[1]) > position[2] - radius) {
		  v_fric = 0.0005; r_fric = 0.0005;
		} else
		  v_fric=r_fric=0.0;
	  }

	  if(inPipe) { v_fric = 0.08; r_fric = 0.10; }

	  if(c.flags & CELL_TRACK) {
		velocity[0] = velocity[0] * (1.0 - v_fric) + (rotation[0]+c.velocity[0]) * v_fric;
		velocity[1] = velocity[1] * (1.0 - v_fric) + (rotation[1]+c.velocity[1]) * v_fric;
		rotation[0] = rotation[0] * (1.0 - r_fric) + (velocity[0]-c.velocity[0]) * r_fric;
		rotation[1] = rotation[1] * (1.0 - r_fric) + (velocity[1]-c.velocity[1]) * r_fric;
	  } else {
		velocity[0] = velocity[0] * (1.0 - v_fric) + rotation[0] * v_fric;
		velocity[1] = velocity[1] * (1.0 - v_fric) + rotation[1] * v_fric;
		rotation[0] = rotation[0] * (1.0 - r_fric) + velocity[0] * r_fric;
		rotation[1] = rotation[1] * (1.0 - r_fric) + velocity[1] * r_fric;
	  }
	}

	/* rotational friction - limits the speed when on ice or in the air, water etc. */
	rotation[0] *= 0.9995; rotation[1] *= 0.9995;
	double effective_friction = 0.001 * friction;
	if(inTheAir && !(map->getWaterHeight(position[0],position[1]) > position[2] - radius)) effective_friction *= 0.1;
	else if(!inTheAir) {
	  if(inPipe) {}
	  else {		
		if(c.flags & CELL_ACID) effective_friction *= 2.0;
		if(c.flags & CELL_ICE) effective_friction *= 0.1;
		if(c.flags & CELL_SAND) effective_friction *= (c.flags&CELL_TRACK?20.0:10.0);
		else if(c.flags & CELL_TRACK) effective_friction *= 4.0; // Note. not both sand and track effect
	  }
	  if(modTimeLeft[MOD_SPIKE]) effective_friction *= 1.5;
	  if(modTimeLeft[MOD_SPEED]) effective_friction *= 0.5;
	}
	for(i=0;i<2;i++)
	  velocity[i] = velocity[i] - (velocity[i] - c.velocity[i]) * effective_friction;
	velocity[2] *= 1.0 - effective_friction;
	if(inTheAir && velocity[2] > 5.0) velocity[2] *= 0.995;

    for(i=0;i<3;i++)
      position[i] += physicsResolution * velocity[i];

	if(!inPipe) { 
	  handleEdges();

	  /* Ground collisions */
	  if(!checkGroundCollisions(map,0,0)) return false;
	  for(x=-radius+0.05;x<=radius-0.05;x+=0.05)
		if(!checkGroundCollisions(map,x,0.0)) return false;
	  for(y=-radius+0.05;y<=radius-0.05;y+=0.05)
		if(!checkGroundCollisions(map,0.0,y)) return false;
	}

	/* Collisions with other balls */
	handleBallCollisions();

	/* Collisions with forcefields */
	handleForcefieldCollisions();
	

	double dh = position[2]-radius - map->getHeight(position[0],position[1]);
    if(dh > 0.1)
      inTheAir = true;

    /* Pipes */
	handlePipes();

    t += physicsResolution;
  } while(t < time);
  return true;
}
Boolean Ball::checkGroundCollisions(Map *map,Real x,Real y) {
  Real dh = position[2] - sqrt(radius*radius-x*x-y*y) - map->getHeight(position[0]+x,position[1]+y);
  int i;

  if(dh < 0.02) {
    if(inTheAir) {
	  /* We where in the air and have now hit the ground. Calculate
		 a bounce */

	  /* first, change speed etc. if we are rotating */
	  double v_fric = 0.2;
	  double r_fric = 0.4;
	  velocity[0] = velocity[0] * (1.0 - v_fric) + rotation[0] * v_fric;
	  velocity[1] = velocity[1] * (1.0 - v_fric) + rotation[1] * v_fric;
	  rotation[0] = rotation[0] * (1.0 - r_fric) + velocity[0] * r_fric;
	  rotation[1] = rotation[1] * (1.0 - r_fric) + velocity[1] * r_fric;

      Coord3d normal;
      Cell& c = map->cell((int)(position[0]+x),(int)(position[1]+y));
      /* TODO: Use correct normal */
      c.getNormal(normal,Cell::CENTER);

      Real speed = -dotProduct(velocity,normal); // or simply -velocity[2];
      if(speed > 0) {
		Cell &cell = map->cell((int)(position[0]+x),(int)(position[1]+y));

		double crash_speed=speed;
		if(cell.flags&(CELL_TRAMPOLINE|CELL_SAND)) crash_speed *= 0.4;
		if(modTimeLeft[MOD_JUMP]) crash_speed *= 0.8;

		if(!crash(crash_speed)) return false;
		double effective_bounceFactor = bounceFactor;
		if(cell.flags & CELL_ACID) effective_bounceFactor = 0.0;
		if(cell.flags & CELL_SAND) effective_bounceFactor = 0.1;
		if(cell.flags & CELL_TRAMPOLINE) {
		  Real dh = 1.0 * speed * radius*radius*radius;		  
		  effective_bounceFactor += 0.6;
		  for(i=0;i<5;i++)
			cell.heights[i] -= dh;
		  if(cell.sunken <= 0.0)
			new Trampoline(cell);
		  cell.sunken += dh;
		}
		speed *= 1.0 + effective_bounceFactor;
		velocity[0] += normal[0] * speed;
		velocity[1] += normal[1] * speed; 
		velocity[2] += normal[2] * speed;
		
		if(cell.flags & CELL_ACID) {
		  GLfloat acidColor[4]={0.1,0.5,0.1,0.5};
		  Coord3d center; assign(position,center);
		  center[2]=map->getHeight(center[0],center[1]);
		  Splash *s = new Splash(center,velocity,acidColor,speed*radius*20.0,radius);
		} 


		if(cell.flags & CELL_SAND) {
		  /* lots of friction when we crash into sand */
		  velocity[0] *= 0.5;
		  velocity[1] *= 0.5;
		  velocity[2] *= 0.5;
		  if(radius > 0.2)
			for(i=0;i<10;i++)
			  if(frandom() < (speed-1.0)*0.2) generateSandDebris();
		  if(speed > 4.0)
			playEffect(SFX_SAND_CRASH);
		}
      }
      
      if(dh < 0.01) { /* ugly fix to stop being caught on edges */
		Cell &cell = map->cell((int)(position[0]+x),(int)(position[1]+y));
		velocity[0] -= 0.005 * x;
		velocity[1] -= 0.005 * y;
		if(cell.flags & CELL_SAND) {
		  velocity[0] -= 0.5 * x;
		  velocity[1] -= 0.5 * y;
		}
      }	  
    }


	if(velocity[2] > 2.0) {
	  position[2] = position[2] - dh + 0.02;
	} else {
	  position[2] -= dh;
	  velocity[2] -= dh;
	  inTheAir = false;
	}
  }
  return true;
}

Boolean Ball::crash(Real speed) {
  if(modTimeLeft[MOD_GLASS]) speed *= 1.5;

  if(speed > crashTolerance) { die(DIE_CRASH); return false; }

  // Make ball dizzy if crash is above 60% of what we can
  // tolerate. If we already are very dizzy (>4s or permanent)
  // keep it that way.
  if(speed > crashTolerance * (modTimeLeft[MOD_JUMP] ? 0.9 : 0.6))
	if(modTimeLeft[MOD_DIZZY] >= 0.0 &&
	   modTimeLeft[MOD_DIZZY] < 3.0+1.0*Settings::settings->difficulty) 
	  modTimeLeft[MOD_DIZZY] = 3.0+1.0*Settings::settings->difficulty;

  return true;
}
Boolean Ball::crash(Real speed,Ball *ball) {
  return this->crash(speed);
}
void Ball::generateSandDebris() {
  Coord3d pos,vel;
  Real a = frandom() * 2.0 * M_PI;
  double speed=sqrt(velocity[0]*velocity[0]+velocity[1]*velocity[1]+velocity[2]*velocity[2]);
  pos[0] = position[0] + radius*1.7*sin(a); pos[1] = position[1]+radius*1.7*cos(a); pos[2] = position[2] - radius*.5;
  vel[0] = velocity[0] + 2.0*speed*sin(a); // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[1] = velocity[1] + 2.0*speed*cos(a); // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[2] = velocity[2]; // + speed * 1/2048.0 * ((rand()%2048)-1024);
  Debris *d = new Debris(NULL,pos,vel,0.5+1.0*frandom());
  d->initialSize=0.05;
  d->primaryColor[0] = 0.6 + 0.3*frandom();
  d->primaryColor[1] = 0.5 + 0.4*frandom();
  d->primaryColor[2] = 0.1 + 0.3*frandom();
  d->friction = 0.0;
  d->calcRadius();
}
void Ball::generateDebris(GLfloat color[4]) {
  Coord3d pos,vel;
  Real a = frandom() * 2.0 * M_PI;
  double speed=sqrt(velocity[0]*velocity[0]+velocity[1]*velocity[1]+velocity[2]*velocity[2]);
  pos[0] = position[0] + radius*1.5*sin(a); pos[1] = position[1]+radius*1.5*cos(a); pos[2] = position[2] - radius*.5;
  vel[0] = velocity[0] + 2.0*speed*sin(a); // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[1] = velocity[1] + 2.0*speed*cos(a); // + speed * 1/2048.0 * ((rand()%2048)-1024);
  vel[2] = velocity[2]; // + speed * 1/2048.0 * ((rand()%2048)-1024);
  Debris *d = new Debris(NULL,pos,vel,0.5+1.0*frandom());
  d->initialSize=0.05;
  d->primaryColor[0] = color[0];
  d->primaryColor[1] = color[1];
  d->primaryColor[2] = color[2];
  d->friction = 0.0;
  d->calcRadius();
}

void Ball::handleBallCollisions() {
  int i;

  set<Ball*>::iterator iter = balls->begin();
  set<Ball*>::iterator end = balls->end();
  Coord3d v;
  double dist,err,speed;
  Ball *ball;
  for(;iter!=end;iter++) {
	ball = *iter;
	if(ball == this) continue;
	if(!ball->alive) continue;
	if(ball->no_physics) continue;
	sub(ball->position,position,v);
	dist = length(v);
	if(dist < radius + ball->radius - 1e-3) {
	  err = radius + ball->radius - dist;
	  position[0] -= err * v[0];
	  position[1] -= err * v[1];
	  position[2] -= err * v[2];
	  normalize(v);
	  speed = dotProduct(v,velocity) - dotProduct(v,ball->velocity);
	  if(speed < 1e-3) continue;
	  double myWeight = radius*radius*radius, hisWeight=ball->radius*ball->radius*ball->radius, totWeight = myWeight + hisWeight;
	  myWeight /= totWeight; hisWeight /= totWeight;
	  this->crash(speed*hisWeight*1.5*(ball->modTimeLeft[MOD_SPIKE]?6.0:1.0),ball);
	  ball->crash(speed*myWeight*1.5*(this->modTimeLeft[MOD_SPIKE]?6.0:1.0),this);
	  for(i=0;i<3;i++) {
		velocity[i] -= speed * v[i] * 3.0 * hisWeight;
		ball->velocity[i] += speed * v[i] * 3.0 * myWeight;
	  }
	}
  }	  
}
void Ball::handleForcefieldCollisions() {
  int i;

  set<ForceField*>::iterator iter = ForceField::forcefields->begin();
  set<ForceField*>::iterator end = ForceField::forcefields->end();
  for(;iter!=end;iter++) {
	ForceField *ff = *iter;
	if(!ff->is_on) continue;

	/* Boundingbox test, continue if we cannot possibly be withing FF */
	if(ff->direction[0] >= 0.0) {
	  if(this->position[0] + radius < ff->position[0]) continue;
	  if(this->position[0] - radius > ff->position[0]+ff->direction[0]) continue;
	} else {
	  if(this->position[0] + radius < ff->position[0]+ff->direction[0]) continue;
	  if(this->position[0] - radius > ff->position[0]) continue;
	}
	if(ff->direction[1] >= 0.0) {
	  if(this->position[1] + radius < ff->position[1]) continue;
	  if(this->position[1] - radius > ff->position[1]+ff->direction[1]) continue;
	} else {
	  if(this->position[1] + radius < ff->position[1]+ff->direction[1]) continue;
	  if(this->position[1] - radius > ff->position[1]) continue;
	}
	
	/*                                */
	/* Detailed collision computation */
	/*                                */
	Coord3d v,ff_normal;
	sub(this->position,ff->position,v);		
	v[2] = 0.0;
	ff_normal[0] = ff->direction[1]; ff_normal[1] = ff->direction[0]; ff_normal[2] = 0.0; normalize(ff_normal);
	double xy_dist = dotProduct(v,ff_normal);
	//if(xy_dist > this->radius) continue;
	assign(ff->direction,ff_normal); normalize(ff_normal); 
	double ff_where = dotProduct(v,ff_normal); // how long along ff->direction the hit is
	double ff_len = length(ff->direction); // the xy-length of the forcefield
	if(ff_where < 0) { xy_dist = sqrt(xy_dist*xy_dist + ff_where*ff_where); ff_where = 0.0; }
	else if(ff_where > ff_len) {
	  double tmp = ff_where - ff_len;
	  xy_dist = sqrt(xy_dist*xy_dist + tmp*tmp);
	  ff_where = ff_len;
	}
	double ff_base = ff->position[2] + ff_where / ff_len * ff->direction[2]; // lower z of forcefield at point of impact	
	double ff_where_h = position[2]; // z position where we hit the forcefield
	double z_dist=0.0;
	if(position[2] < ff_base) { z_dist = ff_base - position[2]; ff_where_h = ff_base; }
	else if(position[2] > ff_base + ff->height) {
	  z_dist = position[2] - ff_base - ff->height;
	  ff_where_h = ff_base + ff->height; 
	}
	double dist = sqrt(xy_dist*xy_dist+z_dist*z_dist);
	if(dist < radius) {
	  // normal of forcefield
	  ff_normal[0] = ff->direction[1]; ff_normal[1] = ff->direction[0]; ff_normal[2] = 0.0; normalize(ff_normal);
	  
	  // the direction we are hitting it from
	  v[0] = ff->position[0] + ff_where/ff_len * ff->direction[0] - position[0];
	  v[1] = ff->position[1] + ff_where/ff_len * ff->direction[1] - position[1];
	  v[2] = ff_where_h - position[2];
	  normalize(v);
	  
	  // sign for direction
	  double sign = dotProduct(ff_normal,v);
	  if(sign < 0) sign = -1.0; else sign = 1.0;
	  
	  double bounce = ff->bounceFactor * dotProduct(velocity,v);
	  if(bounce < 0.0) continue; // we are already heading *away* from the forcefield
	  
	  if(sign == 1.0) {
		if(ff->allow & FF_KILL1) { die(DIE_FF); return; }
		else if(!(ff->allow & FF_BOUNCE1)) continue;
	  } else if(sign == -1.0) {
		if(ff->allow & FF_KILL2) { die(DIE_FF); return; }
		else if(!(ff->allow & FF_BOUNCE2)) continue;			
	  }
	  
	  if(ff_where <= 0.0 || ff_where >= ff_len) {
		// Hitting a forcefield side edge
		velocity[0] -= bounce * v[0];
		velocity[1] -= bounce * v[1];
	  } else if(v[2] < 0.0) {
		// Hitting forcefield from above
		if(velocity[2] < 0.0)
		  velocity[2] -= velocity[2] * ff->bounceFactor;
	  } else if(v[2] > 0.0) {
		// Hitting forcefield from below (!)
		if(velocity[2] > 0.0)
		  velocity[2] -= velocity[2] * ff->bounceFactor;
	  } else {
		// Hitting forcefield from the side
		velocity[0] -= sign * bounce * ff_normal[0];
		velocity[1] -= sign * bounce * ff_normal[1];
	  }
	}
  }
}
void Ball::handleEdges() {
  Map *map = Game::current->map;

  if(map->getHeight(position[0]+radius,position[1]) >= position[2]-radius*0.0) {
	if(velocity[0] > 0) { crash(velocity[0]*0.0); velocity[0] *= -bounceFactor; }
	velocity[0] -= 0.1;	//position[0] -= 0.1;
  }
  if(map->getHeight(position[0]-radius,position[1]) >= position[2]-radius*0.0) {
	if(velocity[0] < 0) { crash(-velocity[0]*0.0); velocity[0] *= -bounceFactor; }
	velocity[0] += 0.1;	//position[0] += 0.1;
  }
  if(map->getHeight(position[0],position[1]+radius) >= position[2]-radius*0.0) {
	if(velocity[1] > 0) { crash(velocity[1]*0.0); velocity[1] *= -bounceFactor; }
	velocity[1] -= 0.1;	//position[1] -= 0.1;      
  }
  if(map->getHeight(position[0],position[1]-radius) >= position[2]-radius*0.0) {
	if(velocity[1] < 0) { crash(-velocity[1]*0.0); velocity[1] *= -bounceFactor; }
	velocity[1] += 0.1;	//position[1] += 0.1;      
  }
  
  const double sqrt2=0.7071067;
  if(map->getHeight(position[0]+radius*sqrt2,position[1]+radius*sqrt2)  >= position[2]-radius*0.0) {
	if(velocity[0] > 0) { crash(velocity[0]*0.0); velocity[0] *= -bounceFactor; }
	velocity[0] -= 0.1; //position[0] -= 0.1;
	if(velocity[1] > 0) { crash(velocity[1]*0.0); velocity[1] *= -bounceFactor; }
	velocity[1] -= 0.1; //position[1] -= 0.1;
  }
  if(map->getHeight(position[0]-radius*sqrt2,position[1]+radius*sqrt2)  >= position[2]-radius*0.0) {
	if(velocity[0] < 0) { crash(-velocity[0]*0.0); velocity[0] *= -bounceFactor; }
	velocity[0] += 0.1; //position[0] += 0.1;
	if(velocity[1] > 0) { crash(velocity[1]*0.0); velocity[1] *= -bounceFactor; }
	velocity[1] -= 0.1; //position[1] -= 0.1;
  }
  if(map->getHeight(position[0]+radius*sqrt2,position[1]-radius*sqrt2)  >= position[2]-radius*0.0) {
	if(velocity[0] > 0) { crash(velocity[0]*0.0); velocity[0] *= -bounceFactor; }
	velocity[0] -= 0.1; //position[0] -= 0.1;
	if(velocity[1] < 0) { crash(-velocity[1]*0.0);  velocity[1] *= -bounceFactor; }
	velocity[1] += 0.1; //position[1] += 0.1;
  }
  if(map->getHeight(position[0]-radius*sqrt2,position[1]-radius*sqrt2)  >= position[2]-radius*0.0) {
	if(velocity[0] < 0) { crash(-velocity[0]*0.0); velocity[0] *= -bounceFactor; }
	velocity[0] += 0.1; //position[0] += 0.1;
	if(velocity[1] < 0) { crash(-velocity[1]*0.0); velocity[1] *= -bounceFactor; }
	velocity[1] += 0.1; //position[1] += 0.1;
  }
}
void Ball::handlePipes() {
  int i;
  inPipe=false;

  set<Pipe*>::iterator iter = Pipe::pipes->begin();
  set<Pipe*>::iterator end = Pipe::pipes->end();
  for(;iter!=end;iter++) {
	Pipe *pipe = *iter;
	Coord3d direction;  // direction of pipe
	Coord3d dirNorm;    // normalized direction
	sub(pipe->to,pipe->from,direction);
	double pipeLength = length(direction);
	sub(pipe->to,pipe->from,dirNorm);
	normalize(dirNorm);
	Coord3d v0;         // pipe enterance -> ball position
	sub(position,pipe->from,v0); 
	double l = max(0.0,min(1.0,dotProduct(v0,dirNorm) / length(direction))); // where along pipe ball is projected
	Coord3d proj;     // where (as pos) the ball is projected
	for(i=0;i<3;i++) proj[i] = pipe->from[i] + l*direction[i];
	Coord3d v1;         // projection point -> ball position
	sub(position,proj,v1);
	double distance = length(v1); // how far away from the pipe the ball is
	if(distance > pipe->radius || l == 0.0 || l == 1.0) { 
	  /* Ball is on the outside */
	  // note. If <inPipe> then we are already inside another pipe. No collision!
	  if(pipe->radius > 0.1 && pipe->radius < radius && distance < radius) {
		/* Added code to bounce of too small pipes, except for *realy* thin pipes which
		 are meant to be lifts */
		Coord3d normal; assign(v1,normal); normalize(normal);
		double speed=-dotProduct(velocity,normal);
		if(speed > 0) 
		  for(i=0;i<3;i++) velocity[i] += speed * normal[i] * 1.5;
		double correction = radius - distance;
		for(i=0;i<3;i++)
		  for(i=0;i<3;i++) position[i] += correction * normal[i];
	  } else  if(distance < pipe->radius + radius && l != 0.0 && l != 1.0 && !inPipe) {
		/* Collision from outside */
		if(pipe->flags & PIPE_SOFT_ENTER & l < 0.2 / pipeLength) continue;
		if(pipe->flags & PIPE_SOFT_EXIT & l > 1.0 - 0.2 / pipeLength) continue;

		Coord3d normal; assign(v1,normal); normalize(normal);
		double speed=-dotProduct(velocity,normal);
		if(speed > 0) 
		  for(i=0;i<3;i++) velocity[i] += speed * normal[i] * 1.5;
		double correction = pipe->radius + radius - distance;
		for(i=0;i<3;i++)
		  for(i=0;i<3;i++) position[i] += correction * normal[i];
	  }
	} else {
	  /* Ball is on the inside */
	  inPipe=true;
	  if(distance > pipe->radius*0.97 - radius && l != 0.0 && l != 1.0) {
		/* Collision from inside */
		Coord3d normal; assign(v1,normal); normalize(normal);
		double speed=dotProduct(velocity,normal);
		if(speed > 0) 
		  for(i=0;i<3;i++) velocity[i] -= speed * normal[i] * 1.5;
		double correction = distance - (pipe->radius*0.97 - radius);
		for(i=0;i<3;i++)
		  for(i=0;i<3;i++) position[i] -= correction * normal[i];
	  }
	  
	  double zHere = (1.0 - l) * pipe->from[2] + l * pipe->to[2];
	  if(distance > pipe->radius*0.94 - radius && position[2] < zHere) {
		/* Ball is touching lower part of pipe wall */
		inTheAir=false;
		Coord3d normal; assign(v1,normal); normalize(normal);
		double scale = gravity * physicsResolution / (-normal[2]+1e-3);
		velocity[0] -= normal[0] * scale;
		velocity[1] -= normal[1] * scale;
		//velocity[2] += normal[2] * gravity * physicsResolution;
	  }
	  
	  /* Wind */
	  if(dotProduct(velocity,dirNorm) > 0.0)
		for(i=0;i<3;i++)
		  velocity[i] += pipe->windForward * physicsResolution * dirNorm[i];
	  else
		for(i=0;i<3;i++)
		  velocity[i] += pipe->windBackward * physicsResolution * dirNorm[i];

	}
  }

  /* Check for pipeConnectors to support ball, only if we are not inside a pipe already */
  if(!inPipe) {
	set<PipeConnector*>::iterator iter2 = PipeConnector::connectors->begin();
	set<PipeConnector*>::iterator end2 = PipeConnector::connectors->end();
	for(;iter2!=end2;iter2++) {
	  PipeConnector *connector = *iter2;
	  Coord3d v0; // ball -> connector
	  sub(connector->position,position,v0);
	  double dist = length(v0); // Distance ball center, connector center
	  if(dist > connector->radius) {
		/* Ball is outside connector */
		if(dist < connector->radius + radius) {
		  /* Collision from outside */
		  normalize(v0);
		  double speed=dotProduct(velocity,v0);
		  if(speed > 0)
			for(i=0;i<3;i++) velocity[i] -= speed * v0[i] * 1.5;
		  double correction = connector->radius + radius - dist;
		  for(i=0;i<3;i++)
			for(i=0;i<3;i++) position[i] -= correction * v0[i];
		}
	  } else {
		/* Ball is inside connector */
		inPipe=true;

		if(dist > connector->radius*0.97 - radius) {
		  /* Collision from inside */
		  normalize(v0);
		  double speed=dotProduct(velocity,v0);
		  if(speed > 0) 
			for(i=0;i<3;i++) velocity[i] += speed * v0[i] * 1.5;
		  double correction = dist - (connector->radius*0.97 - radius);
		  for(i=0;i<3;i++)
			for(i=0;i<3;i++) position[i] += correction * v0[i];
		}
		if(dist > connector->radius*0.94 - radius && position[2] < connector->position[2]) {
		  /* Ball is touching lower part of connector */
		  inTheAir=false;
		  normalize(v0);
		  double scale = 0.5*gravity * physicsResolution / (-v0[2]+1e-3);
		  velocity[0] -= v0[0] * scale;
		  velocity[1] -= v0[1] * scale;
		}
	  }
	}
  }
}
