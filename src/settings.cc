/* settings.cc
   Contains all the game settings

   Copyright (C) 2000-2004  Mathias Broxvall

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

#include "settings.h"

#include "editMode.h"
#include "general.h"
#include "guile.h"

#include <SDL2/SDL_joystick.h>
#include <dirent.h>
#include <libguile.h>
#include <locale.h>
#include <string.h>
#include <sys/stat.h>
#include <cstdlib>

/* These are the names/codes codes of languages available from the settings
   menu. The name of languages should appear in the respective native
   language. Eg. the Swedish language is written as 'Svenska'
*/
const char *Settings::languageCodes[7][3] = {{"", "", ""},
                                             {"de", "de_DE", "de_DE.utf8"},
                                             {"fr", "fr_FR", "fr_FR.utf8"},
                                             {"it", "it_IT", "it_IT.utf8"},
                                             {"hu", "hu_HU", "hu_HU.utf8"},
                                             {"sk", "sk_SK", "sk_SK.utf8"},
                                             {"sv", "sv_SE", "sv_SE.utf8"}};
const char *Settings::languageNames[7] = {"Default", "Deutsch", "Francais", "Italiano",
                                          "Magyar",  "Slovak",  "Svenska"};
int Settings::nLanguages = 7;

Settings *Settings::settings;
void Settings::init() { settings = new Settings(); }
Settings::Settings() {
  mouseSensitivity = 1.0;
  is_windowed = 0;
  resolution = DEFAULT_RESOLUTION;
  gfx_details = 3;
  showFPS = 0;
  difficulty = 0;
  sandbox = 0;
  sfxVolume = 1.0;
  musicVolume = 1.0;
  rotateSteering = 0;
  joystickIndex = 0;
  joystick = NULL;
  colorDepth = 32;
  joy_left = -32767;
  joy_right = 32767;
  joy_up = -32767;
  joy_down = 32767;
  lastJoyX = 0;
  lastJoyY = 0;
  ignoreMouse = 0;
  doReflections = 0;
  doShadows = 0;
  language = 0;
  vsynced = 1;
  doSpecialLevel = 0;
  nLevelSets = 0;
  joy_center[0] = 0;
  joy_center[1] = 0;
  memset(specialLevel, 0, sizeof(specialLevel));
  memset(levelSets, 0, sizeof(levelSets));

  /* Load all settings from a scheme-syntaxed config file */
  char str[256];
  snprintf(str, sizeof(str) - 1, "%s/.trackballs/settings", getenv("HOME"));
  if (access(str, R_OK) != -1) {
    SCM ip = scm_open_file(scm_from_utf8_string(str), scm_from_utf8_string("r"));
    // ^ TODO catch exception
    /* Iteratively read key-value pairs */
    for (int i = 0; i < 1000; i++) {
      SCM contents = scm_read(ip);
      if (SCM_EOF_OBJECT_P(contents)) { break; }
      if (!scm_to_bool(scm_list_p(contents)) || scm_to_int(scm_length(contents)) != 2) {
        warning("Configuration file should be a series of (key value) tuples.");
        continue;
      }
      SCM key = SCM_CAR(contents);
      SCM value = SCM_CADR(contents);
      if (!scm_is_symbol(key) || !SCM_NUMBERP(value)) {
        warning("Configuration file should be a series of (key value) tuples.");
        continue;
      }
      const int intnum = 18, realnum = 3;
      const char *intkeys[intnum] = {
          "gfx-details",    "show-fps",     "is-windowed",     "resolution", "color-depth",
          "joystick-index", "joy_center-x", "joy_center-y",    "joy-left",   "joy-right",
          "joy-up",         "joy-down",     "rotate-steering", "language",   "ignore-mouse",
          "do-reflections", "do-shadows",   "vsync-on"};

      int *intdests[intnum] = {&gfx_details,    &showFPS,       &is_windowed,   &resolution,
                               &colorDepth,     &joystickIndex, &joy_center[0], &joy_center[1],
                               &joy_left,       &joy_right,     &joy_up,        &joy_down,
                               &rotateSteering, &language,      &ignoreMouse,   &doReflections,
                               &doShadows,      &vsynced};
      const char *realkeys[realnum] = {"sfx-volume", "music-volume", "mouse-sensitivity"};
      double *realdests[realnum] = {&sfxVolume, &musicVolume, &mouseSensitivity};

      int matched = 0;
      char *skey = scm_to_utf8_string(scm_symbol_to_string(key));
      for (int j = 0; j < realnum; j++) {
        if (!strcmp(realkeys[j], skey)) {
          if (scm_is_real(value)) {
            *realdests[j] = scm_to_double(value);
          } else {
            warning("Value associated with %s is not a number", skey);
          }
          matched = 1;
          break;
        }
      }
      for (int j = 0; j < intnum; j++) {
        if (!strcmp(intkeys[j], skey)) {
          if (scm_is_integer(value)) {
            *intdests[j] = scm_to_int(value);
          } else {
            warning("Value associated with %s is not an integer", skey);
          }
          matched = 1;
          break;
        }
      }
      if (!matched) { warning("Unidentified setting: %s", skey); }
      free(skey);
    }
    scm_close(ip);
  }
}
void Settings::loadLevelSets() {
  /* Load all levelsets */
  char str[512];
  nLevelSets = 0;

  /* ugly fix to make levelset lv.set the first level set */
  snprintf(str, sizeof(str), "%s/levels/lv.set", effectiveShareDir);
  loadLevelSet(str, "lv.set");

  snprintf(str, sizeof(str), "%s/levels", effectiveShareDir);
  DIR *dir = opendir(str);
  if (!dir) {
    error("Can't find the %s/ directory", str);
  } else {
    struct dirent *dirent;
    while ((dirent = readdir(dir))) {
      if (strlen(dirent->d_name) > 4 &&
          strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".set") == 0) {
        if (strcmp(dirent->d_name, "lv.set")) {
          snprintf(str, sizeof(str), "%s/levels/%s", effectiveShareDir, dirent->d_name);
          loadLevelSet(str, dirent->d_name);
        }
      }
    }
    closedir(dir);
  }

  snprintf(str, sizeof(str) - 1, "%s/.trackballs/levels", getenv("HOME"));
  dir = opendir(str);
  if (dir) {
    struct dirent *dirent;
    while ((dirent = readdir(dir))) {
      if (strlen(dirent->d_name) > 4 &&
          strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".set") == 0) {
        snprintf(str, sizeof(str) - 1, "%s/.trackballs/levels/%s", getenv("HOME"),
                 dirent->d_name);
        loadLevelSet(str, dirent->d_name);
      }
    }
    closedir(dir);
  }

  if (!nLevelSets) {
    error("failed to load any levelsets, place levels in %s/levels/", effectiveShareDir);
  }
}
void Settings::loadLevelSet(const char *setname, const char *shortname) {
  const char *reqmnt =
      "Levelset file %s should be a series of (key \"value\") or (key (_ \"value\")) tuples.";

  strncpy(levelSets[nLevelSets].path, shortname, 256);
  // Null terminate everything in case fields are not set
  levelSets[nLevelSets].name[0] = 0;
  levelSets[nLevelSets].startLevel[0] = 0;
  levelSets[nLevelSets].startLevelName[0] = 0;
  levelSets[nLevelSets].imagename[0] = 0;
  levelSets[nLevelSets].description[0][0] = 0;
  levelSets[nLevelSets].description[1][0] = 0;
  levelSets[nLevelSets].description[2][0] = 0;
  levelSets[nLevelSets].description[3][0] = 0;
  levelSets[nLevelSets].description[4][0] = 0;

  if (access(setname, R_OK) == -1) {
    warning("Did not have read access to level set file %s", setname);
    return;
  }
  SCM ip = scm_open_file(scm_from_utf8_string(setname), scm_from_utf8_string("r"));
  for (int i = 0; i < 1000; i++) {
    SCM contents = scm_read(ip);
    if (SCM_EOF_OBJECT_P(contents)) { break; }
    if (!scm_to_bool(scm_list_p(contents)) || scm_to_int(scm_length(contents)) != 2) {
      warning(reqmnt, setname);
      continue;
    }
    SCM key = SCM_CAR(contents);
    SCM val = SCM_CADR(contents);
    if (!scm_is_symbol(key) || (!scm_is_string(val) && !scm_to_bool(scm_list_p(contents)))) {
      warning(reqmnt, setname);
      continue;
    }
    char *eval = NULL;
    if (scm_is_string(val)) {
      eval = scm_to_utf8_string(val);
      int len = strlen(eval);
      if (len > 0 && eval[len - 1] == '\n') { eval[len - 1] = 0; }
    } else {
      if (scm_to_int(scm_length(val)) != 2 || SCM_CAR(val) != scm_from_utf8_symbol("_") ||
          !scm_is_string(SCM_CADR(val))) {
        warning(reqmnt, setname);
        continue;
      }
      eval = scm_to_utf8_string(SCM_CADR(val));
    }

    char *skey = scm_to_utf8_string(scm_symbol_to_string(key));
    if (!strcmp(skey, "name")) {
      strncpy(levelSets[nLevelSets].name, eval, 256);
      levelSets[nLevelSets].name[255] = '\0';
    } else if (!strcmp(skey, "first-level-file")) {
      strncpy(levelSets[nLevelSets].startLevel, eval, 256);
      levelSets[nLevelSets].startLevel[255] = '\0';
    } else if (!strcmp(skey, "first-level-name")) {
      strncpy(levelSets[nLevelSets].startLevelName, eval, 256);
      levelSets[nLevelSets].startLevelName[255] = '\0';
    } else if (!strcmp(skey, "desc1")) {
      strncpy(&levelSets[nLevelSets].description[0][0], eval, 120);
      levelSets[nLevelSets].description[0][119] = '\0';
    } else if (!strcmp(skey, "desc2")) {
      strncpy(&levelSets[nLevelSets].description[1][0], eval, 120);
      levelSets[nLevelSets].description[1][119] = '\0';
    } else if (!strcmp(skey, "desc3")) {
      strncpy(&levelSets[nLevelSets].description[2][0], eval, 120);
      levelSets[nLevelSets].description[2][119] = '\0';
    } else if (!strcmp(skey, "desc4")) {
      strncpy(&levelSets[nLevelSets].description[3][0], eval, 120);
      levelSets[nLevelSets].description[3][119] = '\0';
    } else if (!strcmp(skey, "desc5")) {
      strncpy(&levelSets[nLevelSets].description[4][0], eval, 120);
      levelSets[nLevelSets].description[4][119] = '\0';
    } else {
      warning("Unidentified key '%s' in level set file %s", skey, setname);
    }

    free(skey);
    free(eval);
  }
  scm_close_input_port(ip);

  char imagename[256];
  strncpy(imagename, setname, sizeof(imagename));
  imagename[255] = '\0';
  strncpy(imagename + strlen(imagename) - 4, ".jpg", sizeof(imagename) - strlen(imagename));
  imagename[255] = '\0';
  FILE *fp2 = fopen(setname, "r");
  if (fp2)
    fclose(fp2);
  else
    imagename[0] = 0;
  strncpy(levelSets[nLevelSets].imagename, imagename, sizeof(levelSets[nLevelSets].imagename));
  levelSets[nLevelSets].imagename[255] = '\0';

  nLevelSets++;
}

