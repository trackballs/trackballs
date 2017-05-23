/** \file editMode.cc
   Enables the editing of a given map.
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
#include "gameMode.h"
#include "editMode.h"
#include "glHelp.h"
#include "map.h"
#include "settings.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "menusystem.h"
#include "myWindow.h"
#include "editWindows.h"
#include "editMode_codes.h"
#include "setupMode.h"

using namespace std;

/* See cMenuNames_i18n inside EditMode::init for the values here */
char* cMenuNames[N_SUBMENUS];

/* Names of all submeny entries.
   If name begins with '*' then it is just decoration.
   If name begins with '/' then don't prepend shortcut key (already part of name).
   Otherwise, prepend shortcut key and make a selectagle area of the menu entry.
   The menu codes in editMode_codes must correspond to the menus placement in this struct.
*/
/* See cMenuEntries_i18n inside EditMode::init for the values here */
char* cMenuEntries[N_SUBMENUS][MAX_MENU_ENTRIES];

/* Explanation: one string per menu. Each character corresponds to one shortcut key, * means no
   shortcut
   available. If multiple entries share the same shortcut it is the currently active entry that
   wins or
   the lowest numbered if none of them are currently open.
*/
const char* cKeyShortcuts[N_SUBMENUS] = {
    /* File */
    "***sq*",
    /* Edit */
    "***** umkhj+-",
    /* Colour */
    "1234",
    /* Flag */
    "12345678*t",
    /* Features */
    "1234567",
    /* Move */
    "UDLR*******rcxv",
    /* Window */
    "***",
    /* View */
    "br",
};

#define FLAG_ICE 0
#define FLAG_ACID 1
#define FLAG_SAND 2
#define FLAG_KILL 3
#define FLAG_TRAMPOLINE 4
#define FLAG_NOGRID 5
#define FLAG_TRACK 6
#define FLAG_FLAT 7
#define NUM_FLAGS 8

char* flagNames[NUM_FLAGS];

#define HILL_SPIKE 0
#define HILL_SMALL 1
#define HILL_MEDIUM 2
#define HILL_LARGE 3
#define HILL_HUGE 4
#define SMOOTH_SMALL 5
#define SMOOTH_LARGE 6
#define N_HILLS 7

char* hillNames[N_HILLS];

#define MAX_SUBCOMMANDS 10
enum { tlFile = 0, tlEdit = 1, tlView = 2, tlFlags = 3, tlTextures = 4, tlSize = 5 };
enum { subFileSave = 0, subFileQuit };
enum { subEditRegion = 10 };
enum { subViewBirdsEye = 20, subViewSwitch, subViewCrosshair, subViewClearWalls };
enum {
  subFlagsIce = 30,
  subFlagsAcid = 31,
  subFlagsSand = 32,
  subFlagsKill = 33,
  subFlagsTrampoline = 34,
  subFlagsNoGrid = 35,
  subFlagsTrack = 36,
  subFlagsFlat = 37,
};
enum { subTexturesNext = 40, subTexturesPrev, subTexturesRotate };

enum {
  editModeHeight = 0,
  editModeColor,
  editModeWater,
  editModeVelocity,
  editModeNoLines,
  editModeFeatures,
  nEditModes
};

/* These are all the codes for different areas of the screen that can be selected */
#define CODE_FROM_COMMAND(x) (10 + (x))
#define CODE_TO_COMMAND(x) ((x)-10)
/*
#define CODE_EDITMODE    100
*/
#define CODE_CELL_N 201
#define CODE_CELL_E 202
#define CODE_CELL_S 203
#define CODE_CELL_W 204
#define CODE_CELL_C 205
#define CODE_CELL_ALL 206

int switchViewpoint = 0, birdsEye = 0;
int markX = -1, markY;
int doAskSave = 0;

int selectedMenu = -1;

EditMode* EditMode::editMode;

void EditMode::init() {
  char* cMenuNames_i18n[N_SUBMENUS] = {
      /* TRANSLATORS: This is a list of all the menus in the map editor. */
      _("File"),    _("Edit"), _("Color"),  _("Flags"),
      _("Feature"), _("Move"), _("Window"), _("View"),
  };
  memcpy(cMenuNames, cMenuNames_i18n, sizeof(cMenuNames));

  const char* cMenuEntries_i18n[N_SUBMENUS][MAX_MENU_ENTRIES] = {
      /* TRANSLATORS: This is a list of all the submenus in the map
          editor, if the initial character is * or / then that character
          must be perserved as it is. */
      {_("New"), _("Open"), _("Close"), _("Save"), _("Exit"), _("Test level"), NULL},
      {_("Edit height"), _("Edit color"), _("Edit water"), _("Edit velocity"), _("Edit lines"),
       _("*<SPACE> Whole cell"), _("Upper corner"), _("Bottom corner"), _("Right corner"),
       _("Left corner"), _("Center"), _("Raise increment"), _("Lower increment"),
       _("*<SHIFT> reversed"), _("*<CTRL> walls"), NULL},
      {_("Inc. red"), _("Inc. green"), _("Inc. blue"), _("Inc. alpha"), NULL},
      {_("Ice"), _("Acid"), _("Sand"), _("Kill"), _("Bounce"), _("No grid"), _("Track"),
       _("Shade flat"), _("Texture"), _("ch. texture"), NULL},
      {_("Spike"), _("Small hill"), _("Medium hill"), _("Large hill"), _("Huge hill"),
       _("Smooth small"), _("Smooth large"), NULL},
      {_("/UP Move up"), _("/DOWN Move down"), _("/LEFT Move left"), _("/RIGHT Move right"),
       "/", _("Shift map up"), _("Shift map down"), _("Shift map left"), _("Shift map right"),
       _("*<SHIFT> x5"), "/", _("Set region marker"), _("Clear marker"), _("Copy region"),
       _("Paste region"), NULL},
      {_("Editor"), _("Toolbar"), _("Status"), NULL},
      {_("Birds's eye"), _("Rotate view"), NULL},
  };
  memcpy(cMenuEntries, cMenuEntries_i18n, sizeof(cMenuEntries));

  char* flagNames_i18n[NUM_FLAGS] = {_("Ice"),   _("Acid"),       _("Sand"),
                                     _("Kill"),  _("Trampoline"), _("No grid"),
                                     _("Track"), _("Shade flat")};
  memcpy(flagNames, flagNames_i18n, sizeof(flagNames));

  char* hillNames_i18n[N_HILLS] = {_("Spike"),       _("Small hill"), _("Medium hill"),
                                   _("Large hill"),  _("Huge hill"),  _("Small smooth"),
                                   _("Large smooth")};
  memcpy(hillNames, hillNames_i18n, sizeof(hillNames));

  /* Create the editmode object */
  new EditMode();
}

