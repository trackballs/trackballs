/** \file map.cc
   Handles all information about the current map, including the individual cells.
*/
/*
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
#include "map.h"
#include "SDL/SDL_endian.h"
#include "SDL/SDL_image.h"

#include "settings.h"
#include "game.h"
#include "gameMode.h"
#include "editMode.h"

using namespace std;

#define WRITE_PORTABLE 1
#define READ_PORTABLE 1


const int Map::flagNone=0,Map::flagFlashCenter=1,Map::flagTranslucent=2,Map::flagShowCross=4;
const int Map::mapFormatVersion=7;

inline int32_t saveInt(int32_t v) { return (int32_t) SDL_SwapBE32((uint32_t)v); }
inline int32_t loadInt(int32_t v) { return (int32_t) SDL_SwapBE32((uint32_t)v); }

/* initialization of some fields to be sure... */
Cell::Cell()  {
  texture = -1;
  flags = 0;
  for(int i=0;i<5;i++)
	waterHeights[i]=-20.0;  
}

/* Returns the normal for a point on the edge of the cell */
void Cell::getNormal(Coord3d normal,int vertex) {
  Coord3d v1;
  Coord3d v2;
  
  switch(vertex) {
  case SOUTH+WEST:
	v1[0] = 1.0; v1[1] = 0.0; v1[2] = heights[SOUTH+EAST]-heights[SOUTH+WEST];
	v2[0] = 0.5; v2[1] = 0.5; v2[2] = heights[CENTER]-heights[SOUTH+WEST];
    break;
  case SOUTH+EAST:
	v1[0] = 0.0; v1[1] = 1.0; v1[2] = heights[NORTH+EAST]-heights[SOUTH+EAST];
    v2[0] = -.5; v2[1] = 0.5; v2[2] = heights[CENTER]-heights[SOUTH+EAST];
    break;
  case NORTH+EAST:
	v1[0] = -1.0; v1[1] = 0.0; v1[2] = heights[NORTH+WEST]-heights[NORTH+EAST];
	v2[0] = -0.5; v2[1] = -0.5; v2[2] = heights[CENTER]-heights[NORTH+EAST];
    break;
  case NORTH+WEST:
	v1[0] = 0.0; v1[1] = -1.0; v1[2] = heights[SOUTH+WEST]-heights[NORTH+WEST];
	v2[0] = 0.5; v2[1] = -0.5; v2[2] = heights[CENTER]-heights[NORTH+WEST];
    break;
  case CENTER:
    getNormal(normal,SOUTH+WEST);
    getNormal(v1,SOUTH+EAST);
    add(v1,normal,normal);
    getNormal(v1,NORTH+WEST);
    add(v1,normal,normal);
    getNormal(v1,NORTH+EAST);
    add(v1,normal,normal);
    normalize(normal);
    return;
  }
  crossProduct(v1,v2,normal);
  normalize(normal);
}
/* Works on water heights */
void Cell::getWaterNormal(Coord3d normal,int vertex) {
  Coord3d v1;
  Coord3d v2;
  
  switch(vertex) {
  case SOUTH+WEST:
	v1[0] = 1.0; v1[1] = 0.0; v1[2] = waterHeights[SOUTH+EAST]-waterHeights[SOUTH+WEST];
	v2[0] = 0.5; v2[1] = 0.5; v2[2] = waterHeights[CENTER]-waterHeights[SOUTH+WEST];
    break;
  case SOUTH+EAST:
	v1[0] = 0.0; v1[1] = 1.0; v1[2] = waterHeights[NORTH+EAST]-waterHeights[SOUTH+EAST];
    v2[0] = -.5; v2[1] = 0.5; v2[2] = waterHeights[CENTER]-waterHeights[SOUTH+EAST];
    break;
  case NORTH+EAST:
	v1[0] = -1.0; v1[1] = 0.0; v1[2] = waterHeights[NORTH+WEST]-waterHeights[NORTH+EAST];
	v2[0] = -0.5; v2[1] = -0.5; v2[2] = waterHeights[CENTER]-waterHeights[NORTH+EAST];
    break;
  case NORTH+WEST:
	v1[0] = 0.0; v1[1] = -1.0; v1[2] = waterHeights[SOUTH+WEST]-waterHeights[NORTH+WEST];
	v2[0] = 0.5; v2[1] = -0.5; v2[2] = waterHeights[CENTER]-waterHeights[NORTH+WEST];
    break;
  case CENTER:
    getNormal(normal,SOUTH+WEST);
    getNormal(v1,SOUTH+EAST);
    add(v1,normal,normal);
    getNormal(v1,NORTH+WEST);
    add(v1,normal,normal);
    getNormal(v1,NORTH+EAST);
    add(v1,normal,normal);
    normalize(normal);
    return;
  }
  crossProduct(v1,v2,normal);
  normalize(normal);
}



