/* editMode.h
   Enables the editing of a given map.

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

#ifndef EDITMODE_H
#define EDITMODE_H

#include "gameMode.h"

class Game;
class Map;

class EditMode : public GameMode {
 public:
  EditMode();
  ~EditMode();

  static void init();

  void loadMap(char *mapname);
  void closeMap();
  void saveMap();

  void drawMenus();
  void drawInfo();

  void display();
  void key(int);
  void special(int, int, int);
  void idle(Real td);
  void mouseDown(int state, int x, int y);

  void activated();
  void deactivated();
  void closeAllDialogWindows();
  void askNew();
  void askQuit();
  void askSave();
  void askClose();
  void askOpen();
  void testLevel();
  void copyRegion();
  void pasteRegion();

  Game *game;
  Map *map;

  char levelname[256];
  char pathname[256];
  double time;

  static EditMode *editMode;

 protected:
 private:
  void doCommand(int);
  void doCellAction(int, int);
  void makeHill(int radius);
  void doSmooth(int radius);

  int x, y;
  int mapIsWritable;

  double scale; /* This is the increment with which we modify heights etc. or scale textures */
  double rotation; /* Only for textures */
  Real raise;
  Real color[4];
  int menuChoise;
  int doSave;
  int hill;
  int currentEditMode, currentFeature;

  class EMenuWindow *menuWindow;
  class EStatusWindow *statusWindow;
  class EQuitWindow *quitWindow;
  class ESaveWindow *saveWindow;
  class ECloseWindow *closeWindow;
  class EOpenWindow *openWindow;
  class ENewWindow *newWindow;
  friend class EMenuWindow;
  friend class ESubWindow;
  friend class EStatusWindow;

  class Cell *cellClipboard;
  int cellClipboardWidth, cellClipboardHeight;
};

#endif
