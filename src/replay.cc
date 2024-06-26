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

#include <sys/stat.h>
#include <zlib.h>

#include <cstring>

Replay::Replay() {}
Replay::~Replay() {}

static const char log_header_msg[] =
    "Trackballs log file: binary player input data, very fragile.";

static bool getLogPath(const char* level_name, char log_file[768]) {
  char str[768];

  snprintf(str, sizeof(str) - 1, "%s/logs", effectiveLocalDir);
  if (pathIsLink(str)) {
    warning("Error, %s//logs is a symbolic link. Cannot load/save replay log",
            effectiveLocalDir);
    return false;
  } else if (!pathIsDir(str)) {
    mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);
  }

  snprintf(log_file, 768 - 1, "%s/logs/%s.log", effectiveLocalDir, level_name);
  if (pathIsLink(str)) {
    warning("Error, %s/logs/%s.log is a symbolic link. Cannot load/save replay log",
            effectiveLocalDir, level_name);
    return false;
  }
  return true;
}
void Replay::read(const char* level_name) {
  fprintf(stderr, "Reading logs for '%s'\n", level_name);
  log.clear();

  char log_file[784];
  if (!getLogPath(level_name, log_file)) { return; }

  gzFile file = gzopen(log_file, "r9");
  char tmp[256];
  gzread(file, tmp, strlen(log_header_msg) + 1);
  gzread(file, &settings, sizeof(settings));
  while (true) {
    union {
      struct PlayerControlFrame frame;
      char buf[sizeof(struct PlayerControlFrame)];
    } data;
    int nbytes = gzread(file, (void*)data.buf, sizeof(struct PlayerControlFrame));
    if (nbytes != sizeof(struct PlayerControlFrame)) { break; }
    log.push_back(data.frame);
  }

  gzclose(file);
}
void Replay::save(const char* level_name) {
  fprintf(stderr, "Writing logs for '%s'\n", level_name);
  char log_file[784];
  if (!getLogPath(level_name, log_file)) { return; }

  gzFile file = gzopen(log_file, "w9");
  gzwrite(file, (void*)log_header_msg, strlen(log_header_msg) + 1);
  gzwrite(file, &settings, sizeof(settings));
  for (int i = 0; i < log.size(); i++) {
    union {
      struct PlayerControlFrame frame;
      char buf[sizeof(struct PlayerControlFrame)];
    } data;
    memset(data.buf, 0, sizeof(struct PlayerControlFrame));
    data.frame.dx = log[i].dx;
    data.frame.dy = log[i].dy;
    data.frame.jump = log[i].jump;
    data.frame.die = log[i].die;
    data.frame.inactive = log[i].inactive;
    gzwrite(file, (void*)data.buf, sizeof(struct PlayerControlFrame));
  }
  gzclose(file);
}

void Replay::clear() { log.clear(); }
void Replay::init(struct GameSettings s) { settings = s; }
void Replay::add(struct PlayerControlFrame frame) { log.push_back(frame); }

struct PlayerControlFrame Replay::get(int tick) {
  if (log.size() > tick && tick >= 0) {
    return log[tick];
  } else {
    struct PlayerControlFrame neutral;
    neutral.inactive = true;
    neutral.dx = 0.;
    neutral.dy = 0.;
    neutral.jump = false;
    neutral.die = false;
    return neutral;
  }
}

struct GameSettings Replay::getSettings() { return settings; }
