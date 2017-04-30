/* pipe.h
   One-way pipes dragging balls along them

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

#ifndef PIPE_H
#define PIPE_H

#define PIPE_SOFT_ENTER  1
#define PIPE_SOFT_EXIT   2

class Pipe : public Animated {
 public:
  Pipe(Coord3d from,Coord3d to,Real radius);
  void draw();
  void draw2();
  void tick(Real t);
  /** Recomputes the bounding box of the pipe. Needed after changes in from/to position */
  void computeBoundingBox();
  void onRemove();

  static void init();
  static void reset();

  Coord3d from,to;  
  Coord3d up,right;
  Real radius;

  static class std::set<Pipe *> *pipes;  
  Real windForward,windBackward;
 private:
  void drawTrunk();
};


#endif
