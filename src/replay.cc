/* replay.cc

   Copyright (C) 2018  Manuel Stoeckl

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

#include "replay.h"

#include <zlib.h>

Replay::Replay() {}
Replay::~Replay() {}

void Replay::read(const char* level_name) {}
void Replay::save(const char* level_name) {}

void Replay::clear() {}
void Replay::add(struct PlayerControlFrame frame) {}

struct PlayerControlFrame Replay::get(int tick) {}
