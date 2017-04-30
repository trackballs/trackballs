/* map.h
   Handles all information about the current map, including the individual cells.

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

#ifndef MAP_H
#define MAP_H

// more simple like this...
#define CELL_ICE             (1 << 0)
#define CELL_ACID            (1 << 1)
#define CELL_SAND            (1 << 2)
#define CELL_KILL            (1 << 3)
#define CELL_TRAMPOLINE      (1 << 4)
#define CELL_NOGRID          (1 << 5)
#define CELL_TRACK           (1 << 6)
#define CELL_NOLINENORTH     (1 << 7)
#define CELL_NOLINESOUTH     (1 << 8)
#define CELL_NOLINEEAST      (1 << 9)
#define CELL_NOLINEWEST      (1 << 10)

class Cell {
 public:
  /* Note. if you add fields here you must update the dump, load functions as well as editMode::pasteRegion */

  float velocity[2],heights[5];
  GLfloat colors[5][4];
  GLfloat wallColors[4][4];
  float sunken; // used for trampoline effect
  float waterHeights[5];  // absolute height of water in cell, values <= -100.0 is treated as no water.
  float textureCoords[4][2]; // Texture coordinates of corners. Center is average.

  int flags,texture;
  int displayList, displayListLastCompiled, displayListDirty;

  void dump(gzFile gp);
  void load(class Map *map,gzFile gp,int version);
  
  Cell();
  void getNormal(Coord3d,int vertex);
  void getWaterNormal(Coord3d,int vertex);
  Real getHeight(Real x,Real y);
  Real getWaterHeight(Real x,Real y);
  static const int NORTH=1,SOUTH=0,EAST=2,WEST=0,CENTER=4;
};

class Map {
 public:
  Map(char *mapname);
  virtual ~Map();

  inline Cell& cell(int x,int y) {
    return cells[(x<0?0:(x>=width?width-1:x))+width*(y<0?0:(y>=height?height-1:y))];
  };
  int save(char *name,int x,int y);
  void draw(int birdseye,int stage,int x,int y);
  void drawCell(int birdsEye,int stage,int x,int y);
  void drawCellAA(int birdsEye,int x,int y);
  void drawFootprint(int x,int y,int kind);
  inline Real getHeight(Real x,Real y) {
	int ix=(int) x;
	int iy=(int) y;
	return cell(ix,iy).getHeight(x-ix,y-iy);
  }
  inline Real getWaterHeight(Real x,Real y) {
	int ix=(int) x;
	int iy=(int) y;
	Cell& c = cell(ix,iy);
	return c.getWaterHeight(x-ix,y-iy);
  }


  int width;
  int height; 
  int flags,isBonus,isTransparent;
  char mapname[256],author[256];
  int indexTranslation[256]; // translates indices when reading map.

  int tx_Ice,tx_Acid,tx_Sand,tx_Track;
  Coord3d startPosition;

  static const int flagNone,flagFlashCenter,flagTranslucent,flagShowCross;
  static const int mapFormatVersion;
 protected:
 private:
  Cell *cells;

  int cachedCX, cachedCY, cacheCount;
  char cacheVisible[41][41];  

  /* For maintaining display lists  */
  int displayLists, nLists, nListsWide;
};

#endif
