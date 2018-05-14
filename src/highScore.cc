/** \file highScore.cc
   Saves and loads the highscore lists
*/
/*
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

#include "highScore.h"

#include "game.h"
#include "general.h"
#include "guile.h"
#include "settings.h"

#include <libguile.h>
#include <zlib.h>
#include <cstdlib>

static char highScorePath[256];
static HighScore* highScore = NULL;

HighScore::HighScore() {
  for (int levelSet = 0; levelSet < Settings::settings->nLevelSets; levelSet++)
    for (int i = 0; i < 10; i++) {
      points[levelSet][i] = 1000 - i * 100;
      dummy_player[levelSet][i] = 1;
      memset(names[levelSet][i], 0, sizeof(names[levelSet][i]));
    }

#ifdef ALT_HIGHSCORES
  /* Copy ALT_HIGHSCORES into highScorePath, substituting the '~' character for the HOME
   * directory */
  if (ALT_HIGHSCORES[0] == '~') {
    if (ALT_HIGHSCORES[1] == 0)
      snprintf(highScorePath, sizeof(highScorePath), "%s/.trackballs/highScores",
               getenv("HOME"));
    else if (ALT_HIGHSCORES[1] == '/')
      snprintf(highScorePath, sizeof(highScorePath), "%s%s/highScores", getenv("HOME"),
               &ALT_HIGHSCORES[1]);
    else {
      error("Bad ALT_HIGHSCORES compiled into game '%s'", ALT_HIGHSCORES);
    }
  } else {
    snprintf(highScorePath, sizeof(highScorePath), "%s/highScores", ALT_HIGHSCORES);
  }
#else
  snprintf(highScorePath, sizeof(highScorePath), "%s/highScores", effectiveShareDir);
#endif
  if (pathIsLink(highScorePath)) {
    warning("%s is a symbolic link. Cannot load highscores\n", highScorePath);
    return;
  }
  SCM ip = scm_port_from_gzip(highScorePath, 256 * 10 * 25 * 3);
  if (SCM_EOF_OBJECT_P(ip)) { return; }

  SCM contents = scm_read(ip);
  if (SCM_EOF_OBJECT_P(contents) || !scm_is_integer(contents)) {
    scm_close_input_port(ip);
    warning("Incorrect format for highscore file %s", highScorePath);
    return;
  }
  int nLevelSets = scm_to_int32(contents);
  int levelSet;
  for (; nLevelSets; nLevelSets--) {
    SCM block = scm_read(ip);
    if (SCM_EOF_OBJECT_P(block) || !scm_to_bool(scm_list_p(block)) ||
        scm_to_int(scm_length(block)) != 11) {
      scm_close_input_port(ip);
      warning("Incorrect format for highscore file %s", highScorePath);
      return;
    }
    SCM sname = SCM_CAR(block);
    char* name = scm_to_utf8_string(sname);
    for (levelSet = 0; levelSet < Settings::settings->nLevelSets; levelSet++)
      if (strcmp(name, Settings::settings->levelSets[levelSet].path) == 0) break;
    if (levelSet == Settings::settings->nLevelSets) {
      scm_close_input_port(ip);
      warning("Highscores contains info about unknown levelset %s", name);
      free(name);
      return;
    }
    free(name);
    for (int i = 0; i < 10; i++) {
      SCM cell = scm_list_ref(block, scm_from_int32(i + 1));
      if (!scm_to_bool(scm_list_p(cell)) || scm_to_int(scm_length(cell)) != 2 ||
          !((scm_is_string(SCM_CAR(cell)) &&
             scm_to_int32(scm_string_length(SCM_CAR(cell))) < 25) ||
            scm_is_false(SCM_CAR(cell))) ||
          !scm_is_integer(SCM_CADR(cell))) {
        scm_close_input_port(ip);
        warning("Incorrect format for highscore file %s", highScorePath);
        return;
      }
      if (scm_is_false(SCM_CAR(cell))) {
        dummy_player[levelSet][i] = 1;
      } else {
        char* lname = scm_to_utf8_string(SCM_CAR(cell));
        strncpy(&names[levelSet][i][0], lname, 25);
        free(lname);
        points[levelSet][i] = scm_to_int32(SCM_CADR(cell));
        dummy_player[levelSet][i] = 0;
      }
    }
  }
  scm_close_input_port(ip);
}
HighScore* HighScore::init() {
  if (!highScore) highScore = new HighScore();
  return highScore;
}
void HighScore::cleanup() {
  if (highScore) delete highScore;
}
int HighScore::isHighScore(int score) {
  if (Game::current->currentLevelSet < 0) return 0;
  return score > points[Game::current->currentLevelSet][9];
}
void HighScore::addHighScore(int score, char* name) {
  int levelSet = Game::current->currentLevelSet;
  if (levelSet < 0) return;

  int i;
  for (i = 0; i < 10; i++)
    if (score > points[levelSet][i]) break;
  if (i < 10) {
    for (int j = 9; j > i; j--) {
      strncpy(names[levelSet][j], names[levelSet][j - 1], sizeof(names[levelSet][j]));
      points[levelSet][j] = points[levelSet][j - 1];
      dummy_player[levelSet][j] = dummy_player[levelSet][j - 1];
    }
    strncpy(names[levelSet][i], name, sizeof(names[levelSet][i]));
    points[levelSet][i] = score;
    dummy_player[levelSet][i] = 0;
  }

  if (pathIsLink(highScorePath)) {
    warning("Error, %s is a symbolic link. Cannot save highscores", highScorePath);
    return;
  }

  Settings* settings = Settings::settings;
  gzFile gp = gzopen(highScorePath, "wb9");
  if (!gp) {
    warning("Warning. Cannot save highscores at %s, check file permissions", highScorePath);
    return;
  }
  gzprintf(gp, "%d\n", Settings::settings->nLevelSets);
  for (levelSet = 0; levelSet < Settings::settings->nLevelSets; levelSet++) {
    char* lsname = ascm_format(settings->levelSets[levelSet].path);
    gzprintf(gp, "(%s\n", lsname);
    free(lsname);
    for (int i = 0; i < 10; i++) {
      if (dummy_player[levelSet][i]) {
        gzprintf(gp, "  (#f %d)\n", points[levelSet][i]);
      } else {
        char* recname = ascm_format(&names[levelSet][i][0]);
        gzprintf(gp, "  (%s %d)\n", recname, points[levelSet][i]);
        free(recname);
      }
    }
    gzprintf(gp, ")\n");
  }
  gzclose(gp);
}