void Settings::save() {
  char str[256];

  snprintf(str, sizeof(str) - 1, "%s/.trackballs", getenv("HOME"));
  if (pathIsLink(str)) {
    warning("%s is a symbolic link. Cannot save settings", str);
    return;
  }

  mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);
  snprintf(str, sizeof(str) - 1, "%s/.trackballs/settings", getenv("HOME"));
  if (pathIsLink(str)) {
    warning("%s is a symbolic link. Cannot save settings", str);
    return;
  }

  /* Save all settings here */
  FILE *fp = fopen(str, "w");
  if (!fp) {
    warning("Could not save settings to '%s'.", str);
  } else {
    fprintf(fp, "(sfx-volume %.3g)\n", sfxVolume);
    fprintf(fp, "(music-volume %.3g)\n", musicVolume);
    fprintf(fp, "(mouse-sensitivity %.3g)\n", mouseSensitivity);
    fprintf(fp, "(gfx-details %d)\n", gfx_details);
    fprintf(fp, "(show-fps %d)\n", showFPS);
    fprintf(fp, "(is-windowed %d)\n", is_windowed);
    fprintf(fp, "(resolution %d)\n", resolution);
    fprintf(fp, "(color-depth %d)\n", colorDepth);
    fprintf(fp, "(joystick-index %d)\n", joystickIndex);
    fprintf(fp, "(joy_center-x %d)\n", joy_center[0]);
    fprintf(fp, "(joy_center-y %d)\n", joy_center[1]);
    fprintf(fp, "(joy-left %d)\n", joy_left);
    fprintf(fp, "(joy-right %d)\n", joy_right);
    fprintf(fp, "(joy-up %d)\n", joy_up);
    fprintf(fp, "(joy-down %d)\n", joy_down);
    fprintf(fp, "(rotate-steering %d)\n", rotateSteering);
    fprintf(fp, "(ignore-mouse %d)\n", ignoreMouse);
    fprintf(fp, "(do-reflections %d)\n", doReflections);
    fprintf(fp, "(do-shadows %d)\n", doShadows);
    fprintf(fp, "(language %d)\n", language);
    fprintf(fp, "(vsync-on %d)\n", vsynced);
    fclose(fp);
  }
}

