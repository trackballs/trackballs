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

#include "editMode.h"

#include "editMode_codes.h"
#include "editWindows.h"
#include "game.h"
#include "mainMode.h"
#include "map.h"
#include "menusystem.h"
#include "settings.h"
#include "setupMode.h"
#include "smartTrigger.h"
#include "trigger.h"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <string.h>
#include <sys/stat.h>

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
    /* Repair */
    "1234567890",
    /* Move */
    "UDLR*******rcxv",
    /* Window */
    "***",
    /* View */
    "brlc",
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
  subRepairCellCont = 50,
  subRepairWaterCont,
  subRepairColorCont,
  subRepairCellCenters,
  subRepairWaterCenters,
  subRepairColorCenters,
  subRepairCellRound,
  subRepairWaterRound,
  subRepairColorRound,
  subRepairVelRound,
};

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

const int viewPoints[4][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

EditMode* EditMode::editMode = NULL;

void EditMode::loadStrings() {
  char* cMenuNames_i18n[N_SUBMENUS] = {
      /* TRANSLATORS: This is a list of all the menus in the map editor. */
      _("File"),   _("Edit"), _("Color"),  _("Flags"), _("Feature"),
      _("Repair"), _("Move"), _("Window"), _("View"),
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
      {_("Cell cont."), _("Water cont."), _("Color cont."), _("Cell center"),
       _("Water center"), _("Color center"), _("Cell round"), _("Water round"),
       _("Color round"), _("Vel. round"), NULL},
      {_("/UP Move up"), _("/DOWN Move down"), _("/LEFT Move left"), _("/RIGHT Move right"),
       "/", _("Shift map up"), _("Shift map down"), _("Shift map left"), _("Shift map right"),
       _("*<SHIFT> x5"), "/", _("Set region marker"), _("Clear marker"), _("Copy region"),
       _("Paste region"), NULL},
      {_("Editor"), _("Toolbar"), _("Status"), NULL},
      {_("Birds's eye"), _("Rotate view"), _("Load entities"), _("Clear entities"), NULL},
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
}

EditMode* EditMode::init() {
  if (!editMode) {
    loadStrings();
    editMode = new EditMode;
  }
  return editMode;
}
void EditMode::cleanup() {
  if (editMode) delete editMode;
}

EditMode::EditMode() {
  map = NULL;
  game = NULL;
  memset(levelname, 0, sizeof(levelname));
  mapIsWritable = 0;

  /* Load the selected level if we start with one selected */
  if (Settings::settings->doSpecialLevel) loadMap(Settings::settings->specialLevel);

  /* Some default values for parameters in the edit mode */
  scale = 0.5;
  rotation = 0.0;
  raise = 0.5;
  color = SRGBColor(0.9, 0.9, 0.9, 1.0);
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
  resizeWindows();

  /* Save a reference to self in both in the class variable. */
  EditMode::editMode = this;
}

EditMode::~EditMode() {
  if (map) saveMap();
  if (cellClipboard) delete[] cellClipboard;
  /* TODO. Delete all windows here */
}

void EditMode::loadMap(char* name) {
  char mapname[512];

  if (map) closeMap();

  // Store name of level to edit
  strncpy(levelname, name, sizeof(levelname));
  levelname[255] = '\0';

  /* Set the pathname under which we will save the map */
  snprintf(pathname, sizeof(pathname), "%s/levels/%s.map", effectiveLocalDir, name);

  // Default load the map from the home directory if existing (same as default pathname)
  snprintf(mapname, sizeof(mapname), "%s/levels/%s.map", effectiveLocalDir, name);

  if (!fileExists(mapname))
    // Alternativly from the share directory
    snprintf(mapname, sizeof(mapname) - 1, "%s/levels/%s.map", effectiveShareDir, name);

  /* Note. not a problem here even if the map does not exists, it will use default values
   * instead */
  map = new Map(mapname);
  map->flags |= Map::flagFlashCenter;
  x = (int)map->startPosition[0];
  y = (int)map->startPosition[1];
  game = NULL;

  mapIsWritable = 1;  // Best guess for now
}

void EditMode::closeMap() {
  if (map) {
    delete map;
    map = NULL;
  }
  if (game) {
    delete game;
    game = NULL;
  }
}

void EditMode::saveMap() {
  if (!map) return;

  char mapname[768];
  char str[768];

  snprintf(str, sizeof(str) - 1, "%s/levels", effectiveLocalDir);
  if (pathIsLink(str)) {
    warning("Error, %s/levels is a symbolic link. Cannot save map", effectiveLocalDir);
    return;
  } else if (!pathIsDir(str))
    mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);

  snprintf(mapname, sizeof(mapname) - 1, "%s/levels/%s.map", effectiveLocalDir, levelname);
  if (pathIsLink(str)) {
    warning("Error, %s/levels/%s.map is a symbolic link. Cannot save map", effectiveLocalDir,
            levelname);
    return;
  }

  map->save(mapname, x, y);
  doSave = 0;
  doAskSave = 0;

  /* Check if there already exists a script file for this map */
  snprintf(str, sizeof(str), "%s/levels/%s.scm", effectiveShareDir, levelname);
  if (!pathIsFile(str)) {
    snprintf(str, sizeof(str), "%s/levels/%s.scm", effectiveLocalDir, levelname);
    if (!pathIsFile(str)) {
      /* No script file exists. Create a default one */
      if (pathIsLink(str)) {
        warning("Error, %s is a symbolic link. Cannot create default script file", str);
        return;
      }
      printf("Creating default script file at: %s\n", str);
      FILE* fp = fopen(str, "wb");
      fprintf(fp, ";;; %s %s\n", _("Track:"), levelname);
      fprintf(fp, ";;; %s\n", _("This is the default script file for this track."));
      fprintf(fp, ";;; %s\n",
              _("Read the documentation for the editor and look at the examples"));
      fprintf(fp, ";;; %s\n\n", _("to learn how to customize it"));
      fprintf(fp, "(set-track-name \"%s\")\n", levelname);
      fprintf(fp, "(set-author \"%s\")\n      ;; %s\n", username, _("Enter your name here"));
      fprintf(fp, "(start-time 180)\n");
      fprintf(fp, "(set-start-position 250.5 250.5)\n");
      fprintf(fp, "(add-goal 249.0 250.0 #f \"\") ;; %s\n",
              _("Add the name of the next level here"));
      fprintf(fp, "(add-flag 248.0 250.0 50 #t 0.1)\n");
      fclose(fp);
    }
  }
}

void EditMode::display() {
  double h;
  if (map) {
    h = map->cell(x, y).heights[Cell::CENTER];
  } else
    h = 0.0;

  /* Show extra cell flag overlay */
  activeView.show_flag_state = true;

  /* configure lighting */
  activeView.fog_enabled = 0;
  MainMode::setupLighting(NULL, false);

  /* Setup matrixes for the camera perspective */
  perspectiveMatrix(40, (GLdouble)screenWidth / (GLdouble)std::max(screenHeight, 1), 0.1, 200,
                    activeView.projection);

  lookAtMatrix(x - viewPoints[switchViewpoint][0] * 7.0,
               y - viewPoints[switchViewpoint][1] * 7.0, (birdsEye ? 30.0 : 10.0) + h * 0.5, x,
               y, h, 0.0, 0.0, 1.0, activeView.modelview);

  activeView.day_mode = true;
  markViewChanged();
  /* Shadow map rendering returns active modelview/projection to orig state */
  if (map) {
    if (Settings::settings->doShadows) {
      Coord3d focus((double)x, (double)y, map->getHeight(x, y));
      renderShadowCascade(focus, map, game);
      activeView.use_shadows = true;
    } else {
      activeView.use_shadows = false;
    }
  }

  /* Some standard GL settings needed */
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glViewport(0, 0, screenWidth, screenHeight);
  glClearColor(0., 0., 0., 1.);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Draw the map and the current mapcursor/selected region */
  if (map) {
    Coord3d map_focus((Real)x, (Real)y, map->cell(x, y).heights[Cell::CENTER]);
    map->draw(0, map_focus, time);
    map->drawLoop(0, 0, map->width - 1, map->height - 1, 0);
    if (game) {
      /* Indicate start position and trigger object locations */
      for (size_t i = 0; i < game->hooks[Role_GameHook].size(); i++) {
        GameHook* hook = game->hooks[Role_GameHook][i];
        if (hook->invalid) continue;

        SmartTrigger* smart_trigger = dynamic_cast<SmartTrigger*>(hook);
        Trigger* dumb_trigger = dynamic_cast<Trigger*>(hook);
        if (smart_trigger) {
          map->drawSpotRing(smart_trigger->x, smart_trigger->y, smart_trigger->radius, 2);
        }
        if (dumb_trigger) {
          map->drawSpotRing(dumb_trigger->x, dumb_trigger->y, dumb_trigger->radius, 1);
        }
      }

      map->drawSpotRing(map->startPosition[0], map->startPosition[1], 0.5, 0);
      game->draw();
    }
    map->draw(1, map_focus, time);
    activeView.show_flag_state = false;

    /* If we have a clipboard selection then display where it will be pasted */
    if (cellClipboard) {
      map->drawFootprint(x, y, x + cellClipboardWidth - 1, y + cellClipboardHeight - 1, 1);
    } else if (markX >= 0) {
      /* Otherwise, if we have a marked region then display footprint over it */
      map->drawFootprint(std::min(markX, x), std::min(markY, y), std::max(markX, x),
                         std::max(markY, y), 0);
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
      map->drawFootprint(x - footprintX / 2, y - footprintY / 2, x + (footprintX + 1) / 2 - 1,
                         y + (footprintY + 1) / 2 - 1, 0);
    }
    /* Finally, if no other case then just draw position of cursor */
    else
      map->drawFootprint(x, y, x, y, 0);
  }

  clearSelectionAreas();
  Enter2DMode();
  MyWindow::drawAll();
  drawMousePointer();
  Leave2DMode();
}

typedef enum { eInfo_Height = 0, eInfo_WaterHeight, eInfo_Textures, nInfos } eCellInfo;
int cellInfo = 0;
char* infoNames[nInfos] = {_("Cell height"), _("Cell water height"), _("Textures")};

int roundint(double f) {
  /* std::round is C++11. */
  return std::floor(f + 0.5);
}

void EditMode::mouseDown(int state, int x, int y) { MyWindow::mouseDownAll(state, x, y); }
void EditMode::doCommand(int command) {
  int mouseX, mouseY;
  int mouseState;
  mouseState = SDL_GetMouseState(&mouseX, &mouseY);

  int shift = SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT);
  int ctrl = SDL_GetModState() & (KMOD_LCTRL | KMOD_RCTRL);

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
    xLow = std::min(x, markX);
    xHigh = std::max(x, markX);
    yLow = std::min(y, markY);
    yHigh = std::max(y, markY);
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
    if (color.w[0] >= MAX_SRGB_VAL)
      color.w[0] = 0;
    else
      color.w[0] = std::min(MAX_SRGB_VAL, color.w[0] + MAX_SRGB_VAL / 20);
    break;
  case COLOR_GREEN:
    if (color.w[1] >= MAX_SRGB_VAL)
      color.w[1] = 0;
    else
      color.w[1] = std::min(MAX_SRGB_VAL, color.w[1] + MAX_SRGB_VAL / 20);
    break;
  case COLOR_BLUE:
    if (color.w[2] >= MAX_SRGB_VAL)
      color.w[2] = 0;
    else
      color.w[2] = std::min(MAX_SRGB_VAL, color.w[2] + MAX_SRGB_VAL / 20);
    break;
  case COLOR_ALPHA:
    if (color.w[3] >= MAX_SRGB_VAL)
      color.w[3] = 0;
    else
      color.w[3] = std::min(MAX_SRGB_VAL, color.w[3] + MAX_SRGB_VAL / 20);
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
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, false);
  } break;
  case FLAG_CH_TEXTURE: {
    int newTexture = mymod(cell.texture + 1 + (shift ? -1 : +1), numTextures + 1) - 1;
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) { map->cell(x1, y1).texture = newTexture; }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, false);
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

  case REPAIR_CELL_CONT:
  case REPAIR_WATER_CONT:
  case REPAIR_COLOR_CONT: {
    /* given four cells at these relative positions, seq. corners match up */
    int dir[4][2] = {{1, 1}, {1, 0}, {0, 1}, {0, 0}};

    int wx = (xHigh - xLow + 1), wy = (yHigh - yLow + 1);
    float* hbuf = new float[wx * wy * 4];
    for (int p = 0; p < (command == REPAIR_COLOR_CONT ? 4 : 1); p++) {
      /* Read changes into a buffer */
      for (x1 = xLow; x1 <= xHigh; x1++)
        for (y1 = yLow; y1 <= yHigh; y1++) {
          /* continuity enforces that corners match the neighbor average,
           * or internal average if there are no neighbors*/
          for (int k = 0; k < 4; k++) {
            float external = 0.;
            float internal = 0.;
            int extc = 0;
            for (int m = 0; m < 4; m++) {
              int altx = x1 - dir[k][0] + dir[m][0], alty = y1 - dir[k][1] + dir[m][1];
              float val;
              switch (command) {
              default:
              case REPAIR_CELL_CONT:
                val = map->cell(altx, alty).heights[m];
                break;
              case REPAIR_WATER_CONT:
                val = map->cell(altx, alty).waterHeights[m];
                break;
              case REPAIR_COLOR_CONT:
                val = map->cell(altx, alty).colors[m].w[p];
              }
              if (xLow <= altx && altx <= xHigh && yLow <= alty && alty <= yHigh) {
                internal += val;
              } else {
                external += val;
                extc++;
              }
            }
            float target;
            if (extc == 0) {
              target = internal / 4;
            } else {
              target = external / extc;
            }
            hbuf[4 * ((x1 - xLow) * wy + (y1 - yLow)) + k] = target;
          }
        }
      /* Apply changes to cells */
      for (x1 = xLow; x1 <= xHigh; x1++)
        for (y1 = yLow; y1 <= yHigh; y1++) {
          Cell& c = map->cell(x1, y1);
          float dcent = 0;
          for (int k = 0; k < 4; k++) {
            float target = hbuf[4 * ((x1 - xLow) * wy + (y1 - yLow)) + k];
            switch (command) {
            default:
            case REPAIR_CELL_CONT:
              dcent += target - c.heights[k];
              c.heights[k] = target;
              break;
            case REPAIR_WATER_CONT:
              dcent += target - c.waterHeights[k];
              c.waterHeights[k] = target;
              break;
            case REPAIR_COLOR_CONT:
              dcent += target - c.colors[k].w[p];
              c.colors[k].w[p] = target;
            }
          }
          switch (command) {
          default:
          case REPAIR_CELL_CONT:
            c.heights[4] += dcent / 4;
            break;
          case REPAIR_WATER_CONT:
            c.waterHeights[4] += dcent / 4;
            break;
          case REPAIR_COLOR_CONT:
            c.colors[4].w[p] += dcent / 4;
          }
        }
    }
    delete[] hbuf;
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, command == REPAIR_CELL_CONT);
  } break;
  case REPAIR_CELL_CENTERS:
  case REPAIR_WATER_CENTERS:
  case REPAIR_COLOR_CENTERS:
  case REPAIR_CELL_ROUND:
  case REPAIR_WATER_ROUND:
  case REPAIR_COLOR_ROUND:
  case REPAIR_VEL_ROUND: {
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c = map->cell(x1, y1);
        switch (command) {
        /* cell center repair sets the center as average of corners */
        case REPAIR_CELL_CENTERS:
          c.heights[Cell::CENTER] =
              0.25 * (c.heights[0] + c.heights[1] + c.heights[2] + c.heights[3]);
          break;
        case REPAIR_WATER_CENTERS:
          c.waterHeights[Cell::CENTER] = 0.25 * (c.waterHeights[0] + c.waterHeights[1] +
                                                 c.waterHeights[2] + c.waterHeights[3]);
          break;
        case REPAIR_COLOR_CENTERS:
          for (int k = 0; k < 4; k++) {
            c.colors[Cell::CENTER].w[k] =
                (c.colors[0].w[k] + c.colors[1].w[k] + c.colors[2].w[k] + c.colors[3].w[k]) /
                4;
          }
          break;

        /* For rounding routines the central cell has 4x resolution */
        case REPAIR_CELL_ROUND:
          for (int k = 0; k < 4; k++) {
            c.heights[k] = scale * roundint(c.heights[k] / scale);
          }
          c.heights[Cell::CENTER] =
              0.25 * scale * roundint(4. * c.heights[Cell::CENTER] / scale);
          break;
        case REPAIR_WATER_ROUND:
          for (int k = 0; k < 4; k++) {
            c.waterHeights[k] = scale * roundint(c.waterHeights[k] / scale);
          }
          c.waterHeights[Cell::CENTER] =
              0.25 * scale * roundint(4. * c.waterHeights[Cell::CENTER] / scale);
          break;
        case REPAIR_COLOR_ROUND:
          for (int m = 0; m < 4; m++) {
            float mscale = MAX_SRGB_VAL / 20;
            for (int k = 0; k < 4; k++) {
              c.colors[k].w[m] = mscale * roundint(c.colors[k].w[m] / mscale);
            }
            c.colors[Cell::CENTER].w[m] =
                0.25 * mscale * roundint(4. * c.colors[Cell::CENTER].w[m] / mscale);
          }
          break;
        case REPAIR_VEL_ROUND:
          c.velocity[0] = scale * roundint(c.velocity[0] / scale);
          c.velocity[1] = scale * roundint(c.velocity[1] / scale);
          break;
        }
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, command == REPAIR_CELL_ROUND);
  } break;

  case MOVE_UP:
    ((switchViewpoint % 2) ? y : x) +=
        viewPoints[switchViewpoint][1] * (ctrl ? 20 : (shift ? 5 : 1));
    break;
  case MOVE_DOWN:
    ((switchViewpoint % 2) ? y : x) -=
        viewPoints[switchViewpoint][1] * (ctrl ? 20 : (shift ? 5 : 1));
    break;
  case MOVE_LEFT:
    ((switchViewpoint % 2) ? x : y) +=
        viewPoints[switchViewpoint][0] * (ctrl ? 20 : (shift ? 5 : 1));
    break;
  case MOVE_RIGHT:
    ((switchViewpoint % 2) ? x : y) -=
        viewPoints[switchViewpoint][0] * (ctrl ? 20 : (shift ? 5 : 1));
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
    switchViewpoint = (switchViewpoint + 1) % 4;
    break;
  case VIEW_CLEAR_ENTITIES:
    if (game) delete game;
    game = NULL;
    break;
  case VIEW_LOAD_ENTITIES: {
    /* reset entities and state */
    if (game) delete game;
    game = new Game(map, levelname);
  } break;

  default:
    /* TODO. Not implemented yet? */
    warning("Command %d not yet implemented", command);
    break;
  }
}

