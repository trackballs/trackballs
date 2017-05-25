/* gamer.cc
   Represents a gamer

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

#include "general.h"
#include "gamer.h"
#include "settings.h"
#include "game.h"
#include "map.h"
#include "guile.h"
#include "player.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
using namespace std;

Gamer::Gamer() {
  strcpy(name, "John Doe");
  if (NULL != getenv("USER")) {
    snprintf(name, 20, "%s", getenv("USER"));
  }
#ifdef WIN32
  else {
    if (NULL != getenv("USERNAME")) { snprintf(name, 20, "%s", getenv("USERNAME")); }
  }
#endif

  for (int i = 0; i < Settings::settings->nLevelSets; i++) levels[i] = new KnownLevel[256];

  setDefaults();
  update();
  reloadNames();
  if (nNames > 0) strncpy(name, names[0], 20);
  update();
}
void Gamer::setDefaults() {
  color = 0;
  totalScore = 0;
  nLevelsCompleted = 0;
  timesPlayed = 0;
  nLevelsCompleted = 0;
  for (int i = 0; i < Settings::settings->nLevelSets; i++) {
    nKnownLevels[i] = 1;
    strcpy(levels[i][0].name, Settings::settings->levelSets[i].startLevelName);
    strcpy(levels[i][0].fileName, Settings::settings->levelSets[i].startLevel);
  }
  textureNum = 0;
}
void Gamer::levelStarted() {
  int i;
  char *level = Game::current->levelName;
  if (Game::current->currentLevelSet < 0) return;  // don't modify profile when cheating
  if (Game::current->map->isBonus) return;  // bonus levels are not added to known levels
  for (i = 0; i < nKnownLevels[Game::current->currentLevelSet]; i++)
    if (strcmp(levels[Game::current->currentLevelSet][i].fileName, level) == 0) break;
  if (i == nKnownLevels[Game::current->currentLevelSet]) {
    strncpy(levels[Game::current->currentLevelSet][i].fileName, level, 64);
    strncpy(levels[Game::current->currentLevelSet][i].name, Game::current->map->mapname, 64);
    nKnownLevels[Game::current->currentLevelSet]++;
    nLevelsCompleted++;
    save();
  }
}

/* Note. save/update need not use a platform independent format since the save
   files are meant to be local. */