/* Gives the height of the cell in a specified (floatingpoint) position */
Real Cell::getHeight(Real x,Real y) {
  Real h1,h2,h3,c;

  c = heights[CENTER];
  if(y<=x)
    if(y<=1-x) { /* SOUTH */
      h1 = heights[SOUTH+WEST];
      h2 = heights[SOUTH+EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * x + (c-h3) * 2 * y;
    } else { /* EAST */
      h1 = heights[SOUTH+EAST];
      h2 = heights[NORTH+EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * y + (c-h3) * 2 * (1-x);
    }
  else
    if(y<=1-x) { /* WEST */
      h1 = heights[NORTH+WEST];
      h2 = heights[SOUTH+WEST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * (1-y) + (c-h3) * 2 * x;
    } else { /* NORTH */
      h1 = heights[NORTH+EAST];
      h2 = heights[NORTH+WEST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * (1-x) + (c-h3) * 2 * (1-y);
    }
}

/* Works for water */
Real Cell::getWaterHeight(Real x,Real y) {
  Real h1,h2,h3,c;

  c = waterHeights[CENTER];
  if(y<=x)
    if(y<=1-x) { /* SOUTH */
      h1 = waterHeights[SOUTH+WEST];
      h2 = waterHeights[SOUTH+EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * x + (c-h3) * 2 * y;
    } else { /* EAST */
      h1 = waterHeights[SOUTH+EAST];
      h2 = waterHeights[NORTH+EAST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * y + (c-h3) * 2 * (1-x);
    }
  else
    if(y<=1-x) { /* WEST */
      h1 = waterHeights[NORTH+WEST];
      h2 = waterHeights[SOUTH+WEST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * (1-y) + (c-h3) * 2 * x;
    } else { /* NORTH */
      h1 = waterHeights[NORTH+EAST];
      h2 = waterHeights[NORTH+WEST];
      h3 = (h1 + h2) / 2;
      return h1 + (h2-h1) * (1-x) + (c-h3) * 2 * (1-y);
    }
}


Map::Map(char *filename) {
  gzFile gp;
  int x,y,i,north,east;
  char str[256];
  GLfloat texCoord[4];

  isBonus=0;
  isTransparent=0;

  cachedCX=cachedCY=-1,cacheCount=0;

  /* Data structures for maintaining displaylists.
	 We allocate 50 * 50 * 2 display lists for 50 x 50 x/y coordinates and 2 drawing stages 
  */
  nListsWide=50;
  nLists = nListsWide*nListsWide*2;
  displayLists = (int) glGenLists(nLists);

  snprintf(mapname,sizeof(mapname),_("Unknown track"));
  snprintf(author,sizeof(author),_("Unknown author"));
  gp=gzopen(filename,"rb");
  if(gp) {
	int version;
	int32_t data[6];
	gzread(gp,data,sizeof(int32_t)*6);
	for(i=0;i<3;i++)
	  startPosition[i] = 0.5 + loadInt(data[i]);
	width = loadInt(data[3]);
	height = loadInt(data[4]);
	version = loadInt(data[5]);

	if(version < mapFormatVersion) 
	  printf("Warning. Map %s is of an old format (v%d, latest is v%d)\n",filename,version,mapFormatVersion);
	else if(version > mapFormatVersion) {
	  fprintf(stderr,"Error. Map %s is from the future (v%d, I know only format v%d)",filename,version,mapFormatVersion);
	  fprintf(stderr,"This error usually occurs because or broken maps or big/small endian issues\n");
	  exit(0);
	}

	if(version >= 7) { /* Read texture indices */
	  gzread(gp,data,sizeof(int32_t)*1);
	  int nt=loadInt(data[0]); // num textures used in map
	  char textureName[64];
	  for(i=0;i<nt;i++) {
		gzread(gp,textureName,64);
		indexTranslation[i]=loadTexture(textureName);
	  }
	} else // for old maps we just assume that all loaded textures are in the same order as from creator
	  for(i=0;i<256;i++) indexTranslation[i]=i;

    cells = new Cell[width*height];

	for(y=0;y<height;y++)
	  for(x=0;x<width;x++) {
		// We have to do this here since Cell::load does not know it's own coordinates
		Cell& c=cell(x,y);
		for(north=0;north<2;north++)
		  for(east=0;east<2;east++) {
			c.textureCoords[north*Cell::NORTH+east*Cell::EAST][0] = (x+east)/4.0;
			c.textureCoords[north*Cell::NORTH+east*Cell::EAST][1] = (y+north)/4.0;
		  }
		
		c.load(this,gp,version);		
	  }
    gzclose(gp);
  } else {
	printf("Warning: could not open %s\n",filename);
    width = height = 256;
    cells = new Cell[width*height];
	startPosition[0] = startPosition[1] = 252;
	startPosition[2] = 0.0;	
    for(x=0;x<width;x++)
      for(y=0;y<height;y++) {
		Cell& c = cells[x+y*width];

		c.flags = 0;
		for(i=0;i<5;i++) {
		  c.heights[i] = -8.0;
		  c.waterHeights[i]=-8.5; // this is the groundwater =)
		  c.colors[i][0] = c.colors[i][1] = c.colors[i][2] = 0.9; c.colors[i][3] = 1.0;
		}
		for(i=0;i<4;i++) {
		  c.wallColors[i][0] = 0.7;
		  c.wallColors[i][1] = 0.2;
		  c.wallColors[i][2] = 0.2;
		  c.wallColors[i][3] = 1.0;
		}
		c.velocity[0] = 0.0;
		c.velocity[1] = 0.0;
		for(north=0;north<2;north++)
		  for(east=0;east<2;east++) {
			c.textureCoords[north*Cell::NORTH+east*Cell::EAST][0] = (x+east)/4.0;
			c.textureCoords[north*Cell::NORTH+east*Cell::EAST][1] = (y+north)/4.0;
		  }
      }
  }

  /* Fix display lists used by each cell */
  for(y=0;y<height;y++)
	for(x=0;x<width;x++) {
	  Cell& c = cells[x+y*width];	  
	  c.displayList = displayLists + ((x % nListsWide) + (y % nListsWide) * nListsWide)*2;
	  c.displayListLastCompiled=-1;
	  c.displayListDirty=1;
	}

  flags = flagNone;      
  startPosition[2]=getHeight(startPosition[0],startPosition[1]);

  tx_Ice=loadTexture("ice.png");
  tx_Acid=loadTexture("acid.png");
  tx_Sand=loadTexture("sand.png");
  tx_Track=loadTexture("track.png");

}
Map::~Map() {
  //  glDeleteLists(cache[0].lists[0],(width/4)*(height/4));
  glDeleteLists(displayLists,nLists);
  delete cells;
}

/* Draws the map on the screen from current viewpoint */
void Map::draw(int birdsEye,int stage,int cx,int cy) {
  int x,y,i,ix,iy;
  int r0,r1,c0,c1;
  int gfx_details = Settings::settings->gfx_details;

  if(stage > 0 && !isTransparent) return;
  static int frameCount=0;
  frameCount++;

  glPushAttrib(GL_ENABLE_BIT);

  r0 = 8+2*gfx_details; r1 = gfx_details>=4?-6:-5;
  c0 = -8; c1 = 8; 
  if(birdsEye) { c0=-10; c1=10; r0=10; r1=-10; }

  GLfloat specular[4] = {0.0, 0.0, 0.0, 0.0};
  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMaterialf(GL_FRONT,GL_SHININESS,0.0);
  glDisable(GL_TEXTURE_2D);

  int row,column;

  if(stage > 0) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  } else
    glDisable(GL_BLEND);

  GLint viewport[4];
  GLdouble model_matrix[16], proj_matrix[16], rot_matrix[16];  


#define MARGIN 50

  if(cx != cachedCX || cy != cachedCY || cacheCount > 10) {
	double t0 = getSystemTime();

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
    glGetDoublev(GL_PROJECTION_MATRIX, proj_matrix);
    
    Coord3d worldCoord,screenCoord;

	int ix,iy;
	int visibleCnt=0;

	for(ix=0;ix<41;ix++)
	  for(iy=0;iy<41;iy++) {
		x=cx+ix-20;
		y=cy+iy-20;
		cacheVisible[ix][iy]=0;
		worldCoord[0]=x+.5; worldCoord[1]=y+.5; worldCoord[2]=cell(x,y).heights[Cell::CENTER];
		gluProject(worldCoord[0], worldCoord[1], worldCoord[2],
				   model_matrix,proj_matrix,viewport,
				   &screenCoord[0], &screenCoord[1], &screenCoord[2]);
		if(screenCoord[0] >= - MARGIN && screenCoord[0] <= screenWidth+MARGIN &&
		   screenCoord[1] >= - MARGIN && screenCoord[1] <= screenHeight+MARGIN) { cacheVisible[ix][iy]=1; visibleCnt++; continue; }
		for(int north=0;north<2;north++)
		  for(int east=0;east<2;east++) {
			worldCoord[0]=x+1.0*east; worldCoord[1]=y+1.0*north; worldCoord[2]=cell(x,y).heights[north*Cell::NORTH+east*Cell::EAST];
			gluProject(worldCoord[0], worldCoord[1], worldCoord[2],
					   model_matrix,proj_matrix,viewport,
					   &screenCoord[0], &screenCoord[1], &screenCoord[2]);
			if(screenCoord[0] >= - MARGIN && screenCoord[0] <= screenWidth+MARGIN &&
			   screenCoord[1] >= - MARGIN && screenCoord[1] <= screenHeight+MARGIN) {
			  cacheVisible[ix][iy]=1;
			  visibleCnt++;
			  goto cont; // The almighty goto statment, don't do this at home kids!
			}
		  }			
	  cont:	x=x;
	  }
	// OPT. use only the +1,+1 versions if birdsEye == true
	for(ix=0;ix<41;ix++)
	  for(iy=0;iy<41;iy++)
		cacheVisible[ix][iy] |= (iy>0?cacheVisible[ix][iy-1]:0)|(iy<40?cacheVisible[ix][iy+1]:0)|
		  (ix>0?cacheVisible[ix-1][iy]:0)|(ix<40?cacheVisible[ix+1][iy]:0);
	cachedCX = cx;
	cachedCY = cy;
	cacheCount = 0;
   
	//printf("Time for cache check: %3.3fms\n",1000.0*(getSystemTime()-t0));
	//printf("%d cells visible\n",visibleCnt);
  } else
	cacheCount++;

  /* If we are doing stage 0 then recompile all display lists (both stage 0 and stage 1) 
	 that are either too old or have been marked as dirty */
  int redrawCnt=0;
  if(stage == 0) 
	for(ix=0;ix<41;ix++)
	  for(iy=0;iy<41;iy++)
	  if(cacheVisible[ix][iy]) {		
		x=cx+ix-20; y=cy+iy-20;
		if(x>=0&&x<width&&y>=0&&y<height) {
		  Cell& c = cell(x,y);
		  if(c.displayListLastCompiled + 10 < frameCount || c.displayListDirty || c.velocity[0] != 0.0 || c.velocity[1] != 0.0) {
			redrawCnt++;
			c.displayListLastCompiled = frameCount;
			c.displayListDirty=0;
			int thisStage=0;
			glNewList(c.displayList+thisStage,GL_COMPILE);
			drawCell(birdsEye,thisStage,x,y);			
			glEndList();
			if(isTransparent) {
			  thisStage=1;
			  glNewList(c.displayList+thisStage,GL_COMPILE);
			  drawCell(birdsEye,thisStage,x,y);			
			  glEndList();
			}
		  }
		}
	  }
  //printf("%d cells redrawn\n",redrawCnt);

  /* Call all the display lists to draw the actual ground */
  for(ix=0;ix<41;ix++)
	for(iy=0;iy<41;iy++)
	  if(cacheVisible[ix][iy]) {
		x=cx+ix-20; y=cy+iy-20;
		if(x>=0&&x<width&&y>=0&&y<height) {
		  Cell& c = cell(x,y);		  
		  glCallList(c.displayList+stage);
		}
	  }
  glPopMatrix();

  glPopAttrib();
}
void Map::drawFootprint(int x,int y,int kind) {
  Cell& center = cell(x,y);
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);

  if(Settings::settings->gfx_details >= 4) {
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  }
  glLineWidth(4);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glTranslatef(0,0,0.02);

  glDisable(GL_LIGHTING);
  glBegin(GL_LINE_LOOP); {
	if(kind == 0) glColor4f(0.2,0.2,0.5,1.0);
	else if(kind == 1) glColor4f(0.5,0.2,0.2,1.0);
	glVertex3f(x,y,center.heights[Cell::SOUTH+Cell::WEST]);
	glVertex3f(x+1,y,center.heights[Cell::SOUTH+Cell::EAST]);
	glVertex3f(x+1,y+1,center.heights[Cell::NORTH+Cell::EAST]);
	glVertex3f(x,y+1,center.heights[Cell::NORTH+Cell::WEST]);
  } glEnd();

  glLineWidth(1);
  glPopMatrix();
  glPopAttrib();
}

double smoothSemiRand(int x,int y,double scale) {
  double dt=(Game::current?Game::current->gameTime:((EditMode*)GameMode::current)->time)*scale;
  int t=(int)dt;
  double frac=dt - t;
  return semiRand(x,y,t) * (1.-frac) + semiRand(x,y,t+1) * frac;
}

void Map::drawCell(int birdsEye,int stage,int x,int y) {
  Coord3d normal,normal1;
  Real texScale = 0.25;
  int i,draw;
  int gfx_details = Settings::settings->gfx_details;
  double txOffset=0.0,tyOffset=0.0;
  Cell& c = cell(x,y);

  if(c.flags & CELL_ICE) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[tx_Ice]);
	glMaterialf(GL_FRONT,GL_SHININESS,10.0);
  } else if(c.flags & CELL_ACID) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[tx_Acid]);
	glMaterialf(GL_FRONT,GL_SHININESS,10.0);
  } else if(c.flags & CELL_SAND) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[tx_Sand]);
	glMaterialf(GL_FRONT,GL_SHININESS,10.0);
  } else if(c.flags & CELL_TRACK) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[tx_Track]);
	glMaterialf(GL_FRONT,GL_SHININESS,10.0);
  } else if(c.texture >= 0)  {
  	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[c.texture]);
	glMaterialf(GL_FRONT,GL_SHININESS,10.0);
  }
  if(c.flags & CELL_TRACK) {
	txOffset=-(Game::current?Game::current->gameTime:((EditMode*)GameMode::current)->time)*c.velocity[0]*texScale;
	tyOffset=-(Game::current?Game::current->gameTime:((EditMode*)GameMode::current)->time)*c.velocity[1]*texScale;
  }
  /* TODO. Do txOffset as a texture matrix transformation instead? */

  int topTransparent=0;
  for(i=0;i<5;i++) if(c.colors[i][3] < 0.95) topTransparent=1;

  /*** Draw the filled content of the top of this cell ***/
  if((topTransparent && stage==1) || (!topTransparent && stage==0)  || !isTransparent) {
	if(gfx_details >= 1) 
	  glBegin(GL_TRIANGLE_FAN); 
	else
	  glBegin(GL_QUADS); 
	{	
	  /* Specify vertices */
	  if(gfx_details >= 1) {
		c.getNormal(normal,Cell::CENTER);
		glNormal3dv(normal);
		glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.colors[Cell::CENTER]);
		glTexCoord2f((c.textureCoords[0][0]+c.textureCoords[1][0]+c.textureCoords[2][0]+c.textureCoords[3][0])/4.0+txOffset,
					 (c.textureCoords[0][1]+c.textureCoords[1][1]+c.textureCoords[2][1]+c.textureCoords[3][1])/4.0+tyOffset);
		glVertex3f(x+0.5,y+0.5,c.heights[Cell::CENTER]);
	  }

	  /* Note the use of "normal1" here so we can reuse it for the last vertice */
	  c.getNormal(normal1,Cell::SOUTH+Cell::WEST);
	  glNormal3dv(normal1);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.colors[Cell::SOUTH+Cell::WEST]);
	  glTexCoord2f(c.textureCoords[Cell::SOUTH+Cell::WEST][0]+txOffset,c.textureCoords[Cell::SOUTH+Cell::WEST][1]+tyOffset);
	  glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
	
	  c.getNormal(normal,Cell::SOUTH+Cell::EAST);
	  glNormal3dv(normal);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.colors[Cell::SOUTH+Cell::EAST]);
	  glTexCoord2f(c.textureCoords[Cell::SOUTH+Cell::EAST][0]+txOffset,c.textureCoords[Cell::SOUTH+Cell::EAST][1]+tyOffset);
	  glVertex3f(x+1.01,y,c.heights[Cell::SOUTH+Cell::EAST]);
	  
	  c.getNormal(normal,Cell::NORTH+Cell::EAST);
	  glNormal3dv(normal);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.colors[Cell::NORTH+Cell::EAST]);
	  glTexCoord2f(c.textureCoords[Cell::NORTH+Cell::EAST][0]+txOffset,c.textureCoords[Cell::NORTH+Cell::EAST][1]+tyOffset);
	  glVertex3f(x+1.01,y+1.01,c.heights[Cell::NORTH+Cell::EAST]);
	  
	  c.getNormal(normal,Cell::NORTH+Cell::WEST);
	  glNormal3dv(normal);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.colors[Cell::NORTH+Cell::WEST]);
	  glTexCoord2f(c.textureCoords[Cell::NORTH+Cell::WEST][0]+txOffset,c.textureCoords[Cell::NORTH+Cell::WEST][1]+tyOffset);
	  glVertex3f(x,y+1.01,c.heights[Cell::NORTH+Cell::WEST]);
		
	  if(gfx_details >= 1) {
		glNormal3dv(normal1);
		glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.colors[Cell::SOUTH+Cell::WEST]);
		glTexCoord2f(c.textureCoords[Cell::SOUTH+Cell::WEST][0]+txOffset,c.textureCoords[Cell::SOUTH+Cell::WEST][1]+tyOffset);
		glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
	  }
	} glEnd();
  }
  glDisable(GL_TEXTURE_2D);
  glMaterialf(GL_FRONT,GL_SHININESS,0.0);
  
  /*** Draw the lines around this cell */
  if(stage == 0 && !(c.flags & CELL_NOGRID)) {
	glPolygonMode(GL_FRONT,GL_LINE);
	GLfloat gridColor[4] = {0.0,0.0,0.0,0.8};
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,gridColor);
	glNormal3f(0.0,0.0,1.0);
	/* Use polygon offset to avoid stippling of the lines */
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0, -1.0);
	
	if(0 && Settings::settings->gfx_details >= 4) {
	  /* SORRY - no antialiasing for now! We have to draw the cells in two passes for this and I don't want to that for efficiency now (two display lists!) */
	  glEnable(GL_BLEND);
	  glEnable(GL_LINE_SMOOTH);
	  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	  glLineWidth(1.0);
	  if (Settings::settings->gfx_details >= 5) {
		/* TODO. Should we realy use polygon smooth here? */
		glEnable(GL_POLYGON_SMOOTH);
	  }
	} else {
	  glDisable(GL_BLEND);
	  glDisable(GL_LINE_SMOOTH);
	  glLineWidth(1.0);
	}

	glBegin(GL_QUADS); 
	if(c.flags & CELL_NOLINESOUTH) glEdgeFlag(GL_FALSE);
	else glEdgeFlag(GL_TRUE);
	glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
	if(c.flags & CELL_NOLINEEAST) glEdgeFlag(GL_FALSE);
	else glEdgeFlag(GL_TRUE);
	glVertex3f(x+1.01,y,c.heights[Cell::SOUTH+Cell::EAST]);
	if(c.flags & CELL_NOLINENORTH) glEdgeFlag(GL_FALSE);
	else glEdgeFlag(GL_TRUE);
	glVertex3f(x+1.01,y+1.01,c.heights[Cell::NORTH+Cell::EAST]);
	if(c.flags & CELL_NOLINEWEST) glEdgeFlag(GL_FALSE);
	else glEdgeFlag(GL_TRUE);
	glVertex3f(x,y+1.01,c.heights[Cell::NORTH+Cell::WEST]);
	glEnd();

	glDisable(GL_POLYGON_OFFSET_LINE);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT,GL_FILL);
  } 
    
  /* Draw south side of cell */
  {
	Cell& c2 = cell(x,y-1); draw=0;
	int southTransparent=c.wallColors[Cell::SOUTH+Cell::EAST][3] < 0.95 || c.wallColors[Cell::SOUTH+Cell::WEST][3] < 0.95 || 
	  c2.wallColors[Cell::NORTH+Cell::EAST][3] < 0.95 || c2.wallColors[Cell::NORTH+Cell::WEST][3] < 0.95;
	if((southTransparent && stage==1) || (!southTransparent && stage==0) || !isTransparent)
	  {
		
		if(c2.heights[Cell::NORTH+Cell::WEST] < c.heights[Cell::SOUTH+Cell::WEST] ||
		   c2.heights[Cell::NORTH+Cell::EAST] < c.heights[Cell::SOUTH+Cell::EAST]) {
		  glNormal3f(0.0,-1.0,0.0);
		  draw=1;
		} else
		  glNormal3f(0.0,+1.0,0.0);
		
		if(birdsEye || draw) {
		  glBegin(GL_TRIANGLE_STRIP);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::NORTH+Cell::EAST]);
		  glVertex3f(x+1.01,y,c2.heights[Cell::NORTH+Cell::EAST]);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::SOUTH+Cell::EAST]);
		  glVertex3f(x+1.01,y,c.heights[Cell::SOUTH+Cell::EAST]);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::NORTH+Cell::WEST]);
		  glVertex3f(x,y,c2.heights[Cell::NORTH+Cell::WEST]);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::SOUTH+Cell::WEST]);
		  glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
		  glEnd();
		}
	  }
  }
  
  /* Draw west side of cell */
  {
	Cell& c2 = cell(x-1,y); draw=0;
	int westTransparent=c.wallColors[Cell::SOUTH+Cell::WEST][3] < 0.95 || c.wallColors[Cell::NORTH+Cell::WEST][3] < 0.95 || 
	  c2.wallColors[Cell::SOUTH+Cell::EAST][3] < 0.95 || c2.wallColors[Cell::NORTH+Cell::EAST][3] < 0.95;
	if((westTransparent && stage==1) || (!westTransparent && stage==0) || !isTransparent)
	  {
		
		if(c2.heights[Cell::SOUTH+Cell::EAST] < c.heights[Cell::SOUTH+Cell::WEST] ||
		   c2.heights[Cell::NORTH+Cell::EAST] < c.heights[Cell::NORTH+Cell::WEST]) {
		  glNormal3f(-1.0,0.0,0.0);
		  draw=1;
		} else
		  glNormal3f(+1.0,0.0,0.0);
	  
		if(birdsEye || draw) {
		  glColor4f(0.0,0.0,0.0,1.0);
		  glBegin(GL_TRIANGLE_STRIP);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::SOUTH+Cell::EAST]);
		  glVertex3f(x,y,c2.heights[Cell::SOUTH+Cell::EAST]);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::SOUTH+Cell::WEST]);
		  glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::NORTH+Cell::EAST]);
		  glVertex3f(x,y+1.01,c2.heights[Cell::NORTH+Cell::EAST]);
		  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::NORTH+Cell::WEST]);
		  glVertex3f(x,y+1.01,c.heights[Cell::NORTH+Cell::WEST]);
		  glEnd();
		}
	  }
  }

  /******************* draw water ****************/
  if(stage == 1 &&
	 (c.waterHeights[0] > c.heights[0] || c.waterHeights[1] > c.heights[1] || 
	  c.waterHeights[2] > c.heights[2] || c.waterHeights[3] > c.heights[3] || c.waterHeights[4] > c.heights[4])) {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[tx_Ice]);
	glMaterialf(GL_FRONT,GL_SHININESS,10.0);
	txOffset=-(Game::current?Game::current->gameTime:((EditMode*)GameMode::current)->time)*c.velocity[0]*texScale;
	tyOffset=-(Game::current?Game::current->gameTime:((EditMode*)GameMode::current)->time)*c.velocity[1]*texScale;

	if(gfx_details >= 1) 
	  glBegin(GL_TRIANGLE_FAN); 
	else
	  glBegin(GL_POLYGON); 
	{	
		
	  GLfloat colors[4]={0.3,0.3,0.7,0.6};

	  /* Specify vertices */
	  if(gfx_details >= 1) {
		c.getWaterNormal(normal,Cell::CENTER);
		glNormal3dv(normal);
		//colors[3]=max(0.7,(c.waterHeights[Cell::CENTER]-c.heights[Cell::CENTER])*1.0);
		glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colors);
		glTexCoord2f((0.5+x)*texScale+txOffset,(0.5+y)*texScale+tyOffset);
		glVertex3f(x+0.5,y+0.5,c.waterHeights[Cell::CENTER]);
	  }

	  c.getWaterNormal(normal,Cell::SOUTH+Cell::WEST);
	  glNormal3dv(normal);
	  //colors[3]=max(0.60,(c.waterHeights[Cell::SOUTH+Cell::WEST]-c.heights[Cell::SOUTH+Cell::WEST])*1.0);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colors);
	  glTexCoord2f((0.0+x)*texScale+txOffset+smoothSemiRand(x,y,1.0),(0.0+y)*texScale+tyOffset+smoothSemiRand(x+17,y,1.0));
	  glVertex3f(x,y,c.waterHeights[Cell::SOUTH+Cell::WEST]);
		
	  c.getWaterNormal(normal,Cell::SOUTH+Cell::EAST);
	  glNormal3dv(normal);
	  //colors[3]=max(0.60,(c.waterHeights[Cell::SOUTH+Cell::EAST]-c.heights[Cell::SOUTH+Cell::EAST])*1.0);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colors);
	  glTexCoord2f((1.0+x)*texScale+txOffset+smoothSemiRand(x+1,y,0.5),(0.0+y)*texScale+tyOffset+smoothSemiRand(x+17,y,0.5));
	  glVertex3f(x+1.01,y,c.waterHeights[Cell::SOUTH+Cell::EAST]);
	  
	  c.getWaterNormal(normal,Cell::NORTH+Cell::EAST);
	  glNormal3dv(normal);
	  //colors[3]=max(0.60,(c.waterHeights[Cell::NORTH+Cell::EAST]-c.heights[Cell::NORTH+Cell::EAST])*1.0);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colors);
	  glTexCoord2f((1.0+x)*texScale+txOffset+smoothSemiRand(x+1,y,0.5),(1.0+y)*texScale+tyOffset+smoothSemiRand(x+17,y+1,0.5));
	  glVertex3f(x+1.01,y+1.01,c.waterHeights[Cell::NORTH+Cell::EAST]);
	  
	  c.getWaterNormal(normal,Cell::NORTH+Cell::WEST);
	  glNormal3dv(normal);
	  //colors[3]=max(0.60,(c.waterHeights[Cell::NORTH+Cell::WEST]-c.heights[Cell::NORTH+Cell::WEST])*1.0);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colors);
	  glTexCoord2f((0.0+x)*texScale+txOffset+smoothSemiRand(x,y,0.5),(1.0+y)*texScale+tyOffset+smoothSemiRand(x+17,y+1,0.5));
	  glVertex3f(x,y+1.01,c.waterHeights[Cell::NORTH+Cell::WEST]);
		
	  c.getWaterNormal(normal,Cell::SOUTH+Cell::WEST);
	  glNormal3dv(normal);
	  //colors[3]=max(0.60,(c.waterHeights[Cell::SOUTH+Cell::WEST]-c.heights[Cell::SOUTH+Cell::WEST])*1.0);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,colors);
	  glTexCoord2f((0.0+x)*texScale+txOffset+smoothSemiRand(x,y,0.5),(0.0+y)*texScale+tyOffset+smoothSemiRand(x+17,y,0.5));
	  glVertex3f(x,y,c.waterHeights[Cell::SOUTH+Cell::WEST]);
	} glEnd();

	glMaterialf(GL_FRONT,GL_SHININESS,0.0);
	glDisable(GL_TEXTURE_2D);
  }
}

