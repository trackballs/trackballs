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

#include "general.h"
#include "glHelp.h"
#include "menusystem.h"
#include "myWindow.h"

using namespace std;

MyWindow *MyWindow::allWindows=NULL;

MyWindow::MyWindow(int x,int y,int width,int height) {  
  prev=NULL;
  next=NULL;
  isDragging=0;
  isDraggable=1;

  moveTo(x,y);
  resize(width,height);

  /* Default background */
  background[0] = 0.2;
  background[1] = 0.2;
  background[2] = 0.5;
  background[3] = 0.8;

  /* Default border color */
  border[0] = 0.8;
  border[1] = 0.8;
  border[2] = 0.8;
  border[3] = 1.0;
}

/*                               */
/* Default for virtual functions */
/*                               */
void MyWindow::mouseDown(int button,int x,int y) {
  if(isDraggable) isDragging=1;
  dragStartX=x; dragStartY=y;
}
void MyWindow::mouse(int state,int x,int y) {
  if(!(state & SDL_BUTTON_LMASK)) isDragging=0;
  else {
	if(isDragging) {	
	  moveTo(this->x+x-dragStartX,this->y+y-dragStartY);
	  dragStartX=x; dragStartY=y;
	}  
  }
}
void MyWindow::key(int key,int x,int y) {}

void MyWindow::tick() { }
void MyWindow::draw() {
  Enter2DMode();
  glDisable(GL_TEXTURE_2D);

  glColor4f(0.0,0.0,0.0,0.5);
  glLineWidth(2);
  glBegin(GL_LINES);
  glVertex2i(x+width+2,y+1); 	
  glVertex2i(x+width+2,y+height+1);
  glVertex2i(x+2,y+height+1); 	
  glVertex2i(x+width+2,y+height+1);
  glEnd();


  glColor4fv(background);
  glBegin(GL_QUADS);
  glVertex2i(x,y); 	
  glVertex2i(x,y+height);
  glVertex2i(x+width,y+height);
  glVertex2i(x+width,y);
  glEnd();
  glLineWidth(1);
  glColor4fv(border);
  glBegin(GL_LINE_LOOP);
  glVertex2i(x,y); 	
  glVertex2i(x+width,y);
  glVertex2i(x+width,y+height);
  glVertex2i(x,y+height);
  glEnd();
  Leave2DMode();
}
void MyWindow::attached() {
}
void MyWindow::removed() {
}

/*                               */
/*   Window handling functions   */
/*                               */
void MyWindow::attach() {
  next = allWindows;
  prev = &allWindows;
  allWindows = this;
  attached();  
}
void MyWindow::remove() {
  if(!prev) return; /* We where previously not attached */
  *prev=next;
  prev=NULL;
  removed();
}
void MyWindow::raiseToFront() {
  if(!prev) return; /* Not currently attached */
  *prev=next;
  next = allWindows;
  prev = &allWindows;
  allWindows = this;
}
void MyWindow::lowerToBack() {
  /* TODO. Not yet implemented */
}
void MyWindow::moveTo(int x,int y) { this->x = x; this->y = y; }
void MyWindow::resize(int w,int h) { this->width = w; this->height = h; }

int MyWindow::isInside(int x,int y) {
  return x >= this->x && x < this->x+this->width &&
	y >= this->y && y < this->y+this->height;
}

/*                          */
/*     Static functions     */
/*                          */
void MyWindow::resetWindows() {
  MyWindow *pntr=allWindows;
  while(pntr) {
	MyWindow *next=pntr->next;
	pntr->next=NULL;
	pntr->prev=NULL;
	pntr->removed();
	pntr=next;
  }
  allWindows=NULL;
} 

void MyWindow::drawAll() {
  /* This drawing function is a little bit messy since
	 we have to draw from back to front although the list
	 is given from front to back only. We solve it with recursion */
  drawAll(allWindows);
}
void MyWindow::drawAll(MyWindow *window) {
  if(!window) return;
  drawAll(window->next);
  window->draw();
}
void MyWindow::tickAll() {
  MyWindow *pntr=allWindows;
  while(pntr) {
	MyWindow *next=pntr->next;
	pntr->tick();
	pntr=next;
  }  
}
void MyWindow::mouseAll(int state,int x,int y) {
  MyWindow *pntr=allWindows;
  while(pntr) {
	MyWindow *next=pntr->next;
	if(pntr->isInside(x,y)) {
	  pntr->mouse(state,x,y);
	  break;
	}
	pntr=next;
  }  
}
void MyWindow::mouseDownAll(int state,int x,int y) {
  MyWindow *pntr=allWindows;
  while(pntr) {
	MyWindow *next=pntr->next;
	if(pntr->isInside(x,y)) {
	  pntr->mouseDown(state,x,y);
	  break;
	}
	pntr=next;
  }  
}
void MyWindow::keyAll(int key) {
  int x,y;
  SDL_GetMouseState(&x,&y);

  MyWindow *pntr=allWindows;
  while(pntr) {
	MyWindow *next=pntr->next;
	if(pntr->isInside(x,y)) {
	  pntr->key(key,x,y);
	  break;
	}
	pntr=next;
  }  
}
int MyWindow::isAttached() {
  MyWindow *pntr=allWindows;
  while(pntr) {
	if(pntr == this) return 1;
	pntr = pntr->next;
  }
  return 0;
}