EditMode::EditMode() {
  map = NULL;
  strcpy(levelname, "");
  mapIsWritable = 0;

  /*
  char mapname[256];
  snprintf(mapname,sizeof(mapname)-1,"%s/.trackballs/levels/%s.map",getenv("HOME"),Settings::settings->specialLevel);
  */

  /* Load the selected level if we start with one selected */
  if (Settings::settings->doSpecialLevel) loadMap(Settings::settings->specialLevel);

  /* Some default values for parameters in the edit mode */
  scale = 0.5;
  rotation = 0.0;
  raise = 0.5;
  color[0] = color[1] = color[2] = 0.9;
  color[3] = 1.0;
  doSave = 0;
  cellClipboard = NULL;

  /* Setup the windows */
  menuWindow = new EMenuWindow();
  statusWindow = new EStatusWindow();
  quitWindow = new EQuitWindow();
  saveWindow = new ESaveWindow();
  closeWindow = new ECloseWindow();
  openWindow = new EOpenWindow();
  newWindow = new ENewWindow();
  currentEditMode = editModeHeight;

  /* Save a reference to self in both in the class variable. */
  EditMode::editMode = this;
}

EditMode::~EditMode() {
  if (map) saveMap();
  if (cellClipboard) delete[] cellClipboard;
  /* TODO. Delete all windows here */
}

void EditMode::loadMap(char* name) {
  char mapname[256];

  if (map) closeMap();

  // Store name of level to edit
  snprintf(levelname, sizeof(levelname), name);

  /* Set the pathname under which we will save the map */
  snprintf(pathname, sizeof(pathname), "%s/.trackballs/levels/%s.map", getenv("HOME"), name);

  // Default load the map from the home directory if existing (same as default pathname)
  snprintf(mapname, sizeof(mapname), "%s/.trackballs/levels/%s.map", getenv("HOME"), name);

  if (!fileExists(mapname))
    // Alternativly from the share directory
    snprintf(mapname, sizeof(mapname) - 1, "%s/levels/%s.map", SHARE_DIR, name);

  /* Note. not a problem here even if the map does not exists, it will use default values
   * instead */
  map = new Map(mapname);
  map->flags |= Map::flagFlashCenter;
  map->isTransparent = 1;
  x = (int)map->startPosition[0];
  y = (int)map->startPosition[1];

  /* Verify that we can save the map in the home directory */
  /* THIS TEST DOES NOT WORK PROPERLY - CREATES BROKEN MAPS. ALSO SECURITY RISK WITH SYMLINKS
   */
  /*
  snprintf(mapname,sizeof(mapname),"%s/.trackballs/levels/%s.map",getenv("HOME"),name);
  FILE *fp = fopen(mapname,"ab");
  if(!fp) mapIsWritable=0;
  else mapIsWritable=1;
  if(fp) fclose(fp);
  */
  mapIsWritable = 1;  // Best guess for now

  // test that we can save the map properly
  /* This is an old test if if could be saved in eg. the share directory
  if(!useHome) {
        FILE *fp = fopen(mapname,"ab");
        if(!fp) {
          useHome=1;
          snprintf(mapname,sizeof(mapname)-1,"%s/.trackballs/levels/%s.map",getenv("HOME"),Settings::settings->specialLevel);
        }
        else fclose(fp);
        }*/
}

void EditMode::closeMap() {
  if (map) {
    delete map;
    map = NULL;
  }
}

void EditMode::saveMap() {
  if (!map) return;

  char mapname[256];
  char str[256];

  snprintf(str, sizeof(str) - 1, "%s/.trackballs", getenv("HOME"));
  if (pathIsLink(str)) {
    fprintf(stderr, _("Error, %s/.trackballs is a symbolic link. Cannot save map\n"),
            getenv("HOME"));
    return;
  } else if (!pathIsDir(str))
    mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);

  snprintf(str, sizeof(str) - 1, "%s/.trackballs/levels", getenv("HOME"));
  if (pathIsLink(str)) {
    fprintf(stderr, _("Error, %s/.trackballs/levels is a symbolic link. Cannot save map\n"),
            getenv("HOME"));
    return;
  } else if (!pathIsDir(str))
    mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);

  snprintf(mapname, sizeof(mapname) - 1, "%s/.trackballs/levels/%s.map", getenv("HOME"),
           levelname);
  if (pathIsLink(str)) {
    fprintf(stderr,
            _("Error, %s/.trackballs/levels/%s.map is a symbolic link. Cannot save map\n"),
            getenv("HOME"), levelname);
    return;
  }
  // snprintf(mapname,sizeof(mapname)-1,"%s/levels/%s.map",SHARE_DIR,name); only if we allow
  // saving in the share dir
  printf(_("Saving map as '%s'\n"), mapname);

  map->save(mapname, x, y);
  doSave = 0;
  doAskSave = 0;

  /* Check if there already exists a script file for this map */
  snprintf(str, sizeof(str), "%s/levels/%s.scm", SHARE_DIR, levelname);
  if (!pathIsFile(str)) {
    snprintf(str, sizeof(str), "%s/.trackballs/levels/%s.scm", getenv("HOME"), levelname);
    if (!pathIsFile(str)) {
      /* No script file exists. Create a default one */
      if (pathIsLink(str)) {
        fprintf(stderr, _("Error, %s is a symbolic link. Cannot create default script file\n"),
                str);
        return;
      }
      printf("Creating default script file at: %s\n", str);
      FILE* fp = fopen(str, "wb");
      fprintf(fp, _(";;; Track: %s\n"), levelname);
      fprintf(fp, _(";;; This is the default script file for this track.\n"));
      fprintf(fp, _(";;; Read the documentation for the editor and look at the examples\n"));
      fprintf(fp, _(";;; to learn how to customize it\n\n"));
      fprintf(fp, "(set-track-name \"%s\")\n", levelname);
      fprintf(fp, "(set-author \"%s\")\n      ;; Enter your name here ", getenv("USER"));
      fprintf(fp, "(start-time 180)\n");
      fprintf(fp, "(set-start-position 250.5 250.5)\n");
      fprintf(fp,
              "(map-is-transparent #f)    ;; Set to #t if you have any water or other "
              "transparent parts\n");
      fprintf(fp, "(add-goal 249 250 #f \"\") ;; Add the name of the next level here\n");
      fprintf(fp, "(add-flag 248 250 50 1 0.1)\n");
      fclose(fp);
    }
  }
}