/****************************************************
 drawCellAA
****************************************************/

void Map::drawCellAA(int birdsEye,int x,int y) {
  Coord3d normal;
  int i,draw;
  int gfx_details = Settings::settings->gfx_details;
  
  Cell& c = cell(x,y);

  /* Draw south side of cell */
  {
	Cell& c2 = cell(x,y-1); draw=0;
	if(c2.heights[Cell::NORTH+Cell::WEST] < c.heights[Cell::SOUTH+Cell::WEST] ||
	   c2.heights[Cell::NORTH+Cell::EAST] < c.heights[Cell::SOUTH+Cell::EAST]) {
	  glNormal3f(0.0,-1.0,0.0);
	  draw=1;
	} else
	  glNormal3f(0.0,+1.0,0.0);
	
	if(birdsEye || draw) {
	  glBegin(GL_LINES);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::NORTH+Cell::EAST]);
	  glVertex3f(x+1.01,y,c2.heights[Cell::NORTH+Cell::EAST]);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::SOUTH+Cell::EAST]);
	  glVertex3f(x+1.01,y,c.heights[Cell::SOUTH+Cell::EAST]);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::NORTH+Cell::WEST]);
	  glVertex3f(x,y,c2.heights[Cell::NORTH+Cell::WEST]);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::SOUTH+Cell::WEST]);
	  glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
	  glEnd();
	}
  }
  
  /* Draw west side of cell */
  {
	Cell& c2 = cell(x-1,y); draw=0;
	if(c2.heights[Cell::SOUTH+Cell::EAST] < c.heights[Cell::SOUTH+Cell::WEST] ||
	   c2.heights[Cell::NORTH+Cell::EAST] < c.heights[Cell::NORTH+Cell::WEST]) {
	  glNormal3f(-1.0,0.0,0.0);
	  draw=1;
	} else
	  glNormal3f(+1.0,0.0,0.0);
	
	if(birdsEye || draw) {
	  glBegin(GL_LINES);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::SOUTH+Cell::EAST]);
	  glVertex3f(x,y,c2.heights[Cell::SOUTH+Cell::EAST]);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::SOUTH+Cell::WEST]);
	  glVertex3f(x,y,c.heights[Cell::SOUTH+Cell::WEST]);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c2.wallColors[Cell::NORTH+Cell::EAST]);
	  glVertex3f(x,y+1.01,c2.heights[Cell::NORTH+Cell::EAST]);
	  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,c.wallColors[Cell::NORTH+Cell::WEST]);
	  glVertex3f(x,y+1.01,c.heights[Cell::NORTH+Cell::WEST]);
	  glEnd();		
	}
  }
}


