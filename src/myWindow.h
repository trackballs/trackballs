/* myWindow.h
   Provides a simple windowing system inside openGL. 

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

#ifndef MYWINDOW_H
#define MYWINDOW_H

class MyWindow {
 public:
  MyWindow(int x,int y,int width,int height);
  /* All drawing is done from back to front, so we only need
	 one drawing stage even if the windows are rendered with 
	 translucency
  */
  virtual void draw();
  virtual void tick();
  virtual void mouse(int state,int x,int y);
  virtual void mouseDown(int button,int x,int y);
  virtual void key(int key,int x,int y);  
  virtual void attached();
  virtual void removed();

  /* Changes position of window in current drawing hierarchy */
  void raiseToFront();
  void lowerToBack();
  void moveTo(int x,int y);
  void resize(int w,int h);

  /* Add's or removes a specific window */
  void attach();
  void remove();
  
  /* Test if a screen point is inside window */
  int isInside(int x,int y);

  /* Test is this window is attached */
  int isAttached();

  /* Removes all windows from current drawing list of windows */
  static void resetWindows();
  
  /* Initiate drawing / event handling by all appropriate windows */
  static void drawAll();
  static void tickAll();
  static void mouseAll(int state,int x,int y);
  static void mouseDownAll(int state,int x,int y);
  static void keyAll(int key);

  /* Coordinates of upper left corner + width/height of window */
  int x,y,width,height;
  int isDraggable;
  GLfloat background[4];
  GLfloat border[4];

 private:
  MyWindow *next, **prev;
  static MyWindow *allWindows;
  int isDragging,dragStartX,dragStartY;

  static void drawAll(MyWindow *start); 
};

#endif
