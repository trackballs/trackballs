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

#include "myWindow.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

MyWindow *MyWindow::allWindows = NULL;

MyWindow::MyWindow(int x, int y, int width, int height) {
  prev = NULL;
  next = NULL;
  isDragging = 0;
  isDraggable = 1;
  dragStartX = 0;
  dragStartY = 0;

  moveTo(x, y);
  resize(width, height);

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
void MyWindow::mouseDown(int /*button*/, int x, int y) {
  if (isDraggable) isDragging = 1;
  dragStartX = x;
  dragStartY = y;
}
void MyWindow::mouse(int state, int x, int y) {
  if (!(state & SDL_BUTTON_LMASK))
    isDragging = 0;
  else {
    if (isDragging) {
      moveTo(this->x + x - dragStartX, this->y + y - dragStartY);
      dragStartX = x;
      dragStartY = y;
    }
  }
}
void MyWindow::key(int /*key*/, int /*shift*/, int /*x*/, int /*y*/) {}

void MyWindow::tick() {}
void MyWindow::draw() {
  // 3 overlapping rectangles: shadow, rim, face
  draw2DRectangle(x + 1, y + 1, width + 2, height + 2, 0., 0., 1., 1., 0., 0., 0., 0.5);
  draw2DRectangle(x - 1, y - 1, width + 2, height + 2, 0., 0., 1., 1., border[0], border[1],
                  border[2], border[3]);
  draw2DRectangle(x, y, width, height, 0., 0., 1., 1., background[0], background[1],
                  background[2], background[3]);
}
void MyWindow::attached() {}
void MyWindow::removed() {}

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
  if (!prev) return; /* We where previously not attached */
  *prev = next;
  prev = NULL;
  removed();
}
void MyWindow::raiseToFront() {
  if (!prev) return; /* Not currently attached */
  *prev = next;
  next = allWindows;
  prev = &allWindows;
  allWindows = this;
}
void MyWindow::lowerToBack() { /* TODO. Not yet implemented */
}
void MyWindow::moveTo(int x, int y) {
  this->x = x;
  this->y = y;
}
void MyWindow::resize(int w, int h) {
  this->width = w;
  this->height = h;
}

int MyWindow::isInside(int x, int y) const {
  return x >= this->x && x < this->x + this->width && y >= this->y &&
         y < this->y + this->height;
}

/*                          */
/*     Static functions     */
/*                          */
void MyWindow::resetWindows() {
  MyWindow *pntr = allWindows;
  while (pntr) {
    MyWindow *next = pntr->next;
    pntr->next = NULL;
    pntr->prev = NULL;
    pntr->removed();
    pntr = next;
  }
  allWindows = NULL;
}

void MyWindow::drawAll() {
  /* This drawing function is a little bit messy since
         we have to draw from back to front although the list
         is given from front to back only. We solve it with recursion */
  drawAll(allWindows);
}
void MyWindow::drawAll(MyWindow *window) {
  if (!window) return;
  drawAll(window->next);
  window->draw();
}
void MyWindow::tickAll() {
  MyWindow *pntr = allWindows;
  while (pntr) {
    MyWindow *next = pntr->next;
    pntr->tick();
    pntr = next;
  }
}
void MyWindow::mouseAll(int state, int x, int y) {
  MyWindow *pntr = allWindows;
  while (pntr) {
    MyWindow *next = pntr->next;
    if (pntr->isInside(x, y)) {
      pntr->mouse(state, x, y);
      break;
    }
    pntr = next;
  }
}
void MyWindow::mouseDownAll(int state, int x, int y) {
  MyWindow *pntr = allWindows;
  while (pntr) {
    MyWindow *next = pntr->next;
    if (pntr->isInside(x, y) || (x == -1 && y == -1)) {
      pntr->mouseDown(state, x, y);
      break;
    }
    pntr = next;
  }
}
void MyWindow::keyAll(int key) {
  int x, y;
  SDL_GetMouseState(&x, &y);
  int shift = SDL_GetModState() & KMOD_SHIFT;

  MyWindow *pntr = allWindows;
  while (pntr) {
    MyWindow *next = pntr->next;
    if (pntr->isInside(x, y)) {
      pntr->key(key, shift, x, y);
      break;
    }
    pntr = next;
  }
}
int MyWindow::isAttached() const {
  MyWindow *pntr = allWindows;
  while (pntr) {
    if (pntr == this) return 1;
    pntr = pntr->next;
  }
  return 0;
}