/** Saves the map to file in compressed binary or compressed ascii format */
int Map::save(char *pathname,int x,int y) {
  int i,version=mapFormatVersion;

  if(pathIsLink(pathname)) {
	fprintf(stderr,"Error, %s is a link, cannot save map\n",pathname);
	return 0;
  }
  
  gzFile gp = gzopen(pathname,"wb9");
  if(!gp) return 0;
  startPosition[0] = x;
  startPosition[1] = y;
  startPosition[2] = cell(x,y).heights[Cell::CENTER]; 
  int32_t data[6];
  for(i=0;i<3;i++) data[i] = saveInt((int32_t) startPosition[i]); // no decimal part needed
  data[3] = saveInt((int32_t) width);
  data[4] = saveInt((int32_t) height);
  data[5] = saveInt((int32_t) version);
  gzwrite(gp,data,sizeof(int32_t)*6);

  /* new from version 7, save texture names */
  data[0]=saveInt(numTextures);
  gzwrite(gp,data,sizeof(int32_t)*1);	
  for(i=0;i<numTextures;i++) {
	char textureName[64];
	snprintf(textureName,63,textureNames[i]);
	gzwrite(gp,textureName,64);	
  }

  for(i=0;i<width*height;i++)
	cells[i].dump(gp);
  gzclose(gp);   
  return 1;
}

