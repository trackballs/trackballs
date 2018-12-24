/* replay.h
   Input recording and replay utilities

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

#ifndef REPLAY_H
#define REPLAY_H

#include "player.h"

#include <vector>

class Replay {
 public:
  Replay();
  ~Replay();

  void read(const char* level_name);
  void save(const char* level_name);

  void clear();
  void add(struct PlayerControlFrame frame);

  struct PlayerControlFrame get(int tick);

 private:
  // todo: find something with better worst case
  std::vector<struct PlayerControlFrame> log;
};

#endif
