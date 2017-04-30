/* template.cc
   Use to create new .cc project files

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
#include "glHelp.h"
#include "animated.h"
#include "sign.h"
#include "game.h"

using namespace std;

Sign::Sign(char *string,Real l,Real s,Real r,Coord3d pos) {
  assign(pos,position);
  if(l <= 0.0) l = 1e10;
  life=l;
  scale=s;
  rotation=r;
  tot_rot = 0.0;
  if(Game::current->isNight) {
	primaryColor[0] = 1.0;
	primaryColor[1] = 1.0;
	primaryColor[2] = 1.0;
  } else if(Game::current->fogThickness) {
	primaryColor[0] = 0.0;
	primaryColor[1] = 0.0;
	primaryColor[2] = 0.0;
  } else {
	primaryColor[0] = 1.0;
	primaryColor[1] = 1.0;
	primaryColor[2] = 1.0;
  }

  texture=0;
  mkTexture(string);
}

void Sign::mkTexture(char *string) {
  SDL_Surface *text;
  SDL_Color fgColor={255,255,255};
  GLfloat texcoord[4];

  if(texture) glDeleteTextures(1,&texture);
  text = TTF_RenderText_Blended(ingameFont, string, fgColor);
  texture = LoadTexture(text, texcoord);
  width=text->w;
  SDL_FreeSurface(text);

  texMinX = texcoord[0];
  texMinY = texcoord[1];
  texMaxX = texcoord[2];
  texMaxY = texcoord[3];  

  /* Note the slight over approximation of the boundingbox, needed since the sign
	 is rotating. Exact expression would be 0.005*scale*width*sqrt(2) */
  boundingBox[0][0]=-0.008*scale*width;
  boundingBox[1][0]=+0.008*scale*width;
  boundingBox[0][1]=-0.008*scale*width;
  boundingBox[1][1]=+0.008*scale*width;
  boundingBox[0][2]=-0.25*scale;
  boundingBox[1][2]=0.25*scale;
}

void Sign::draw() {
}
void Sign::draw2() {
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(position[0],position[1],position[2]);
  glRotatef(tot_rot,0.0,0.0,1.0);

  glColor4f(primaryColor[0],primaryColor[1],primaryColor[2],life>1.0?1.0:life);
  glBindTexture(GL_TEXTURE_2D, texture);
  glBegin(GL_TRIANGLE_STRIP);
  int swap = tot_rot > 70. && tot_rot < 70. + 180.;
  glTexCoord2f(swap?texMinX:texMaxX, texMinY); glVertex3f(+0.005*scale*width,-0.005*scale*width,0.25*scale);
  glTexCoord2f(swap?texMaxX:texMinX, texMinY); glVertex3f(-0.005*scale*width,+0.005*scale*width,0.25*scale);
  glTexCoord2f(swap?texMinX:texMaxX, texMaxY); glVertex3f(+0.005*scale*width,-0.005*scale*width,-0.25*scale);
  glTexCoord2f(swap?texMaxX:texMinX, texMaxY); glVertex3f(-0.005*scale*width,+0.005*scale*width,-0.25*scale);
  glEnd();

  glPopMatrix();
  glPopAttrib();
}

void Sign::tick(Real t) {
  life -= t;
  if(life <= 0) {
	/* time to die... */
	glDeleteTextures(1,&texture);
	remove();
	//delete this;
  }
  tot_rot += t * rotation;
  while(tot_rot > 360.0) tot_rot -= 360.0;
}