void Cell::dump(gzFile gp) {
  Real texCoord[5];

  int32_t data[8];
  int i,j;
  for(i=0;i<5;i++)
	data[i] = saveInt((int32_t) (heights[i]/0.0025));
  gzwrite(gp,data,sizeof(int32_t)*5);
  for(i=0;i<5;i++) {
	for(j=0;j<4;j++) data[j] = saveInt((int32_t) (colors[i][j] / 0.01));
	gzwrite(gp,data,sizeof(int32_t)*4);
  }
  data[0] = saveInt((int32_t) flags);
  data[1] = saveInt((int32_t) texture);
  gzwrite(gp,data,sizeof(int32_t)*2);

  for(i=0;i<4;i++) {
	for(j=0;j<4;j++) data[j] = saveInt((int32_t) (wallColors[i][j] / 0.01));
	gzwrite(gp,data,sizeof(int32_t)*4);
  }

  data[0]=saveInt((int32_t) (velocity[0] / 0.01));
  data[1]=saveInt((int32_t) (velocity[1] / 0.01));
  gzwrite(gp,data,sizeof(int32_t)*2);

  for(i=0;i<5;i++)
	data[i] = saveInt((int32_t) (waterHeights[i]/0.0025));
  gzwrite(gp,data,sizeof(int32_t)*5);

  for(i=0;i<4;i++) {	
	data[i*2] = texture>=0||flags?saveInt((int32_t) (textureCoords[i][0]/0.0025)):0;
	data[i*2+1] = texture>=0||flags?saveInt((int32_t) (textureCoords[i][1]/0.0025)):0;
  }
  gzwrite(gp,data,sizeof(int32_t)*8);
}


