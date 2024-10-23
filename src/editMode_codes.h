/* editMode_codes.h
   Containts all the internal codes & constants for the edit mode and edit windows

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

#ifndef EDITMODECODES_H
#define EDITMODECODES_H

#define N_SUBMENUS 9
#define FILE_MENU 0
#define EDIT_MENU 1
#define COLOR_MENU 2
#define FLAGS_MENU 3
#define FEATURE_MENU 4
#define REPAIR_MENU 5
#define MOVE_MENU 6
#define WINDOW_MENU 7
#define VIEW_MENU 8

#define MAX_MENU_ENTRIES 20

#define FILE_NEW 00
#define FILE_OPEN 01
#define FILE_CLOSE 02
#define FILE_SAVE 03
#define FILE_EXIT 04
#define FILE_TEST 05

#define EDIT_HEIGHT 20
#define EDIT_COLOR 21
#define EDIT_WATER 22
#define EDIT_VELOCITY 23
#define EDIT_LINES 24
#define EDIT_ALL 25
#define EDIT_UPPER 26
#define EDIT_BOTTOM 27
#define EDIT_RIGHT 28
#define EDIT_LEFT 29
#define EDIT_CENTER 30
#define EDIT_RAISE_INCREMENT 31
#define EDIT_LOWER_INCREMENT 32

#define COLOR_RED 40
#define COLOR_GREEN 41
#define COLOR_BLUE 42
#define COLOR_ALPHA 43

#define FLAG_0 60
#define FLAG_1 61
#define FLAG_2 62
#define FLAG_3 63
#define FLAG_4 64
#define FLAG_5 65
#define FLAG_6 66
#define FLAG_7 67
#define FLAG_CH_TEXTURE 69

#define FEATURE_SPIKE 80
#define FEATURE_SMALL_HILL 81
#define FEATURE_MEDIUM_HILL 82
#define FEATURE_LARGE_HILL 83
#define FEATURE_HUGE_HILL 84
#define FEATURE_SMALL_SMOOTH 85
#define FEATURE_LARGE_SMOOTH 86

#define FEATURE_SPIKE 80
#define FEATURE_SMALL_HILL 81
#define FEATURE_MEDIUM_HILL 82
#define FEATURE_LARGE_HILL 83
#define FEATURE_HUGE_HILL 84
#define FEATURE_SMALL_SMOOTH 85
#define FEATURE_LARGE_SMOOTH 86

#define REPAIR_CELL_CONT 100
#define REPAIR_WATER_CONT 101
#define REPAIR_COLOR_CONT 102
#define REPAIR_CELL_CENTERS 103
#define REPAIR_WATER_CENTERS 104
#define REPAIR_COLOR_CENTERS 105
#define REPAIR_CELL_ROUND 106
#define REPAIR_WATER_ROUND 107
#define REPAIR_COLOR_ROUND 108
#define REPAIR_VEL_ROUND 109

#define MOVE_UP 120
#define MOVE_DOWN 121
#define MOVE_LEFT 122
#define MOVE_RIGHT 123
// BLANK                      124
#define MOVE_SHIFT_UP 125
#define MOVE_SHIFT_DOWN 126
#define MOVE_SHIFT_LEFT 127
#define MOVE_SHIFT_RIGHT 128
// SHIFT                      129
// BLANK                      130
#define MOVE_SET_MARKER 131
#define MOVE_CLEAR_MARKER 132
#define MOVE_COPY_REGION 133
#define MOVE_PASTE_REGION 134

#define WINDOW_EDITOR 140
#define WINDOW_TOOLBAR 141
#define WINDOW_STATUS 142

#define VIEW_BIRD 160
#define VIEW_ROTATE 161
#define VIEW_LOAD_ENTITIES 162
#define VIEW_CLEAR_ENTITIES 163

#define EDITMODE_HEIGHT 0
#define EDITMODE_COLOR 1
#define EDITMODE_WATER 2
#define EDITMODE_VELOCITY 3
#define EDITMODE_NOLINES 4
#define EDITMODE_FEATURES 5
#define N_EDITMODES 6

extern char *cMenuNames[N_SUBMENUS];
extern char *cMenuEntries[N_SUBMENUS][MAX_MENU_ENTRIES];
extern const char *cKeyShortcuts[N_SUBMENUS];

/* Converts between SUBMENU id's and highlight codes */
#define CODE_FROM_MENU(x) ((x) + 1)
#define CODE_TO_MENU(x) ((x) - 1)

#define CODE_FROM_MENUENTRY(x) (x + 10)
#define CODE_TO_MENUENTRY(x) (x - 10)

#define CODE_INCREMENT 200
#define CODE_EDITMODE 201
#define CODE_YES 202
#define CODE_NO 203
#define CODE_OK 204
#define CODE_CANCEL 205
#define CODE_PAGE 206
#define CODE_MAP0 300

#endif