void EditMode::display() {
  double h;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40, (GLdouble)screenWidth / (GLdouble)max(screenHeight, 1), 1.0, 1e20);

  /* Setup openGL matrixes for the camera perspective */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  if (map) {
    h = map->cell(x, y).heights[Cell::CENTER];
  } else
    h = 0.0;

  if (!switchViewpoint)
    gluLookAt(x - 7.0, y - 7.0, (birdsEye ? 30.0 : 10.0) + h * 0.5, x, y, h, 0.0, 0.0, 1.0);
  else
    gluLookAt(x + 7.0, y + 7.0, (birdsEye ? 30.0 : 10.0) + h * 0.5, x, y, h, 0.0, 0.0, 1.0);

  /* Some standard GL settings needed */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  GLfloat lightDiffuse[] = {0.9, 0.9, 0.9, 0};
  GLfloat lightPosition[] = {-100.0, -50.0, 150.0, 0.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glShadeModel(GL_SMOOTH);

  /* Draw the map and the current mapcursor/selected region */
  int x0, y0;
  if (map) {
    map->draw(switchViewpoint | birdsEye, 0, x, y);
    map->draw(switchViewpoint | birdsEye, 1, x, y);
    /* If we have a clipboard selection then display where it will be pasted */
    if (cellClipboard) {
      for (x0 = x; x0 < x + cellClipboardWidth; x0++)
        for (y0 = y; y0 < y + cellClipboardHeight; y0++) map->drawFootprint(x0, y0, 1);
    } else if (markX >= 0) {
      /* Otherwise, if we have a marked region then display footprint over it */
      for (x0 = min(markX, x); x0 <= max(markX, x); x0++)
        for (y0 = min(markY, y); y0 <= max(markY, y); y0++) map->drawFootprint(x0, y0, 0);
    } else if (currentEditMode == editModeFeatures) {
      /* We have currently in the "features" edit mode, display foot
         print of selected feature */
      int footprintX, footprintY;
      switch (currentFeature) {
      case FEATURE_SPIKE:
        footprintX = 2;
        footprintY = 2;
        break;
      case FEATURE_SMALL_HILL:
        footprintX = 3;
        footprintY = 3;
        break;
      case FEATURE_MEDIUM_HILL:
        footprintX = 5;
        footprintY = 5;
        break;
      case FEATURE_LARGE_HILL:
        footprintX = 7;
        footprintY = 7;
        break;
      case FEATURE_HUGE_HILL:
        footprintX = 11;
        footprintY = 11;
        break;
      case FEATURE_SMALL_SMOOTH:
        footprintX = 5;
        footprintY = 5;
        break;
      case FEATURE_LARGE_SMOOTH:
        footprintX = 11;
        footprintY = 11;
        break;
      default:
        footprintX = 1;
        footprintY = 1;
      }
      for (x0 = x - footprintX / 2; x0 < x + (footprintX + 1) / 2; x0++)
        for (y0 = y - footprintY / 2; y0 < y + (footprintY + 1) / 2; y0++)
          map->drawFootprint(x0, y0, 0);
    }
    /* Finally, if no other case then just draw position of cursor */
    else
      map->drawFootprint(x, y, 0);
  }

  /*
  if(menuChoise == MENU_HILLS)
        switch(hill) {
        case HILL_SPIKE:
          map->drawFootprint(x,y);
          map->drawFootprint(x-1,y);
          map->drawFootprint(x,y-1);
          map->drawFootprint(x-1,y-1);
          break;
        case HILL_SMALL: for(x0=x-1;x0<=x+1;x0++) for(y0=y-1;y0<=y+1;y0++)
  map->drawFootprint(x0,y0); break;
        case HILL_MEDIUM: for(x0=x-2;x0<=x+2;x0++) for(y0=y-2;y0<=y+2;y0++)
  map->drawFootprint(x0,y0); break;
        case HILL_LARGE: for(x0=x-3;x0<=x+3;x0++) for(y0=y-3;y0<=y+3;y0++)
  map->drawFootprint(x0,y0); break;
        case HILL_HUGE: for(x0=x-5;x0<=x+5;x0++) for(y0=y-5;y0<=y+5;y0++)
  map->drawFootprint(x0,y0); break;
        case SMOOTH_SMALL: for(x0=x-2;x0<=x+2;x0++) for(y0=y-2;y0<=y+2;y0++)
  map->drawFootprint(x0,y0); break;
        case SMOOTH_LARGE: for(x0=x-4;x0<=x+4;x0++) for(y0=y-4;y0<=y+4;y0++)
  map->drawFootprint(x0,y0); break;
        }
  else
  */

  // displayFrameRate();

  Enter2DMode();
#ifdef FOOBARS

  snprintf(str, sizeof(str), _("Raise: %f"), raise);
  glColor3f(1.0, 1.0, 1.0);
  draw2DString(infoFont, str, 10, 20 + 0 * 18, 255, 255, 0);

  snprintf(str, sizeof(str), _("Pos: %d,%d"), x, y);
  draw2DString(infoFont, str, 10, 20 + 1 * 18, 255, 255, 0);
  if (menuChoise == MENU_HILLS) {
    snprintf(str, sizeof(str), _("Feature: %s"), hillNames[hill]);
    draw2DString(infoFont, str, 10, 20 + 2 * 18, 255, 255, 0);
  } else {
    snprintf(str, sizeof(str), _("Color %1.1f %1.1f %1.1f %1.1f"), color[0], color[1],
             color[2], color[3]);
    draw2DString(infoFont, str, 10, 20 + 2 * 18, 255, 255, 0);
    glColor4f(color[0], color[1], color[2], color[3]);
    glBegin(GL_POLYGON);
    glVertex2i(185, 17 + 1 * 18);
    glVertex2i(203, 17 + 1 * 18);
    glVertex2i(203, 13 + 2 * 18);
    glVertex2i(185, 13 + 2 * 18);
    glEnd();
  }
  glColor3f(1.0, 1.0, 1.0);

  for (i = 0; i < MENU_SIZE; i++) {
    if (menuChoise == i)
      draw2DString(infoFont, menuNames[i], 220, 20 + i * 18, 128, 255, 128);
    else
      draw2DString(infoFont, menuNames[i], 220, 20 + i * 18, 64, 192, 64);
  }

  for (i = 0; i < NUM_FLAGS; i++) {
    snprintf(str, sizeof(str), "%d. %s: %s", i + 1, flagNames[i],
             map->cell(x, y).flags & (1 << i) ? _("yes") : _("no"));
    draw2DString(infoFont, str, 350, 20 + i * 18, 128, 255, 128);
  }

  {
    Cell& c = map->cell(x, y);
    if (menuChoise == MENU_WATER) {
      draw2DString(infoFont, _("Water height"), screenWidth - 280, 40, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.waterHeights[3]);
      draw2DString(infoFont, str, screenWidth - 70, 20, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.waterHeights[0]);
      draw2DString(infoFont, str, screenWidth - 70, 60, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.waterHeights[2]);
      draw2DString(infoFont, str, screenWidth - 30, 40, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.waterHeights[1]);
      draw2DString(infoFont, str, screenWidth - 110, 40, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.waterHeights[Cell::CENTER]);
      draw2DString(infoFont, str, screenWidth - 70, 40, 255, 255, 0);
    } else if (menuChoise == MENU_VELOCITY) {
      snprintf(str, sizeof(str), "Velocity: %2.1f %2.1f", c.velocity[0], c.velocity[1]);
      draw2DString(infoFont, str, screenWidth - 260, 40, 255, 255, 0);
    } else {
      draw2DString(infoFont, "Height", screenWidth - 200, 40, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.heights[3]);
      draw2DString(infoFont, str, screenWidth - 70, 20, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.heights[0]);
      Draw2dstring(infoFont, str, screenWidth - 70, 60, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.heights[2]);
      draw2DString(infoFont, str, screenWidth - 30, 40, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.heights[1]);
      draw2DString(infoFont, str, screenWidth - 110, 40, 255, 255, 0);
      snprintf(str, sizeof(str), "%2.1f", c.heights[Cell::CENTER]);
      draw2DString(infoFont, str, screenWidth - 70, 40, 255, 255, 0);
    }
  }

#endif
  //  drawMenus();
  Leave2DMode();

  clearSelectionAreas();
  MyWindow::drawAll();
  drawMousePointer();
}

typedef enum { eInfo_Height = 0, eInfo_WaterHeight, eInfo_Textures, nInfos } eCellInfo;
int cellInfo = 0;
char* infoNames[nInfos] = {_("Cell height"), _("Cell water height"), _("Textures")};