int Settings::hasJoystick() {
  if (!Settings::settings->joystickIndex) return 0;

  if (!joystick) joystick = SDL_JoystickOpen(Settings::settings->joystickIndex - 1);
  if (!joystick) {
    warning("failed to open joystick no. %d", Settings::settings->joystickIndex - 1);
    joystickIndex = 0;
  }
  return joystickIndex;
}
void Settings::closeJoystick() {
  if (joystick) SDL_JoystickClose(joystick);
  joystick = NULL;
}
int Settings::joystickRawX() {
  static int lastX = 0;
  SDL_JoystickUpdate();
  int x = SDL_JoystickGetAxis(joystick, 0);
  // Repair some broken joysticks
  if (repair_joystick) {
    if (lastX > 25000 && x < -3000) x = 32767;
    if (lastX < -25000 && x > 3000) x = -32767;
  }
  lastX = x;
  return x;
}
int Settings::joystickRawY() {
  static int lastY = 0;
  SDL_JoystickUpdate();
  int y = SDL_JoystickGetAxis(joystick, 1);
  // Repair some broken joysticks
  if (repair_joystick) {
    if (lastY > 25000 && y < -3000) y = 32767;
    if (lastY < -25000 && y > 3000) y = -32767;
  }
  lastY = y;
  return y;
}
double Settings::joystickX() {
  if (!joystickIndex) return 0.0;
  if (!joystick) joystick = SDL_JoystickOpen(Settings::settings->joystickIndex - 1);
  if (!joystick) {
    warning("failed to open joystick no. %d", Settings::settings->joystickIndex - 1);
    joystickIndex = 0;
  }
  int joyX = joystickRawX();
  if ((joyX > joy_center[0] && joy_right > joy_center[0]) ||
      (joyX < joy_center[0] && joy_right < joy_center[0])) {
    /* Joystick should be to the right */
    return (joyX - joy_center[0]) / (double)(joy_right - joy_center[0]);
  } else {
    /* Joystick should be to the left */
    return (joyX - joy_center[0]) / (double)(joy_left - joy_center[0]) * -1.0;
  }
}
double Settings::joystickY() {
  if (!joystickIndex) return 0.0;
  if (!joystick) joystick = SDL_JoystickOpen(Settings::settings->joystickIndex - 1);
  if (!joystick) {
    warning("failed to open joystick no. %d", Settings::settings->joystickIndex - 1);
    joystickIndex = 0;
  }

  int joyY = joystickRawY();

  if ((joyY > joy_center[1] && joy_down > joy_center[1]) ||
      (joyY < joy_center[1] && joy_down < joy_center[1])) {
    /* Joystick should be to the down */
    return (joyY - joy_center[1]) / (double)(joy_down - joy_center[1]);
  } else {
    /* Joystick should be to the up */
    return (joyY - joy_center[1]) / (double)(joy_up - joy_center[1]) * -1.0;
  }
}
int Settings::joystickButton(int n) {
  if (!joystickIndex) return 0;
  SDL_JoystickUpdate();
  return SDL_JoystickGetButton(joystick, n);
}
void Settings::setLocale() {
  /* check if the correct language directory exists, otherwise use default */
  if (language != 0) {
    char localedir[512];
#ifdef LOCALEDIR
    snprintf(localedir, 511, "%s/%c%c", LOCALEDIR, languageCodes[language][0][0],
             languageCodes[language][0][1]);
#else
    snprintf(localedir, 511, "%s/locale/%c%c", effectiveShareDir,
             languageCodes[language][0][0], languageCodes[language][0][1]);
#endif

    if (!dirExists(localedir)) {
      warning("locale directory %s missing.\n", localedir);
      // language=0;
    }
  }

  /* Set the locale, trying several name options */
  char *ret = NULL;
  for (int i = 0; i < 3; i++) {
    ret = setlocale(LC_MESSAGES, languageCodes[language][i]);
    if (ret) { break; }
  }
  if (ret == NULL) {
    warning("Setting the locale (%s %s %s) failed", languageCodes[language][0],
            languageCodes[language][1], languageCodes[language][2]);
  }

  EditMode::loadStrings();
}