void Cell::load(Map *map,gzFile gp,int version) {
  int32_t data[8];
  int i,j;

  gzread(gp,data,sizeof(int32_t)*5);
  for(i=0;i<5;i++)
	heights[i] = 0.0025 * loadInt(data[i]);
  for(i=0;i<5;i++) {
	if(version < 4) {
	  // old maps do not have an alpha channel defined
	  gzread(gp,data,sizeof(int32_t)*3);	
	  for(j=0;j<3;j++)
		colors[i][j] = 0.01 * loadInt(data[j]);
	  colors[i][3] = 1.0;
	} else {
	  gzread(gp,data,sizeof(int32_t)*4);	
	  for(j=0;j<4;j++)
		colors[i][j] = 0.01 * loadInt(data[j]);
	}
  }

  gzread(gp,data,sizeof(int32_t)*2);
  flags = loadInt(data[0]);
  if(version <= 1) flags = flags & (CELL_ICE|CELL_ACID);
  i=loadInt(data[1]);
  texture=(i>=0&&i<numTextures?map->indexTranslation[i]:-1);
  // in older maps, this field was not initialized
  if(version < 5)
    texture=-1;
  if(version < 3) { /* Old maps do not have wallColors defined */
	for(i=0;i<4;i++) {
	  wallColors[i][0] = 0.7;
	  wallColors[i][1] = 0.2;
	  wallColors[i][2] = 0.2;
	  wallColors[i][3] = 1.0;
	}
  } else {
	for(i=0;i<4;i++) {
	  if(version < 4) {
		// old maps do not have an alpha channel defined
		gzread(gp,data,sizeof(int32_t)*3);
		for(j=0;j<3;j++)
		  wallColors[i][j] = 0.01 * loadInt(data[j]);
		wallColors[i][3] = 1.0;
	  } else {
		gzread(gp,data,sizeof(int32_t)*4);
		for(j=0;j<4;j++)
		  wallColors[i][j] = 0.01 * loadInt(data[j]);
	  }
	}
  }

  if(version >= 5) {
	gzread(gp,data,sizeof(int32_t)*2);
	velocity[0] = 0.01 * loadInt(data[0]);
	velocity[1] = 0.01 * loadInt(data[1]);
  } else {
	velocity[0] = 0.0;
	velocity[1] = 0.0;
  }

  // currently we just reset the ground water
  if(version >= 6) {
	gzread(gp,data,sizeof(int32_t)*5);
	for(i=0;i<5;i++)
	  waterHeights[i] = 0.0025 * loadInt(data[i]);
  } else {
	for(i=0;i<5;i++)
	  waterHeights[i]=heights[i] - 0.5;
  }
  sunken = 0.0;

  if(version >= 7) {
	gzread(gp,data,sizeof(int32_t)*8);	
	for(i=0;i<4;i++) {
	  float tx=0.0025 * loadInt(data[i*2]);
	  float ty=0.0025 * loadInt(data[i*2+1]);
	  if(tx || ty) {
		textureCoords[i][0]=tx;
		textureCoords[i][1]=ty;
	  }
	}
  }
}

/* Returns the height at a specified (floating point) position */
/* now inlined
Real Map::getHeight(Real x,Real y) {
  int ix=(int) x;
  int iy=(int) y;
  Cell& c = cell(ix,iy);
  return c.getHeight(x-ix,y-iy);
}
*/