void EditMode::mouseDown(int state, int x, int y) { MyWindow::mouseDownAll(state, x, y); }
void EditMode::doCommand(int command) {
  int mouseX, mouseY;
  int mouseState;
  mouseState = SDL_GetMouseState(&mouseX, &mouseY);

  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);

  // printf("Doing command: %d\n",command);

  /* File commands are handled first */
  switch (command) {
  case FILE_NEW:
    askNew();
    return;
  case FILE_OPEN:
    askOpen();
    return;
  case FILE_SAVE:
    askSave();
    return;
  case FILE_EXIT:
    askQuit();
    return;
  case FILE_CLOSE:
    askClose();
    return;
  case FILE_TEST:
    testLevel();
    return;
  }

  if (!map) /* If no map is opened only a limited subset of commands are available */
    return;

  /* Some variables needed for manipulating the map */
  Cell& cell = map->cell(x, y);
  int x1, y1;
  int xLow, xHigh, yLow, yHigh;
  if (markX >= 0) {
    xLow = min(x, markX);
    xHigh = max(x, markX);
    yLow = min(y, markY);
    yHigh = max(y, markY);
  } else {
    xLow = xHigh = x;
    yLow = yHigh = y;
  }

  switch (command) {
  case EDIT_HEIGHT:
  case EDIT_COLOR:
  case EDIT_WATER:
  case EDIT_VELOCITY:
  case EDIT_LINES:
    currentEditMode = command - EDIT_HEIGHT;
    break;
  case EDIT_RAISE_INCREMENT:
    scale = scale + 0.1;
    if (scale > 5.0) scale = 5.0;
    break;
  case EDIT_LOWER_INCREMENT:
    scale = scale - 0.1;
    if (scale < 0.1) scale = 0.1;
    break;

  case EDIT_UPPER:
    doCellAction(CODE_CELL_N, mouseState & SDL_BUTTON_LMASK ? 0 : 1);
    break;
  case EDIT_BOTTOM:
    doCellAction(CODE_CELL_S, mouseState & SDL_BUTTON_LMASK ? 0 : 1);
    break;
  case EDIT_LEFT:
    doCellAction(CODE_CELL_W, mouseState & SDL_BUTTON_LMASK ? 0 : 1);
    break;
  case EDIT_RIGHT:
    doCellAction(CODE_CELL_E, mouseState & SDL_BUTTON_LMASK ? 0 : 1);
    break;
  case EDIT_ALL:
    doCellAction(CODE_CELL_ALL, mouseState & SDL_BUTTON_LMASK ? 0 : 1);
    break;
  case EDIT_CENTER:
    doCellAction(CODE_CELL_C, mouseState & SDL_BUTTON_LMASK ? 0 : 1);
    break;

  case COLOR_RED:
    color[0] = color[0] + 0.05;
    if (color[0] > 1.01) color[0] = 0.0;
    break;
  case COLOR_GREEN:
    color[1] = color[1] + 0.05;
    if (color[1] > 1.01) color[1] = 0.0;
    break;
  case COLOR_BLUE:
    color[2] = color[2] + 0.05;
    if (color[2] > 1.01) color[2] = 0.0;
    break;
  case COLOR_ALPHA:
    color[3] = color[3] + 0.05;
    if (color[3] > 1.01) color[3] = 0.0;
    break;

  case FLAG_0:
  case FLAG_1:
  case FLAG_2:
  case FLAG_3:
  case FLAG_4:
  case FLAG_5:
  case FLAG_6:
  case FLAG_7: {
    int flag = command == FLAG_7 ? 1 << 11 : 1 << (command - FLAG_0);
    int onoff = cell.flags & flag ? 0 : flag;
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);
        c2.flags = (c2.flags & ~flag) | onoff;
        map->markCellUpdated(x1, y1);
      }
  } break;
  case FLAG_CH_TEXTURE: {
    int newTexture = mymod(cell.texture + 1 + (shift ? -1 : +1), numTextures + 1) - 1;
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        map->cell(x1, y1).texture = newTexture;
        map->markCellUpdated(x1, y1);
      }
  } break;

  case FEATURE_SPIKE:
  case FEATURE_SMALL_HILL:
  case FEATURE_MEDIUM_HILL:
  case FEATURE_LARGE_HILL:
  case FEATURE_HUGE_HILL:
  case FEATURE_SMALL_SMOOTH:
  case FEATURE_LARGE_SMOOTH:
    currentEditMode = editModeFeatures;
    currentFeature = command;
    break;

  case MOVE_UP:
    x += (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;
  case MOVE_DOWN:
    x -= (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;
  case MOVE_LEFT:
    y += (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;
  case MOVE_RIGHT:
    y -= (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;

  case MOVE_SET_MARKER:
    markX = x;
    markY = y;
    break;
  case MOVE_CLEAR_MARKER:
    markX = -1;
    if (cellClipboard) delete[] cellClipboard;
    cellClipboard = NULL;
    break;
  case MOVE_COPY_REGION:
    copyRegion();
    markX = -1;
    break;
  case MOVE_PASTE_REGION:
    markX = -1;
    pasteRegion();
    break;

  case WINDOW_EDITOR:
  case WINDOW_TOOLBAR:
  case WINDOW_STATUS:
    break;

  case VIEW_BIRD:
    birdsEye = birdsEye ? 0 : 1;
    break;
  case VIEW_ROTATE:
    switchViewpoint = switchViewpoint ? 0 : 1;
    break;

  default:
    /* TODO. Not implemented yet? */
    printf("Command %d not yet implemented\n", command);
    break;
  }
}

void EditMode::doCellAction(int code, int direction) {
  // printf("Cell action: %d %d\n",code,direction);

  int corner;
  int xLow, xHigh, yLow, yHigh;
  int i, j, x1, y1;

  int ctrl = SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL);
  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);

  /* Figure out which cell corner this is */
  switch (code) {
  case CODE_CELL_N:
    corner = Cell::NORTH + Cell::EAST;
    break;
  case CODE_CELL_E:
    corner = Cell::EAST + Cell::SOUTH;
    break;
  case CODE_CELL_S:
    corner = Cell::SOUTH + Cell::WEST;
    break;
  case CODE_CELL_W:
    corner = Cell::WEST + Cell::NORTH;
    break;
  case CODE_CELL_C:
    corner = Cell::CENTER;
    break;
  default:
    corner = Cell::CENTER;
  }

  if (markX >= 0) {
    xLow = min(x, markX);
    xHigh = max(x, markX);
    yLow = min(y, markY);
    yHigh = max(y, markY);
  } else {
    xLow = xHigh = x;
    yLow = yHigh = y;
  }
  Cell& c = map->cell(x, y);

  switch (currentEditMode) {
  case editModeHeight:
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);
        if (code == CODE_CELL_ALL) {
          for (i = 0; i < 5; i++) c2.heights[i] += (direction ? -scale : scale);
        } else {
          c2.heights[corner] += (direction ? -scale : scale);
          c2.heights[Cell::CENTER] += (direction ? -scale : scale) / 4;
        }
        map->markCellUpdated(x1, y1);
        map->markCellUpdated(x1 + 1, y1);
        map->markCellUpdated(x1, y1 + 1);
        map->markCellUpdated(x1 - 1, y1);
        map->markCellUpdated(x1, y1 - 1);
      }
    break;
  case editModeColor:
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);

        if (direction) {
          /* Pick up color */
          if (ctrl)
            for (i = 0; i < 4; i++) color[i] = c.wallColors[corner][i];
          else
            for (i = 0; i < 4; i++) color[i] = c.colors[corner][i];
        } else if (code == CODE_CELL_ALL) {
          /* Paint color */
          if (ctrl)
            for (j = 0; j < 4; j++)
              for (i = 0; i < 4; i++) c2.wallColors[j][i] = color[i];
          else
            for (j = 0; j < 5; j++)
              for (i = 0; i < 4; i++) c2.colors[j][i] = color[i];
        } else {
          /* Paint color */
          if (ctrl)
            for (i = 0; i < 4; i++) c2.wallColors[corner][i] = color[i];
          else
            for (i = 0; i < 4; i++) c2.colors[corner][i] = color[i];
        }
        map->markCellUpdated(x1, y1);
      }
    break;
  case editModeWater:
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);

        /* Fix for incorrectly initialized water heights */
        if (c2.waterHeights[corner] <= -100.0) {
          c2.waterHeights[corner] = c.heights[corner];
          c2.waterHeights[Cell::CENTER] = c.heights[Cell::CENTER];
        }

        if (code == CODE_CELL_ALL) {
          for (i = 0; i < 5; i++) c2.waterHeights[i] += (direction ? -scale : scale);
        } else {
          c2.waterHeights[corner] += direction ? -scale : scale;
          c2.waterHeights[Cell::CENTER] += (direction ? -scale : scale) / 4;
        }
        map->markCellUpdated(x1, y1);
      }
    break;

  case editModeVelocity:
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);
        if (code == CODE_CELL_N) c2.velocity[1] += direction ? 0.5 : 0.1;
        if (code == CODE_CELL_S) c2.velocity[1] -= direction ? 0.5 : 0.1;
        if (code == CODE_CELL_W) c2.velocity[0] -= direction ? 0.5 : 0.1;
        if (code == CODE_CELL_E) c2.velocity[0] += direction ? 0.5 : 0.1;
        map->markCellUpdated(x1, y1);
      }
    break;

  case editModeNoLines: {
    int flag = 0;
    if (code == CODE_CELL_N) flag = CELL_NOLINENORTH;
    if (code == CODE_CELL_S) flag = CELL_NOLINESOUTH;
    if (code == CODE_CELL_E) flag = CELL_NOLINEEAST;
    if (code == CODE_CELL_W) flag = CELL_NOLINEWEST;
    int onoff = c.flags & flag ? 0 : flag;
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);
        c2.flags = (c2.flags & ~flag) | onoff;
        map->markCellUpdated(x1, y1);
      }
  } break;

  case editModeFeatures: {
    switch (currentFeature) {
    case FEATURE_SPIKE:
      map->cell(x, y).heights[Cell::SOUTH + Cell::WEST] += shift ? -raise : raise;
      map->cell(x - 1, y).heights[Cell::SOUTH + Cell::EAST] += shift ? -raise : raise;
      map->cell(x, y - 1).heights[Cell::NORTH + Cell::WEST] += shift ? -raise : raise;
      map->cell(x - 1, y - 1).heights[Cell::NORTH + Cell::EAST] += shift ? -raise : raise;
      map->markCellUpdated(x, y);
      map->markCellUpdated(x - 1, y);
      map->markCellUpdated(x, y - 1);
      map->markCellUpdated(x - 1, y - 1);
      break;
    case FEATURE_SMALL_HILL:
      map->cell(x, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 1.2;
      for (i = 0; i < 4; i++) map->cell(x, y).heights[i] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y + 1).heights[Cell::SOUTH + Cell::EAST] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y + 1).heights[Cell::SOUTH + Cell::WEST] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y - 1).heights[Cell::NORTH + Cell::EAST] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y - 1).heights[Cell::NORTH + Cell::WEST] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y).heights[Cell::SOUTH + Cell::WEST] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y).heights[Cell::NORTH + Cell::WEST] += (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y).heights[Cell::SOUTH + Cell::EAST] += (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y).heights[Cell::NORTH + Cell::EAST] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y + 1).heights[Cell::SOUTH + Cell::WEST] +=
          (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y + 1).heights[Cell::SOUTH + Cell::EAST] +=
          (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y - 1).heights[Cell::NORTH + Cell::WEST] +=
          (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y - 1).heights[Cell::NORTH + Cell::EAST] +=
          (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x - 1, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x + 1, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x - 1, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x + 1, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->cell(x - 1, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->cell(x, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->cell(x, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      for (int k = x - 1; k <= x + 1; k++) {
        for (int j = y - 1; j <= y + 1; j++) { map->markCellUpdated(k, j); }
      }
      break;
    case FEATURE_MEDIUM_HILL:
      makeHill(2);
      break;
    case FEATURE_LARGE_HILL:
      makeHill(3);
      break;
    case FEATURE_HUGE_HILL:
      makeHill(5);
      break;
    case FEATURE_SMALL_SMOOTH:
      doSmooth(2);
      break;
    case FEATURE_LARGE_SMOOTH:
      doSmooth(5);
      break;
    }
  } break;
  }
}

