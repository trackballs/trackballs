/* guile.h
   Some misc. functions and types for the guile interface.

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

#ifndef GUILE_H
#define GUILE_H

#include <libguile.h>

class Game;
class Animated;
class GameHook;
void initGuileInterface();
SCM smobAnimated_make(Animated* a);
SCM smobGameHook_make(GameHook* h);
void loadScript(Game* game, const char* path);
SCM scm_port_from_gzip(const char* highScorePath, int maxsize);
char* ascm_format(const char* str);
SCM scm_catch_call_n(Game* game, SCM func, SCM args[], int n);
SCM scm_catch_call_0(Game* game, SCM func);
SCM scm_catch_call_1(Game* game, SCM func, SCM arg1);
SCM scm_catch_call_2(Game* game, SCM func, SCM arg1, SCM arg2);

#endif
