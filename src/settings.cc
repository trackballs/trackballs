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

#include "general.h"
#include "settings.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "guile.h"

using namespace std;

/* These are the names/codes codes of languages available from the settings
   menu. The name of languages should appear in the respective native
   language. Eg. the Swedish language is written as 'Svenska'
*/
const char *Settings::languageCodes[7][3] = {{"", "", ""},
                                             {"de", "de_DE", "de_DE.utf8"},
                                             {"fr", "fr_FR", "fr_FR.utf8"},
                                             {"it", "it_IT", "it_IT.utf8"},
                                             {"hu", "hu_Hu", "hu_Hu.utf8"},
                                             {"sk", "sk_SK", "sk_SK.utf8"},
                                             {"sv", "sk_SK", "sk_SK.utf8"}};
const char *Settings::languageNames[7] = {"Default",  "Deutsch", "Francais", "Magyar",
                                          "Italiano", "Slovak",  "Svenska"};
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
  colorDepth = 16;
  joy_left = -32767;
  joy_right = 32767;
  joy_up = -32767;
  joy_down = 32767;
  lastJoyX = 0;
  lastJoyY = 0;
  ignoreMouse = 0;
  doReflections = 0;
  language = 0;
  vsynced = 1;

  /* Load all settings from a scheme-syntaxed config file */
  char str[256];
  snprintf(str, sizeof(str) - 1, "%s/.trackballs/settings", getenv("HOME"));
  const char *errstring1 = _("Configuration file should be a series of (key value) tuples.");
  const char *errstring2 = _("Error in value associated with:");
  if (access(str, R_OK) != -1) {
    SCM ip = scm_open_file(scm_from_locale_string(str), scm_from_locale_string("r"));
    // ^ TODO catch exception
    /* Iteratively read key-value pairs */
    for (int i = 0; i < 1000; i++) {
      SCM contents = scm_read(ip);
      if (SCM_EOF_OBJECT_P(contents)) { break; }
      if (!scm_to_bool(scm_list_p(contents)) || scm_to_int(scm_length(contents)) != 2) {
        fprintf(stderr, "%s\n", errstring1);
        continue;
      }
      SCM key = SCM_CAR(contents);
      SCM value = SCM_CADR(contents);
      if (!scm_is_symbol(key) || !SCM_NUMBERP(value)) {
        fprintf(stderr, "%s\n", errstring1);
        continue;
      }
      char *skey = scm_to_utf8_string(scm_symbol_to_string(key));
      if (!strcmp(skey, "sfx-volume")) {
        if (scm_is_real(value)) {
          sfxVolume = scm_to_double(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "music-volume")) {
        if (scm_is_real(value)) {
          musicVolume = scm_to_double(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "mouse-sensitivity")) {
        if (scm_is_real(value)) {
          mouseSensitivity = scm_to_double(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "gfx-details")) {
        if (scm_is_integer(value)) {
          gfx_details = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "show-fps")) {
        if (scm_is_integer(value)) {
          showFPS = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "is-windowed")) {
        if (scm_is_integer(value)) {
          is_windowed = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "resolution")) {
        if (scm_is_integer(value)) {
          resolution = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "color-depth")) {
        if (scm_is_integer(value)) {
          colorDepth = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joystick-index")) {
        if (scm_is_integer(value)) {
          joystickIndex = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joy_center-x")) {
        if (scm_is_integer(value)) {
          joy_center[0] = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joy_center-y")) {
        if (scm_is_integer(value)) {
          joy_center[1] = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joy-left")) {
        if (scm_is_integer(value)) {
          joy_left = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joy-right")) {
        if (scm_is_integer(value)) {
          joy_right = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joy-up")) {
        if (scm_is_integer(value)) {
          joy_up = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "joy-down")) {
        if (scm_is_integer(value)) {
          joy_down = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "rotate-steering")) {
        if (scm_is_integer(value)) {
          rotateSteering = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "language")) {
        if (scm_is_integer(value)) {
          language = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "ignore-mouse")) {
        if (scm_is_integer(value)) {
          ignoreMouse = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "do-reflections")) {
        if (scm_is_integer(value)) {
          doReflections = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else if (!strcmp(skey, "vsync-on")) {
        if (scm_is_integer(value)) {
          vsynced = scm_to_int(value);
        } else {
          fprintf(stderr, "%s >%s<\n", errstring2, skey);
        }
      } else {
        fprintf(stderr, "%s >%s<\n", _("Unidentified setting:"), skey);
      }

      free(skey);
    }
    scm_close(ip);
  }
}
void Settings::loadLevelSets() {
  /* Load all levelsets */
  char str[256];
  nLevelSets = 0;

  /* ugly fix to make levelset lv.set the first level set */
  snprintf(str, sizeof(str), "%s/levels/lv.set", SHARE_DIR);
  loadLevelSet(str);

  snprintf(str, sizeof(str), "%s/levels", SHARE_DIR);
  DIR *dir = opendir(str);
  if (!dir) {
    printf("Can't find the %s/ directory\n", str);
    exit(0);
  }
  struct dirent *dirent;
  while ((dirent = readdir(dir))) {
    if (strlen(dirent->d_name) > 4 &&
        strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".set") == 0) {
      if (strcmp(dirent->d_name, "lv.set")) {
        snprintf(str, sizeof(str), "%s/levels/%s", SHARE_DIR, dirent->d_name);
        loadLevelSet(str);
      }
    }
  }

  snprintf(str, sizeof(str) - 1, "%s/.trackballs/levels", getenv("HOME"));
  dir = opendir(str);
  if (dir)
    while ((dirent = readdir(dir))) {
      if (strlen(dirent->d_name) > 4 &&
          strcmp(&dirent->d_name[strlen(dirent->d_name) - 4], ".set") == 0) {
        snprintf(str, sizeof(str) - 1, "%s/.trackballs/levels/%s", getenv("HOME"),
                 dirent->d_name);
        loadLevelSet(str);
      }
    }

  if (!nLevelSets) {
    printf("Error: failed to load any levelsets, place levels in %s/levels/", SHARE_DIR);
    exit(0);
  }
}
void Settings::loadLevelSet(char *setname) {
  char imagename[256];
  FILE *fp = fopen(setname, "r");
  FILE *fp2;
  int lineno;

  if (!fp)
    printf("Warning: Failure reading levelSet %s\n", setname);
  else {
    fgets(levelSets[nLevelSets].name, 256, fp);
    if (levelSets[nLevelSets].name[strlen(levelSets[nLevelSets].name) - 1] == '\n')
      levelSets[nLevelSets].name[strlen(levelSets[nLevelSets].name) - 1] = 0;
    fgets(levelSets[nLevelSets].startLevel, 256, fp);
    if (levelSets[nLevelSets].startLevel[strlen(levelSets[nLevelSets].startLevel) - 1] == '\n')
      levelSets[nLevelSets].startLevel[strlen(levelSets[nLevelSets].startLevel) - 1] = 0;
    fgets(levelSets[nLevelSets].startLevelName, 256, fp);
    if (levelSets[nLevelSets]
            .startLevelName[strlen(levelSets[nLevelSets].startLevelName) - 1] == '\n')
      levelSets[nLevelSets].startLevelName[strlen(levelSets[nLevelSets].startLevelName) - 1] =
          0;

    /* This reads a descriptive text about the level */
    for (lineno = 0; lineno < 5; lineno++) levelSets[nLevelSets].description[lineno][0] = 0;
    for (lineno = 0; lineno < 5; lineno++) {
      if (!fgets(levelSets[nLevelSets].description[lineno], 70, fp)) break;
      if (levelSets[nLevelSets]
              .description[lineno][strlen(levelSets[nLevelSets].description[lineno]) - 1] !=
          '\n') {
        printf("Warning - malformed descriptions for levelset %s\n", setname);
      }
      /* Remove final newline character */
      levelSets[nLevelSets]
          .description[lineno][strlen(levelSets[nLevelSets].description[lineno]) - 1] = 0;
    }

    strncpy(imagename, setname, sizeof(imagename));
    strcpy(imagename + strlen(imagename) - 4, ".jpg");
    fp2 = fopen(setname, "r");
    if (fp2)
      fclose(fp2);
    else
      imagename[0] = 0;
    strncpy(levelSets[nLevelSets].imagename, imagename,
            sizeof(levelSets[nLevelSets].imagename));

    nLevelSets++;
    fclose(fp);
  }
}

void Settings::save() {
  char str[256];

  snprintf(str, sizeof(str) - 1, "%s/.trackballs", getenv("HOME"));
  if (pathIsLink(str)) {
    fprintf(stderr, _("Error, %s is a symbolic link. Cannot save settings\n"), str);
    return;
  }

  mkdir(str, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);
  snprintf(str, sizeof(str) - 1, "%s/.trackballs/settings", getenv("HOME"));
  if (pathIsLink(str)) {
    fprintf(stderr, _("Error, %s is a symbolic link. Cannot save settings\n"), str);
    return;
  }

  /* Save all settings here */
  FILE *fp = fopen(str, "w");
  if (!fp) {
    printf("Warning. Could not save settings.\n");
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
    fprintf(fp, "(language %d)\n", language);
    fprintf(fp, "(vsync-on %d)\n", vsynced);
    fclose(fp);
  }
}

int Settings::hasJoystick() {
  if (!Settings::settings->joystickIndex) return 0;

  if (!joystick) joystick = SDL_JoystickOpen(Settings::settings->joystickIndex - 1);
  if (!joystick) {
    printf("Warning, failed to open joystick no. %d\n", Settings::settings->joystickIndex - 1);
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
    printf("Warning, failed to open joystick no. %d\n", Settings::settings->joystickIndex - 1);
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
    printf("Warning, failed to open joystick no. %d\n", Settings::settings->joystickIndex - 1);
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

  /*||
        SDL_JoystickGetButton(joystick,1) ||
        SDL_JoystickGetButton(joystick,2) ||
        SDL_JoystickGetButton(joystick,3);  */
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
      fprintf(stderr, "Warning: locale directory %s missing.\n", localedir);
      // language=0;
    }
  }

  /* Set the locale, trying several name options */
  printf("setLocale: %s ", languageCodes[language][0]);
  char *ret = NULL;
  for (int i = 0; i < 3; i++) {
    ret = setlocale(LC_MESSAGES, languageCodes[language][i]);
    if (ret) { break; }
  }
  printf("-> %s\n", ret);
}