void EditMode::key(int key) {
  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  /* TODO. Send any keys to the EOpenWindow?? */

  if (newWindow->isAttached()) {
    newWindow->key(key, mouseX, mouseY);
    return;
  }

  /* Closing */
  if (closeWindow->isAttached()) {
    if (key == 'y') closeWindow->yes();
    if (key == 'n') closeWindow->no();
    return;
  }

  /* Quiting */
  if (quitWindow->isAttached()) {
    if (key == 'y') quitWindow->yes();
    if (key == 'n') quitWindow->no();
    return;
  }

  /* Saving */
  if (saveWindow->isAttached()) {
    if (key == 'y') saveWindow->yes();
    if (key == 'n') saveWindow->no();
    return;
  }

  /* Handle cell actions specially */
  if (key == 'u') {
    doCellAction(CODE_CELL_N, shift ? 1 : 0);
    return;
  }
  if (key == 'k') {
    doCellAction(CODE_CELL_E, shift ? 1 : 0);
    return;
  }
  if (key == 'm') {
    doCellAction(CODE_CELL_S, shift ? 1 : 0);
    return;
  }
  if (key == 'h') {
    doCellAction(CODE_CELL_W, shift ? 1 : 0);
    return;
  }
  if (key == 'j') {
    doCellAction(CODE_CELL_C, shift ? 1 : 0);
    return;
  }
  if (key == ' ') {
    doCellAction(CODE_CELL_ALL, shift ? 1 : 0);
    return;
  }

  /* Move cursor */
  /*
  switch(key) {
  case SDLK_LEFT: y+=(switchViewpoint?-1:1)*(shift?5:1); break;
  case SDLK_RIGHT: y-=(switchViewpoint?-1:1)*(shift?5:1); break;
  case SDLK_UP: x+=(switchViewpoint?-1:1)*(shift?5:1); break;
  case SDLK_DOWN: x-=(switchViewpoint?-1:1)*(shift?5:1); break;
  }*/

  menuWindow->key(key, shift);

  return;

#ifdef FOOBARS

  if (menuChoise == MENU_HILLS && key >= '1' && key < '1' + N_HILLS) { hill = key - '1'; }

  /* These keys preloads rotated and scaled textures */
  /*
  if(menuChoise == MENU_TEXTURE && key >= '1' && key <= '8') {
        for(x1=xLow;x1<=xHigh;x1++)
          for(y1=yLow;y1<=yHigh;y1++) {
                Cell& c2=map->cell(x1,y1);
                for(north=0;north<2;north++)
                  for(east=0;east<2;east++) {
                        double angle=(2.0*M_PI * (key-'1'))/8.0;
                        c2.textureCoords[Cell::NORTH*north+Cell::EAST*east][0] =
                          ((x1+east)*cos(angle) + (y1+north)*sin(angle))*raise/4.0 *
  (shift?-1.:1.);
                        c2.textureCoords[Cell::NORTH*north+Cell::EAST*east][1] =
                          (- (x1+east)*sin(angle) + (y1+north)*cos(angle))*raise/4.0;
                  }
          }
          }*/

  int mx, my;
  switch (key) {
  case 'w':
  case SDLK_UP:
    /*
    if(menuChoise == MENU_TRANSLATE) {
      if(!switchViewpoint)
            for(mx=map->width-1;mx>0;mx--)
              for(my=0;my<map->height;my++)
                    map->cell(mx,my) = map->cell(mx-1,my);
      else
            for(mx=0;mx<map->width-1;mx++)
              for(my=0;my<map->height;my++)
                    map->cell(mx,my) = map->cell(mx+1,my);
    } else
      if (shift && ctrl)  {
        if (c.flags & CELL_NOLINENORTH)
          c.flags = ~((~c.flags) & CELL_NOLINENORTH);
        else
          c.flags |= CELL_NOLINENORTH; }
      else
      x+=(switchViewpoint?-1:1)*(shift?5:1);
    */
    break;
  case 's':
  case 'x':
  case SDLK_DOWN:
    if (menuChoise == MENU_TRANSLATE) {
      if (!switchViewpoint)
        for (mx = 0; mx < map->width - 1; mx++)
          for (my = 0; my < map->height; my++) map->cell(mx, my) = map->cell(mx + 1, my);
      else
        for (mx = map->width - 1; mx > 0; mx--)
          for (my = 0; my < map->height; my++) map->cell(mx, my) = map->cell(mx - 1, my);
    } else if (shift && ctrl) {
      if (c.flags & CELL_NOLINESOUTH)
        c.flags = ~((~c.flags) & CELL_NOLINESOUTH);
      else
        c.flags |= CELL_NOLINESOUTH;
    } else
      x -= (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;
  case 'a':
  case SDLK_LEFT:
    if (menuChoise == MENU_TRANSLATE) {
      if (!switchViewpoint)
        for (my = map->height; my > 0; my--)
          for (mx = 0; mx < map->width; mx++) map->cell(mx, my) = map->cell(mx, my - 1);
      else
        for (my = 0; my < map->height - 1; my++)
          for (mx = 0; mx < map->width; mx++) map->cell(mx, my) = map->cell(mx, my + 1);
    } else if (shift && ctrl) {
      if (c.flags & CELL_NOLINEWEST)
        c.flags = ~((~c.flags) & CELL_NOLINEWEST);
      else
        c.flags |= CELL_NOLINEWEST;
    } else
      y += (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;
  case 'd':
  case SDLK_RIGHT:
    if (menuChoise == MENU_TRANSLATE) {
      if (!switchViewpoint)
        for (my = 0; my < map->height - 1; my++)
          for (mx = 0; mx < map->width; mx++) map->cell(mx, my) = map->cell(mx, my + 1);
      else
        for (my = map->height; my > 0; my--)
          for (mx = 0; mx < map->width; mx++) map->cell(mx, my) = map->cell(mx, my - 1);
    } else if (shift && ctrl) {
      if (c.flags & CELL_NOLINEEAST)
        c.flags = ~((~c.flags) & CELL_NOLINEEAST);
      else
        c.flags |= CELL_NOLINEEAST;
    } else
      y -= (switchViewpoint ? -1 : 1) * (shift ? 5 : 1);
    break;
  case 'q':
    raise -= shift ? 1.0 : 0.1;
    break;
  case 'e':
    raise += shift ? 1.0 : 0.1;
    break;
  case 'b':
    birdsEye = birdsEye ? 0 : 1;
    break;
  case 't':
    if (map->flags & Map::flagTranslucent)
      map->flags &= ~Map::flagTranslucent;
    else
      map->flags |= Map::flagTranslucent;
    break;

  case 'c':
    if (map->flags & Map::flagShowCross)
      map->flags &= ~Map::flagShowCross;
    else
      map->flags |= Map::flagShowCross;
    break;

  case 'u':
  case 'h':
  case 'm':
  case 'k':

    if (menuChoise == MENU_TEXTURE) {
      for (x1 = xLow; x1 <= xHigh; x1++)
        for (y1 = yLow; y1 <= yHigh; y1++) {
          Cell& c2 = map->cell(x1, y1);
          for (i = 0; i < 4; i++) {
            if (key == 'u') c2.textureCoords[i][1] += shift ? 0.5 / 4. : 0.1 / 4.;
            if (key == 'm') c2.textureCoords[i][1] -= shift ? 0.5 / 4. : 0.1 / 4.;
            if (key == 'h') c2.textureCoords[i][0] -= shift ? 0.5 / 4. : 0.1 / 4.;
            if (key == 'k') c2.textureCoords[i][0] += shift ? 0.5 / 4. : 0.1 / 4.;
          }
        }
      break;
    }

    if (key == 'u')
      corner = Cell::NORTH + Cell::EAST;
    else if (key == 'h')
      corner = Cell::NORTH + Cell::WEST;
    else if (key == 'm')
      corner = Cell::SOUTH + Cell::WEST;
    else if (key == 'k')
      corner = Cell::SOUTH + Cell::EAST;

    else if (menuChoise == MENU_COLOR) {
      for (x1 = xLow; x1 <= xHigh; x1++)
        for (y1 = yLow; y1 <= yHigh; y1++) {
          Cell& c2 = map->cell(x1, y1);

          if (shift) /* Pick up color */
            if (ctrl)
              for (i = 0; i < 4; i++) color[i] = c2.wallColors[corner][i];
            else
              for (i = 0; i < 4; i++) color[i] = c2.colors[corner][i];
          else
              /* Write color */
              if (ctrl)
            for (i = 0; i < 4; i++) c2.wallColors[corner][i] = color[i];
          else
            for (i = 0; i < 4; i++) c2.colors[corner][i] = color[i];
        }
    }
    break;

  case SDLK_TAB:
    // smooth the cells if a selection is active
    /*
    if(menuChoise == MENU_HEIGHT) {

      if (ctrl) {
        c.heights[Cell::CENTER] =
               (c.heights[Cell::NORTH+Cell::WEST] +
                c.heights[Cell::SOUTH+Cell::WEST] +
                c.heights[Cell::NORTH+Cell::EAST] +
                c.heights[Cell::SOUTH+Cell::EAST]) / 4.;
        c.heights[Cell::NORTH+Cell::WEST] =
          c.heights[Cell::SOUTH+Cell::WEST] =
          c.heights[Cell::NORTH+Cell::EAST] =
          c.heights[Cell::SOUTH+Cell::EAST] =
              c.heights[Cell::CENTER];
        break;
      }

      if (markX == -1) {
            c.heights[Cell::CENTER] =
               (c.heights[Cell::NORTH+Cell::WEST] +
                c.heights[Cell::SOUTH+Cell::WEST] +
                c.heights[Cell::NORTH+Cell::EAST] +
                c.heights[Cell::SOUTH+Cell::EAST]) / 4.;
      break;
      }

      // smooth edges
      for(x1=xLow+1;x1<=xHigh;x1++)
            for(y1=yLow;y1<=yHigh-1;y1++) {
              Cell& cur=map->cell(x1,y1);
              Cell& ca1=map->cell(x1,y1+1);
              Cell& ca2=map->cell(x1-1,y1);
              Cell& ca3=map->cell(x1-1,y1+1);

              cur.heights[Cell::NORTH+Cell::WEST] =
              ca1.heights[Cell::SOUTH+Cell::WEST] =
              ca2.heights[Cell::NORTH+Cell::EAST] =
              ca3.heights[Cell::SOUTH+Cell::EAST] =
                 (cur.heights[Cell::NORTH+Cell::WEST] +
                  ca1.heights[Cell::SOUTH+Cell::WEST] +
                  ca2.heights[Cell::NORTH+Cell::EAST] +
                  ca3.heights[Cell::SOUTH+Cell::EAST]) / 4.;
            }

      // smooth the center of the cell
      for(x1=xLow;x1<=xHigh;x1++)
            for(y1=yLow;y1<=yHigh;y1++) {
              Cell& cur=map->cell(x1,y1);

              cur.heights[Cell::CENTER] =
                 (cur.heights[Cell::NORTH+Cell::WEST] +
                  cur.heights[Cell::SOUTH+Cell::WEST] +
                  cur.heights[Cell::NORTH+Cell::EAST] +
                  cur.heights[Cell::SOUTH+Cell::EAST]) / 4.;
            }

    }
    */
    break;

  case 'j':
    /*
    if(menuChoise == MENU_VELOCITY) {
      for(x1=xLow;x1<=xHigh;x1++)
            for(y1=yLow;y1<=yHigh;y1++) {
              Cell& c2=map->cell(x1,y1);
              c2.velocity[0] = c2.velocity[1] = 0.0;
            }
      break;
    }
    */

    if (menuChoise == MENU_HEIGHT) {
      for (x1 = xLow; x1 <= xHigh; x1++)
        for (y1 = yLow; y1 <= yHigh; y1++) {
          Cell& c2 = map->cell(x1, y1);
          if (ctrl)
            if (shift) {
              for (i = 0; i < 5; i++)
                c2.heights[i] = c.heights[Cell::CENTER];  // note the use of 'c' here!
            } else
              c2.heights[Cell::CENTER] =
                  (c2.heights[0] + c2.heights[1] + c2.heights[2] + c2.heights[3]) / 4.0;
          else
            c2.heights[Cell::CENTER] += (shift ? -raise : raise);
        }
    } else if (menuChoise == MENU_COLOR) {
      if (shift) /* Pick up color */
        for (i = 0; i < 4; i++) color[i] = c.colors[Cell::CENTER][i];
      else
          /* Write color */
          if (!ctrl)
        for (i = 0; i < 4; i++) c.colors[Cell::CENTER][i] = color[i];
    }

    break;
  case ' ':
    if (menuChoise == MENU_HILLS) {
      switch (hill) {
      case HILL_SPIKE:
        map->cell(x, y).heights[Cell::SOUTH + Cell::WEST] += shift ? -raise : raise;
        map->cell(x - 1, y).heights[Cell::SOUTH + Cell::EAST] += shift ? -raise : raise;
        map->cell(x, y - 1).heights[Cell::NORTH + Cell::WEST] += shift ? -raise : raise;
        map->cell(x - 1, y - 1).heights[Cell::NORTH + Cell::EAST] += shift ? -raise : raise;
        break;
      case HILL_SMALL:
        // makeHill(1);
        map->cell(x, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 1.2;
        for (i = 0; i < 4; i++) map->cell(x, y).heights[i] += (shift ? -raise : raise) * 1.0;
        map->cell(x, y + 1).heights[Cell::SOUTH + Cell::EAST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x, y + 1).heights[Cell::SOUTH + Cell::WEST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x, y - 1).heights[Cell::NORTH + Cell::EAST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x, y - 1).heights[Cell::NORTH + Cell::WEST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x + 1, y).heights[Cell::SOUTH + Cell::WEST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x + 1, y).heights[Cell::NORTH + Cell::WEST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x - 1, y).heights[Cell::SOUTH + Cell::EAST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x - 1, y).heights[Cell::NORTH + Cell::EAST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x + 1, y + 1).heights[Cell::SOUTH + Cell::WEST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x - 1, y + 1).heights[Cell::SOUTH + Cell::EAST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x + 1, y - 1).heights[Cell::NORTH + Cell::WEST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x - 1, y - 1).heights[Cell::NORTH + Cell::EAST] +=
            (shift ? -raise : raise) * 1.0;
        map->cell(x + 1, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
        map->cell(x - 1, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
        map->cell(x + 1, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
        map->cell(x - 1, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
        map->cell(x + 1, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
        map->cell(x - 1, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
        map->cell(x, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
        map->cell(x, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
        break;
      case HILL_MEDIUM:
        makeHill(2);
        break;
      case HILL_LARGE:
        makeHill(3);
        break;
      case HILL_HUGE:
        makeHill(5);
        break;
      case SMOOTH_SMALL:
        doSmooth(2);
        break;
      case SMOOTH_LARGE:
        doSmooth(4);
        break;
      }
      return;
    } else if (menuChoise == MENU_TEXTURE) {
      int newTexture = mymod(c.texture + 1 + (shift ? 1 : -1), numTextures + 1) - 1;
      for (x1 = xLow; x1 <= xHigh; x1++)
        for (y1 = yLow; y1 <= yHigh; y1++) {
          Cell& c2 = map->cell(x1, y1);
          c2.texture = newTexture;
        }
    }

    /* Pickup color works only for a specific corner of cell */
    if (menuChoise == MENU_COLOR && shift) break;

    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);
        if (menuChoise == MENU_HEIGHT)
          for (i = 0; i < 5; i++) c2.heights[i] += (shift ? -raise : raise);
        else if (menuChoise == MENU_WATER)
          for (i = 0; i < 5; i++) {
            if (c2.waterHeights[i] <= -100.0)
              c2.waterHeights[i] = c2.heights[i] + (shift ? -raise : raise);
            else
              c2.waterHeights[i] += (shift ? -raise : raise);
          }
        else if (menuChoise == MENU_COLOR) {
          if (ctrl)
            for (i = 0; i < 4; i++)
              for (j = 0; j < 4; j++) c2.wallColors[i][j] = color[j];
          else
            for (i = 0; i < 5; i++)
              for (j = 0; j < 4; j++) c2.colors[i][j] = color[j];
        }
      }
    break;
  case '1':
    color[0] += 0.1;
    if (color[0] > 1.0) color[0] = 0.0;
    break;
  case '2':
    color[1] += 0.1;
    if (color[1] > 1.0) color[1] = 0.0;
    break;
  case '3':
    color[2] += 0.1;
    if (color[2] > 1.0) color[2] = 0.0;
    break;
  case '4':
    color[3] += 0.1;
    if (color[3] > 1.0) color[3] = 0.0;
    break;
  }
#endif

  /*
case SDLK_DOWN: menuChoise++; if(menuChoise >= MENU_SIZE) menuChoise = 0; break;
case SDLK_UP: menuChoise--; if(menuChoise < 0) menuChoise = MENU_SIZE-1; break;*/
}
void EditMode::special(int key, int mx, int my) { printf("editmode: %d\n", key); }
void EditMode::idle(Real td) {
  int x, y;
  tickMouse(td);
  Uint8 mouseState = SDL_GetMouseState(&x, &y);
  MyWindow::mouseAll(mouseState, x, y);
  MyWindow::tickAll();
  time += td;
}
void EditMode::activated() {
  menuWindow->attach();
  statusWindow->attach();
}
void EditMode::deactivated() { MyWindow::resetWindows(); }
double steps(double v) { return ((int)(v / 0.1)) * 0.1; }
void EditMode::makeHill(int radius) {
  int mx, my;
  int diameter = radius * 2 + 1;
  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);

  for (mx = -radius; mx <= radius; mx++)
    for (my = -radius; my <= radius; my++) {
      map->cell(x + mx, y + my).heights[Cell::CENTER] +=
          ((int)(sin(1. * M_PI * (mx + radius + 0.5) / diameter) *
                 sin(1. * M_PI * (my + radius + 0.5) / diameter) * (shift ? -raise : raise) *
                 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::NORTH + Cell::EAST] +=
          ((int)(sin(1. * M_PI * (mx + radius + 1.0) / diameter) *
                 sin(1. * M_PI * (my + radius + 1.0) / diameter) * (shift ? -raise : raise) *
                 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::NORTH + Cell::WEST] +=
          ((int)(sin(1. * M_PI * (mx + radius) / diameter) *
                 sin(1. * M_PI * (my + radius + 1.0) / diameter) * (shift ? -raise : raise) *
                 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::SOUTH + Cell::EAST] +=
          ((int)(sin(1. * M_PI * (mx + radius + 1.0) / diameter) *
                 sin(1. * M_PI * (my + radius) / diameter) * (shift ? -raise : raise) * 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::SOUTH + Cell::WEST] +=
          ((int)(sin(1. * M_PI * (mx + radius) / diameter) *
                 sin(1. * M_PI * (my + radius) / diameter) * (shift ? -raise : raise) * 10.)) *
          .1;
      map->markCellUpdated(x + mx, y + my);
    }
}
void EditMode::doSmooth(int radius) {
  int mx, my, i;
  int diameter = radius * 2 + 1;
  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);
  double avgHeight = 0.0;
  int north, east;

  for (mx = -radius; mx <= radius; mx++)
    for (my = -radius; my <= radius; my++) {
      Cell& c1 = map->cell(x + mx, y + my);
      for (i = 0; i < 5; i++) avgHeight += c1.heights[i];
      map->markCellUpdated(x + mx, y + my);
    }
  avgHeight = avgHeight / (5. * diameter * diameter);
  for (mx = -radius; mx <= radius; mx++)
    for (my = -radius; my <= radius; my++) {
      Cell& c = map->cell(x + mx, y + my);
      double s1 = sin(1. * M_PI * (mx + radius + 0.5) / diameter) *
                  sin(1. * M_PI * (my + radius + 0.5) / diameter) * (shift ? -raise : raise);
      c.heights[Cell::CENTER] = steps(c.heights[Cell::CENTER] * (1. - s1) + avgHeight * s1);
      for (north = 0; north < 2; north++)
        for (east = 0; east < 2; east++) {
          double s = sin(1. * M_PI * (my + radius + 1.0 * north) / diameter) *
                     sin(1. * M_PI * (mx + radius + 1.0 * east) / diameter) *
                     (shift ? -raise : raise);
          c.heights[Cell::NORTH * north + Cell::EAST * east] = steps(
              c.heights[Cell::NORTH * north + Cell::EAST * east] * (1. - s) + avgHeight * s);
        }
      map->markCellUpdated(x + mx, y + my);
    }
}
void EditMode::copyRegion() {
  if (markX < 0) return;
  int x0 = min(x, markX);
  int x1 = max(x, markX);
  int y0 = min(y, markY);
  int y1 = max(y, markY);
  int width = x1 - x0 + 1;
  int height = y1 - y0 + 1;
  if (cellClipboard) delete[] cellClipboard;
  cellClipboard = new Cell[width * height];
  cellClipboardWidth = width;
  cellClipboardHeight = height;
  int x, y;
  for (x = 0; x < width; x++)
    for (y = 0; y < height; y++) { cellClipboard[x + y * width] = map->cell(x + x0, y + y0); }
}
void EditMode::pasteRegion() {
  if (!cellClipboard) return;
  int cx, cy;
  for (cx = 0; cx < cellClipboardWidth; cx++)
    for (cy = 0; cy < cellClipboardHeight; cy++) {
      Cell& toCell = map->cell(cx + x, cy + y);
      Cell& fromCell = cellClipboard[cx + cy * cellClipboardWidth];
      /* We cannot just do a memcpy here since we need to preserve displaylists and other meta
       * data */
      memcpy(toCell.velocity, fromCell.velocity, sizeof(toCell.velocity));
      memcpy(toCell.heights, fromCell.heights, sizeof(toCell.heights));
      memcpy(toCell.colors, fromCell.colors, sizeof(toCell.colors));
      memcpy(toCell.wallColors, fromCell.wallColors, sizeof(toCell.wallColors));
      memcpy(toCell.waterHeights, fromCell.waterHeights, sizeof(toCell.waterHeights));
      memcpy(toCell.textureCoords, fromCell.textureCoords, sizeof(toCell.textureCoords));
      toCell.sunken = fromCell.sunken;
      toCell.flags = fromCell.flags;
      toCell.texture = fromCell.texture;
      map->markCellUpdated(cx + x, cy + y);
      map->markCellUpdated(cx + x - 1, cy + y);
      map->markCellUpdated(cx + x, cy + y + 1);
      map->markCellUpdated(cx + x - 1, cy + y);
      map->markCellUpdated(cx + x, cy + y + 1);
    }
}

void EditMode::closeAllDialogWindows() {
  newWindow->remove();
  quitWindow->remove();
  saveWindow->remove();
  closeWindow->remove();
  openWindow->remove();
}
void EditMode::askQuit() {
  closeAllDialogWindows();
  quitWindow->attach();
}
void EditMode::askSave() {
  closeAllDialogWindows();
  saveWindow->attach();
}
void EditMode::askClose() {
  closeAllDialogWindows();
  closeWindow->attach();
}
void EditMode::askOpen() {
  closeAllDialogWindows();
  openWindow->attach();
  openWindow->refreshMapList();
}
void EditMode::askNew() {
  closeAllDialogWindows();
  newWindow->attach();
}

void EditMode::testLevel() {
  if (SetupMode::setupMode) {
    snprintf(Settings::settings->specialLevel, sizeof(Settings::settings->specialLevel),
             levelname);
    Settings::settings->doSpecialLevel = 1;
    saveMap();
    GameMode::activate(SetupMode::setupMode);
  }
}