void Gamer::save() {
  char str[256];
  int levelSet;

  Settings *settings = Settings::settings;

  snprintf(str, sizeof(str) - 1, "%s/.trackballs", getenv("HOME"));
  if (pathIsLink(str)) {
    fprintf(stderr, _("Error, %s/.trackballs is a symbolic link. Cannot save settings\n"),
            getenv("HOME"));
    return;
  }
  if (!pathIsDir(str)) mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);
  snprintf(str, sizeof(str) - 1, "%s/.trackballs/%s.gmr", getenv("HOME"), name);
  if (pathIsLink(str)) {
    fprintf(stderr,
            _("Error, %s/.trackballs/%s.gmr is a symbolic link. Cannot save settings\n"),
            getenv("HOME"), name);
    return;
  }

  gzFile gp = gzopen(str, "wb9");
  if (gp) {
    gzprintf(gp, "(color %d)\n", color);
    gzprintf(gp, "(texture %d)\n", textureNum);
    gzprintf(gp, "(total-score %d)\n", totalScore);
    gzprintf(gp, "(levels-completed %d)\n", nLevelsCompleted);
    gzprintf(gp, "(times-played %d)\n", timesPlayed);
    gzprintf(gp, "(difficulty %d)\n", Settings::settings->difficulty);
    gzprintf(gp, "(sandbox %d)\n", Settings::settings->sandbox);
    gzprintf(gp, "(levelsets %d\n", Settings::settings->nLevelSets);
    for (levelSet = 0; levelSet < Settings::settings->nLevelSets; levelSet++) {
      char *name = ascm_format(settings->levelSets[levelSet].name);
      gzprintf(gp, "  (%s %d\n", name, nKnownLevels[levelSet]);
      free(name);
      for (int i = 0; i < nKnownLevels[levelSet]; i++) {
        char *fname = ascm_format(&levels[levelSet][i].fileName[0]);
        char *tname = ascm_format(&levels[levelSet][i].name[0]);
        gzprintf(gp, "    (%s %s)\n", fname, tname);
        free(fname);
        free(tname);
      }
      gzprintf(gp, "  )\n");
    }
    gzprintf(gp, ")\n");
    gzclose(gp);
  }
}
void Gamer::update() {
  char str[256];

  snprintf(str, sizeof(str) - 1, "%s/.trackballs/%s.gmr", getenv("HOME"), name);
  SCM ip = scm_port_from_gzip(str);
  if (SCM_EOF_OBJECT_P(ip)) {
    setDefaults();
    return;
  }
  const char *fmterr = _("Warning. Profile format error for player %s\n");
  for (int i = 0; i < 1000; i++) {
    SCM blob = scm_read(ip);
    if (SCM_EOF_OBJECT_P(blob)) { break; }
    if (!scm_to_bool(scm_list_p(blob)) || scm_to_int(scm_length(blob)) < 2 ||
        !scm_is_symbol(SCM_CAR(blob))) {
      fprintf(stderr, fmterr, name);
      break;
    }
    char *skey = scm_to_utf8_string(scm_symbol_to_string(SCM_CAR(blob)));
    if (!strcmp(skey, "levelsets")) {
      free(skey);
      if (!scm_is_integer(SCM_CADR(blob))) {
        fprintf(stderr, fmterr, name);
        break;
      }
      int nLevelSets = scm_to_int32(SCM_CADR(blob));
      if (scm_to_int(scm_length(blob)) != nLevelSets + 2) {
        fprintf(stderr, fmterr, name);
        break;
      }

      for (; nLevelSets; nLevelSets--) {
        SCM block = scm_list_ref(blob, scm_from_int32(nLevelSets + 1));
        if (!scm_to_bool(scm_list_p(block)) || !scm_is_string(SCM_CAR(block)) ||
            !scm_is_integer(SCM_CADR(block)) || scm_to_int32(SCM_CADR(block)) <= 0) {
          fprintf(stderr, fmterr, name);
          break;
        }
        char *lsname = scm_to_utf8_string(SCM_CAR(block));
        int levelSet;
        for (levelSet = 0; levelSet < Settings::settings->nLevelSets; levelSet++)
          if (strcmp(lsname, Settings::settings->levelSets[levelSet].name) == 0) break;
        free(lsname);
        if (levelSet == Settings::settings->nLevelSets) {
          fprintf(stderr, _("Error: Profile for %s contains info for unknown levelset %s\n"),
                  str, lsname);
          break;
        }
        nKnownLevels[levelSet] = scm_to_int32(SCM_CADR(block));
        for (int i = 0; i < nKnownLevels[levelSet]; i++) {
          SCM cell = scm_list_ref(block, scm_from_int32(i + 2));
          if (!scm_to_bool(scm_list_p(cell)) || scm_to_int(scm_length(cell)) != 2 ||
              !scm_is_string(SCM_CAR(cell)) || !scm_is_string(SCM_CADR(cell)) ||
              scm_to_int32(scm_string_length(SCM_CAR(cell))) >= 64 ||
              scm_to_int32(scm_string_length(SCM_CADR(cell))) >= 64) {
            fprintf(stderr, fmterr, name);
            break;
          }
          char *fname = scm_to_utf8_string(SCM_CAR(cell));
          char *tname = scm_to_utf8_string(SCM_CADR(cell));
          strncpy(&levels[levelSet][i].fileName[0], fname, 64);
          strncpy(&levels[levelSet][i].name[0], tname, 64);
          free(fname);
          free(tname);
        }
      }
    } else {
      const char *keys[7] = {"color",        "texture",    "total-score", "levels-completed",
                             "times-played", "difficulty", "sandbox"};
      int *dests[7] = {&color,
                       &textureNum,
                       &totalScore,
                       &nLevelsCompleted,
                       &timesPlayed,
                       &Settings::settings->difficulty,
                       &Settings::settings->sandbox};
      if (scm_to_int(scm_length(blob)) != 2 || !scm_is_integer(SCM_CADR(blob))) {
        fprintf(stderr, fmterr, name);
        free(skey);
        break;
      }
      int val = scm_to_int32(SCM_CADR(blob));
      for (int i = 0; i < 7; i++) {
        if (!strcmp(skey, keys[i])) { *dests[i] = val; }
      }
      free(skey);
    }
  }
  scm_close_input_port(ip);
  return;
}
void Gamer::playerLoose() {
  timesPlayed++;
  totalScore += Game::current->player1->score;
  save();
}
void Gamer::reloadNames() {
  char str[256];
  int i, len;

  nNames = 0;
  snprintf(str, sizeof(str) - 1, "%s/.trackballs", getenv("HOME"));
  DIR *dir = opendir(str);
  if (dir) {
    struct dirent *dirent;
    while ((dirent = readdir(dir))) {
      if (strlen(dirent->d_name) > 4 &&
          strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".gmr") == 0) {
        len = strlen(dirent->d_name);
        for (i = 0; i < len - 4; i++) names[nNames][i] = dirent->d_name[i];
        names[nNames][i] = 0;
        nNames++;
      }
    }
    closedir(dir);
  }
}