void EditMode::doCellAction(int code, int direction) {
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
    xLow = std::min(x, markX);
    xHigh = std::max(x, markX);
    yLow = std::min(y, markY);
    yHigh = std::max(y, markY);
  } else {
    xLow = xHigh = x;
    yLow = yHigh = y;
  }
  if (!map) return;
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
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, true);
    break;
  case editModeColor:
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);

        if (direction) {
          /* Pick up color */
          if (ctrl)
            color = c.wallColors[corner];
          else
            color = c.colors[corner];
        } else if (code == CODE_CELL_ALL) {
          /* Paint color */
          if (ctrl)
            for (j = 0; j < 4; j++) c2.wallColors[j] = color;
          else
            for (j = 0; j < 5; j++) c2.colors[j] = color;
        } else {
          /* Paint color */
          if (ctrl)
            c2.wallColors[corner] = color;
          else
            c2.colors[corner] = color;
        }
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, false);
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
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, false);
    break;

  case editModeVelocity:
    for (x1 = xLow; x1 <= xHigh; x1++)
      for (y1 = yLow; y1 <= yHigh; y1++) {
        Cell& c2 = map->cell(x1, y1);
        if (code == CODE_CELL_N) c2.velocity[1] += direction ? 0.5 : 0.1;
        if (code == CODE_CELL_S) c2.velocity[1] -= direction ? 0.5 : 0.1;
        if (code == CODE_CELL_W) c2.velocity[0] -= direction ? 0.5 : 0.1;
        if (code == CODE_CELL_E) c2.velocity[0] += direction ? 0.5 : 0.1;
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, false);
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
      }
    map->markCellsUpdated(xLow, yLow, xHigh, yHigh, false);
  } break;

  case editModeFeatures: {
    switch (currentFeature) {
    case FEATURE_SPIKE:
      map->cell(x, y).heights[Cell::SW] += shift ? -raise : raise;
      map->cell(x - 1, y).heights[Cell::SE] += shift ? -raise : raise;
      map->cell(x, y - 1).heights[Cell::NW] += shift ? -raise : raise;
      map->cell(x - 1, y - 1).heights[Cell::NE] += shift ? -raise : raise;
      map->markCellsUpdated(x - 1, y - 1, x, y, false);
      break;
    case FEATURE_SMALL_HILL:
      map->cell(x, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 1.2;
      for (i = 0; i < 4; i++) map->cell(x, y).heights[i] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y + 1).heights[Cell::SE] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y + 1).heights[Cell::SW] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y - 1).heights[Cell::NE] += (shift ? -raise : raise) * 1.0;
      map->cell(x, y - 1).heights[Cell::NW] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y).heights[Cell::SW] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y).heights[Cell::NW] += (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y).heights[Cell::SE] += (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y).heights[Cell::NE] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y + 1).heights[Cell::SW] += (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y + 1).heights[Cell::SE] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y - 1).heights[Cell::NW] += (shift ? -raise : raise) * 1.0;
      map->cell(x - 1, y - 1).heights[Cell::NE] += (shift ? -raise : raise) * 1.0;
      map->cell(x + 1, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x - 1, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x + 1, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x - 1, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.25;
      map->cell(x + 1, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->cell(x - 1, y).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->cell(x, y + 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->cell(x, y - 1).heights[Cell::CENTER] += (shift ? -raise : raise) * 0.50;
      map->markCellsUpdated(x - 1, y - 1, x + 1, y + 1, false);
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
  int shift = SDL_GetModState() & KMOD_SHIFT;
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  /* TODO. Send any keys to the EOpenWindow?? */

  if (newWindow->isAttached()) {
    newWindow->key(key, shift, mouseX, mouseY);
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

  if (key == SDLK_TAB) { moveKeyboardFocus(shift); }
  if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)
    mouseDown(shift ? 3 : 1, -1, -1);

  menuWindow->key(key, shift, x, y);

  return;
}
void EditMode::tick(Real td) {
  int x, y;
  tickMouse(td);
  Uint8 mouseState = SDL_GetMouseState(&x, &y);
  MyWindow::mouseAll(mouseState, x, y);
  MyWindow::tickAll();
  time += td;
}
void EditMode::activated() {
  clearKeyboardFocus();
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
          ((int)(std::sin(1. * M_PI * (mx + radius + 0.5) / diameter) *
                 std::sin(1. * M_PI * (my + radius + 0.5) / diameter) *
                 (shift ? -raise : raise) * 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::NE] +=
          ((int)(std::sin(1. * M_PI * (mx + radius + 1.0) / diameter) *
                 std::sin(1. * M_PI * (my + radius + 1.0) / diameter) *
                 (shift ? -raise : raise) * 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::NW] +=
          ((int)(std::sin(1. * M_PI * (mx + radius) / diameter) *
                 std::sin(1. * M_PI * (my + radius + 1.0) / diameter) *
                 (shift ? -raise : raise) * 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::SE] +=
          ((int)(std::sin(1. * M_PI * (mx + radius + 1.0) / diameter) *
                 std::sin(1. * M_PI * (my + radius) / diameter) * (shift ? -raise : raise) *
                 10.)) *
          .1;
      map->cell(x + mx, y + my).heights[Cell::SW] +=
          ((int)(std::sin(1. * M_PI * (mx + radius) / diameter) *
                 std::sin(1. * M_PI * (my + radius) / diameter) * (shift ? -raise : raise) *
                 10.)) *
          .1;
    }
  map->markCellsUpdated(x - radius, y - radius, x + radius, y + radius, false);
}
void EditMode::resizeWindows() {
  menuWindow->refreshChildPositions();
  statusWindow->resize(screenWidth, 110);
  statusWindow->moveTo(0, screenHeight - 110);
  quitWindow->moveTo(screenWidth / 2 - 100, screenHeight / 2 - 50);
  saveWindow->moveTo(screenWidth / 2 - 200, screenHeight / 2 - 50);
  closeWindow->moveTo(screenWidth / 2 - 100, screenHeight / 2 - 50);
  openWindow->moveTo(screenWidth / 2 - 200, screenHeight / 2 - 150);
  newWindow->moveTo(screenWidth / 2 - 100, screenHeight / 2 - 50);
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
    }
  map->markCellsUpdated(x - radius, y - radius, x + radius, x + radius, true);
  avgHeight = avgHeight / (5. * diameter * diameter);
  for (mx = -radius; mx <= radius; mx++)
    for (my = -radius; my <= radius; my++) {
      Cell& c = map->cell(x + mx, y + my);
      double s1 = std::sin(1. * M_PI * (mx + radius + 0.5) / diameter) *
                  std::sin(1. * M_PI * (my + radius + 0.5) / diameter) *
                  (shift ? -raise : raise);
      c.heights[Cell::CENTER] = steps(c.heights[Cell::CENTER] * (1. - s1) + avgHeight * s1);
      for (north = 0; north < 2; north++)
        for (east = 0; east < 2; east++) {
          double s = std::sin(1. * M_PI * (my + radius + 1.0 * north) / diameter) *
                     std::sin(1. * M_PI * (mx + radius + 1.0 * east) / diameter) *
                     (shift ? -raise : raise);
          c.heights[Cell::NORTH * north + Cell::EAST * east] = steps(
              c.heights[Cell::NORTH * north + Cell::EAST * east] * (1. - s) + avgHeight * s);
        }
    }
  map->markCellsUpdated(x - radius, y - radius, x + radius, x + radius, true);
}
void EditMode::copyRegion() {
  if (markX < 0) return;
  int x0 = std::min(x, markX);
  int x1 = std::max(x, markX);
  int y0 = std::min(y, markY);
  int y1 = std::max(y, markY);
  int width = x1 - x0 + 1;
  int height = y1 - y0 + 1;
  if (cellClipboard) delete[] cellClipboard;
  cellClipboard = new Cell[width * height];
  cellClipboardWidth = width;
  cellClipboardHeight = height;
  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++) {
      cellClipboard[x + y * width] = map->cell(x + x0, y + y0);
    }
}
void EditMode::pasteRegion() {
  if (!cellClipboard) return;
  for (int cx = 0; cx < cellClipboardWidth; cx++)
    for (int cy = 0; cy < cellClipboardHeight; cy++) {
      Cell& toCell = map->cell(cx + x, cy + y);
      Cell& fromCell = cellClipboard[cx + cy * cellClipboardWidth];
      /* We cannot just do a memcpy here since we need to preserve displaylists and other meta
       * data */
      memcpy(toCell.velocity, fromCell.velocity, sizeof(toCell.velocity));
      memcpy(toCell.heights, fromCell.heights, sizeof(toCell.heights));
      memcpy(toCell.colors, fromCell.colors, sizeof(toCell.colors));
      memcpy(toCell.wallColors, fromCell.wallColors, sizeof(toCell.wallColors));
      memcpy(toCell.waterHeights, fromCell.waterHeights, sizeof(toCell.waterHeights));
      toCell.sunken = fromCell.sunken;
      toCell.flags = fromCell.flags;
      toCell.texture = fromCell.texture;
    }
  map->markCellsUpdated(x, y, x + cellClipboardWidth - 1, y + cellClipboardHeight - 1, true);
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
  snprintf(Settings::settings->specialLevel, sizeof(Settings::settings->specialLevel), "%s",
           levelname);
  Settings::settings->doSpecialLevel = 1;
  saveMap();
  GameMode::activate(SetupMode::init());
}
