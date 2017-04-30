/* editWindows.h
   Implements all the windows in the editMode

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

#ifndef EDITWINDOWS_H
#define EDITWINDOWS_H

#ifndef EDITMODECODES_H
#include "editMode_codes.h"
#endif

class EMenuWindow : public MyWindow {
 public: 
  EMenuWindow(); 
  ~EMenuWindow(); 
  void draw();
  void mouseDown(int state,int x,int y);
  void openSubMenu(int);
  void key(int key);

  int keyToMenuEntry(int);
 private:
  int spacing;

  int activeSubID;
  class ESubWindow *activeSubWindow;
  class ESubWindow *subWindows[N_SUBMENUS];
};

class ESubWindow : public MyWindow {
 public:
  ESubWindow(int id,int x,int y); 
  void draw();
  void mouseDown(int state,int x,int y);
 private:
  int id,rows,fontSize;

  int countRows();
};

class EStatusWindow : public MyWindow {
 public:
  EStatusWindow(); 
  void draw();
  void mouseDown(int state,int x,int y);
 private:
};

class EQuitWindow : public MyWindow {
 public:
  EQuitWindow(); 
  void yes();
  void no();

  void draw();
  void mouseDown(int state,int x,int y);
 private:
};

class ESaveWindow : public MyWindow {
 public:
  ESaveWindow(); 
  void yes();
  void no();

  void draw();
  void mouseDown(int state,int x,int y);
 private:
  int saveCnt;
};

class ECloseWindow : public MyWindow {
 public:
  ECloseWindow(); 
  void yes();
  void no();

  void draw();
  void mouseDown(int state,int x,int y);
 private:
};

class EOpenWindow : public MyWindow {
 public:
  EOpenWindow(); 

  void draw();
  void mouseDown(int state,int x,int y);
  void refreshMapList();
 private:
  char names[200][256];
  int nNames, currPage;
};

class ENewWindow : public MyWindow {
 public:
  ENewWindow(); 

  void draw();
  void mouseDown(int state,int x,int y);
  void key(int key,int x,int y);  

 private:
  char name[256];
};

#endif
