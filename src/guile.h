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

void initGuileInterface();
SCM smobAnimated_make(class Animated* a);
SCM smobGameHook_make(class GameHook* h);
void loadScript(const char* path, SCM in_module);
SCM scm_port_from_gzip(const char* highScorePath, int maxsize);
char* ascm_format(const char* str);
SCM scm_catch_call_n(SCM func, SCM args[], int n);
SCM scm_catch_call_0(SCM func);
SCM scm_catch_call_1(SCM func, SCM arg1);
SCM scm_catch_call_2(SCM func, SCM arg1, SCM arg2);

/* Whitelist of the variables that may be used by level scripts */
extern const char* const all_guile_variables[];

#endif
