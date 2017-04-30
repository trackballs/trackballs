/* menusystem.h
   A very naive menusystem for the various settings screens

   Copyright (C) 2004  Mathias Broxvall

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

#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

/* Primitive for creating anything selectable */
void addArea(int code,int x,int y,int w,int h);

/* Primitives for creating selectable text on screen */
void clearSelectionAreas();
void addText_Left(int code,int fontSize,int y0, const char *str,int leftX);
void addText_Right(int code,int fontSize,int y0, const char *str,int rightX);
void addText_Center(int code,int fontSize,int y0, const char *str,int cx);
void addText_LeftRight(int code,int fontSize,int y0, const char *leftStr,int leftX, const char *rightStr,int rightX);
int getSelectedArea();

/* And for resolution independent centered menus */
void menuItem_Left(int code,int row,const char *str);
void menuItem_Center(int code,int row,const char *str);
void menuItem_LeftRight(int code,int row,const char *leftStr,const char *rightStr);

#endif
