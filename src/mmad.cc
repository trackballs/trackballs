/* main.cc
   Contains the main of the game. See README for a description of the game.

   Copyright (C) 2000-2006  Mathias Broxvall
                            Yannick Perret

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

#include "ball.h"
#include "calibrateJoystickMode.h"
#include "editMode.h"
#include "enterHighScoreMode.h"
#include "font.h"
#include "forcefield.h"
#include "game.h"
#include "gameMode.h"
#include "general.h"
#include "glHelp.h"
#include "guile.h"
#include "helpMode.h"
#include "highScore.h"
#include "hofMode.h"
#include "mainMode.h"
#include "map.h"
#include "menuMode.h"
#include "pipe.h"
#include "pipeConnector.h"
#include "settings.h"
#include "settingsMode.h"
#include "setupMode.h"
#include "sound.h"

#include <SDL2/SDL_image.h>
#include <dirent.h>
#include <getopt.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <shlobj.h>
#include <windows.h>
#endif
using namespace std;

/* Important globals */
SDL_Window *window = NULL;
SDL_Surface *screen = NULL;
SDL_GLContext mainContext;
const char *program_name;
int debug_joystick, repair_joystick;
int not_yet_windowed = 1;
double displayStartTime = 0.;

int theFrameNumber = 0;

char effectiveShareDir[256];
int screenResolutions[5][2] = {{640, 480},
                               {800, 600},
                               {1024, 768},
                               {1280, 1024},
                               {1600, 1200}},
    nScreenResolutions = 5;

void changeScreenResolution() {
  int full = Settings::settings->is_windowed == 0;
  int fixed = Settings::settings->resolution >= 0;

  if (window == NULL) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "/%s/ V%s", PACKAGE, VERSION);

    if (Settings::settings->colorDepth == 16) {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    } else {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    /* Version 3.3 is required for UNSIGNED_INT_2_10_10_10_REV packing method
     * for the VertexAttribPointer function; the 3.2 spec does not permit it */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    /* Uncomment to apply basic antialiasing.
     * SDL can't recover from failure to obtain this if asked for */
    //    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Start at default size; will be adjusted
    int windowHeight, windowWidth;
    Uint32 flags = SDL_WINDOW_OPENGL;
    if (fixed) {
      windowWidth = screenResolutions[Settings::settings->resolution][0];
      windowHeight = screenResolutions[Settings::settings->resolution][1];
      if (full) { flags |= SDL_WINDOW_FULLSCREEN; }
    } else {
      if (full) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      } else {
        flags |= SDL_WINDOW_RESIZABLE;
      }

      SDL_Rect disprect;
      if (!SDL_GetDisplayBounds(0, &disprect)) {
        windowWidth = disprect.w / 2;
        windowHeight = disprect.h / 2;
      } else {
        windowWidth = 800;
        windowHeight = 600;
      }
    }
    not_yet_windowed = full;

    window = SDL_CreateWindow(buffer, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              windowWidth, windowHeight, flags);

    if (window == NULL) {
      warning("Could not create window: %s", SDL_GetError());
      return;
    }

    mainContext = SDL_GL_CreateContext(window);

    char str[256];
    snprintf(str, sizeof(str), "%s/icons/trackballs-32x32.png", effectiveShareDir);
    SDL_Surface *wmIcon = IMG_Load(str);
    if (wmIcon) {
      SDL_SetWindowIcon(window, wmIcon);
      SDL_FreeSurface(wmIcon);
    }
  }

  if (fixed) {
    SDL_SetWindowSize(window, screenResolutions[Settings::settings->resolution][0],
                      screenResolutions[Settings::settings->resolution][1]);
    if (full) {
      SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    } else {
      SDL_SetWindowFullscreen(window, 0);
#if SDL_PATCHLEVEL >= 5 && SDL_MAJOR_VERSION >= 2 && SDL_MINOR_VERSION >= 0
      SDL_SetWindowResizable(window, SDL_FALSE);
#endif
      if (not_yet_windowed) {
        not_yet_windowed = 0;
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
      }
    }
  } else {
    if (full) {
      SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
      SDL_SetWindowFullscreen(window, 0);
      if (not_yet_windowed) {
        not_yet_windowed = 0;
        SDL_Rect disprect;
        if (!SDL_GetDisplayBounds(0, &disprect)) {
          SDL_SetWindowSize(window, disprect.w / 2, disprect.h / 2);
        } else {
          SDL_SetWindowSize(window, 800, 600);
        }
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
      }
#if SDL_PATCHLEVEL >= 5 && SDL_MAJOR_VERSION >= 2 && SDL_MINOR_VERSION >= 0
      SDL_SetWindowResizable(window, SDL_TRUE);
#endif
    }
  }

  screen = SDL_GetWindowSurface(window);

  if (!screen) return;
  /* Pick up what the screen actually has */
  screenWidth = screen->w;
  screenHeight = screen->h;

  /* Use CapsLock key to determine if mouse should be hidden */
  if (SDL_GetModState() & KMOD_CAPS) {
    SDL_ShowCursor(SDL_ENABLE);
  } else {
    SDL_ShowCursor(SDL_DISABLE);
  }
  SDL_GL_SetSwapInterval(Settings::settings->vsynced ? 1 : 0);

  /* Adjust for size change in editmode */
  if (EditMode::editMode) { EditMode::editMode->resizeWindows(); }

  resetTextures();
}

void print_usage(FILE *stream) {
  fprintf(stream, "%s %s %s\n", _("Usage:"), program_name,
          _("[-w, -m] [-e, -l -t <level>] [-r <width>] [-s <sensitivity>]"));
  const char *options[12][2] = {
      {"   -h  --help            ", _("Display this usage information.")},
      {"   -l  --level           ", _("Start from level.")},
      {"   -w  --windowed        ", _("Run in window (Default is fullscreen)")},
      {"   -m  --mute            ", _("Mute sound.")},
      {"   -r  --resolution      ", _("Set resolution to 640, 800 or 1024")},
      {"   -s  --sensitivity     ", _("Mouse sensitivity, default 1.0")},
      {"   -f  --fps             ", _("Displays framerate")},
      {"   -v  --version         ", _("Prints current version number")},
      {"   -t  --touch           ", _("Updates a map to the latest format")},
      {"   -y  --low-memory      ", _("Attempt to conserve memory usage")},
      {"   -9  --debug-joystick  ", _("Debug joystick status")},
      {"   -j  --repair-joystick ", _("Correct for bad joysticks")}};

  for (int i = 0; i < 12; i++) fprintf(stream, "%s%s\n", options[i][0], options[i][1]);
  fprintf(stream, "\n");
  fprintf(stream, "%s\n", _("Important keyboard shortcuts"));
  const char *shortcuts[5][2] = {{_("Escape"), _("Soft quit")},
                                 {_("CapsLock"), _("Unhide mouse pointer")},
                                 {_("CTRL-q"), _("Quit the game immediately")},
                                 {_("CTRL-f"), _("Toggle between fullscreen/windowed mode")},
                                 {"k", _("Kill the ball")}};
  size_t mxlen = 0;
  for (int i = 0; i < 5; i++) mxlen = std::max(strlen(shortcuts[i][0]), mxlen);
  char whitespace[64];
  for (int i = 0; i < 5; i++) {
    memset(whitespace, ' ', 64);
    whitespace[mxlen - strlen(shortcuts[i][0])] = 0;
    fprintf(stream, "   %s%s   %s\n", shortcuts[i][0], whitespace, shortcuts[i][1]);
  }
}

int testDir() {
  if (!strlen(effectiveShareDir)) return 0;
#ifdef WIN32
  // Really this is just done for asthetics
  for (int i = strlen(effectiveShareDir) - 1; i >= 0; i--)
    if (effectiveShareDir[i] == '\\') effectiveShareDir[i] = '/';
#endif
  DIR *dir = opendir(effectiveShareDir);
  // printf("Looking for %s\n", effectiveShareDir);
  if (!dir)
    return 0;
  else
    closedir(dir);
  char str[256];
  snprintf(str, sizeof(str), "%s/levels", effectiveShareDir);
  dir = opendir(str);
  if (!dir)
    return 0;
  else
    closedir(dir);
  /* TODO. Test for all other essential subdirectories */
  return 1;
}

void innerMain(void * /*closure*/, int argc, char **argv) {
  int is_running = 1;
  int editMode = 0, touchMode = 0;
  int audio = SDL_INIT_AUDIO;
  SDL_Event event;
  char *touchName = 0;

  const char *const short_options = "he:l:t:wmr:s:fqvyj";
  const struct option long_options[] = {{"help", 0, NULL, 'h'},
                                        {"level", 1, NULL, 'l'},
                                        {"windowed", 0, NULL, 'w'},
                                        {"mute", 0, NULL, 'm'},
                                        {"resolution", 1, NULL, 'r'},
                                        {"sensitivity", 1, NULL, 's'},
                                        {"fps", 0, NULL, 'f'},
                                        {"version", 0, NULL, 'v'},
                                        {"touch", 1, NULL, 't'},
                                        {"low-memory", 0, NULL, 'y'},
                                        {"debug-joystick", 0, NULL, '9'},
                                        {"repair-joystick", 0, NULL, 'j'},
                                        {NULL, 0, NULL, 0}};
  int next_option;

  program_name = argv[0];

  Settings::init();
  Settings *settings = Settings::settings;
  settings->doSpecialLevel = 0;
  settings->setLocale(); /* Start "correct" i18n as soon as possible */
  low_memory = 0;
  debug_joystick = 0;
  repair_joystick = 0;
  do {
#if defined(__SVR4) && defined(__sun)
    next_option = getopt(argc, argv, short_options);
#else
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);
#endif

    /*
#ifdef solaris
    next_option = getopt(argc,argv,short_options);
#else
    next_option = getopt_long (argc, argv, short_options,
    long_options, NULL);
#endif
    */
    int i;
    switch (next_option) {
    case 'h':
      print_usage(stdout);
      exit(0);
      break;
    case 'l':
      snprintf(Settings::settings->specialLevel, sizeof(Settings::settings->specialLevel) - 1,
               "%s", optarg);
      Settings::settings->doSpecialLevel = 1;
      break;
    case 't':
      touchMode = 1;
      touchName = optarg;
      audio = 0;  // no audio
      break;
    case 'w':
      settings->is_windowed = 1;
      break;
    case 'm':
      audio = 0;
      break;
    case 'r':
      for (i = 0; i < nScreenResolutions; i++)
        if (screenResolutions[i][0] == atoi(optarg)) break;
      if (i < nScreenResolutions)
        settings->resolution = i;
      else {
        char estr[256];
        snprintf(estr, 255, _("Unknown screen resolution of width %d"), i);
        printf("%s\n", estr);
      }
      break;
    case 's':
      Settings::settings->mouseSensitivity = atof(optarg);
      break;
    case 'f':
      Settings::settings->showFPS = 1;
      break;
    case '?':
      print_usage(stderr);
      exit(1);
    case -1:
      break;
    case 'v':
      printf("%s v%s\n", PACKAGE, VERSION);
      exit(0);
      break;
    case 'y':
      low_memory = 1;
      break;
    case '9':
      debug_joystick = 1;
      break;
    case 'j':
      repair_joystick = 1;
      break;
    default:
      abort();
    }
  } while (next_option != -1);

  printf("%s\n", _("Welcome to Trackballs."));
  char str[256];
  snprintf(str, 255, _("Using %s as gamedata directory."), effectiveShareDir);
  printf("%s\n", str);

  /* Initialize SDL */
  if ((SDL_Init(SDL_INIT_VIDEO | audio | SDL_INIT_JOYSTICK) == -1)) {
    error("Could not initialize libSDL. Error message: '%s'. Try '-m' if audio is at fault.",
          SDL_GetError());
  }
  atexit(SDL_Quit);

  // MB: Until here we are using 7 megs of memory
  changeScreenResolution();
  if (!screen) {
    error("Could not initialize screen resolution (message: '%s')", SDL_GetError());
  }
  // MB: Until here we are using 42 megs of memory

  if (SDL_GetModState() & KMOD_CAPS) {
    warning("capslock is on, the mouse will be visible and not grabbed");
  }

  /* initialize OpenGL setup before we draw anything */
  glHelpInit();  // MB: 1.5 megs

  // set the name of the window
  double bootStart = ((double)SDL_GetTicks()) / 1000.0;
  SDL_Surface *splashScreen = loadImage("splashScreen.jpg");
  glViewport(0, 0, screenWidth, screenHeight);

  // Draw the splash screen
  GLfloat texcoord[4];
  GLuint splashTexture = LoadTexture(splashScreen, texcoord);
  SDL_FreeSurface(splashScreen);
  for (int i = 0; i < 2; i++) {
    glClear(GL_COLOR_BUFFER_BIT);
    Enter2DMode();
    draw2DRectangle(0, 0, screenWidth, screenHeight, texcoord[0], texcoord[1], texcoord[2],
                    texcoord[3], 1., 1., 1., 1., splashTexture);
    Leave2DMode();
    SDL_GL_SwapWindow(window);
  }

  // MB: Until here we are using 47 megs of memory.
  // splashscreen is using 5 megs but it is ok since we are freeing it before the real game

  /* Initialize all modules */
  initGuileInterface();  // MB: 0 megs
  generalInit();         // MB: 0 megs

  // MB: Until here we are using 49 megs of memory.

  if (audio != 0) soundInit();
  Settings::settings->loadLevelSets();

  // MB: Until here we are using 51 megs of memory.

  /* Initialize and activate the correct gameModes */
  if (touchMode) {
    char mapname[512];

    snprintf(mapname, sizeof(mapname) - 1, "%s/.trackballs/levels/%s.map", getenv("HOME"),
             touchName);
    if (!fileExists(mapname))
      snprintf(mapname, sizeof(mapname), "%s/levels/%s.map", effectiveShareDir, touchName);
    if (!fileExists(mapname)) snprintf(mapname, sizeof(mapname), "%s", touchName);
    printf("Touching map %s\n", mapname);
    Map *map = new Map(mapname);
    map->save(mapname, (int)map->startPosition[0], (int)map->startPosition[1]);
    exit(0);
  } else if (editMode) {
    EditMode::init();
    Font::init();
    GameMode::activate(EditMode::editMode);
  } else {
    // MB: Reminder 51 megs it was
    Font::init();      // MB: Until here we are using 54 megs of memory.
    MenuMode::init();  // MB: Until here we are using 55 megs of memory.

    MainMode::init();
    HighScore::init();  // MB: 55
    EditMode::init();

    EnterHighScoreMode::init();              // MB: 58
    HallOfFameMode::init();                  // MB: 62
    SettingsMode::init();                    // MB: 64
    GameMode::activate(MenuMode::menuMode);  // MB: 65
    GameHook::init();
    CalibrateJoystickMode::init();

    SetupMode::init();  // MB: 71
    Ball::init();
    ForceField::init();  // MB: 71
    HelpMode::init();    // MB: 74
    Pipe::init();
    PipeConnector::init();
    volumeChanged();

    // Until here 74 megs
  }

  /* Make sure splahsscreen has been shown for atleast 1.5 seconds */
  double timeNow = ((double)SDL_GetTicks()) / 1000.0;
  while (timeNow < bootStart + 1.5) {
    glClear(GL_COLOR_BUFFER_BIT);
    Enter2DMode();
    draw2DRectangle(0, 0, screenWidth, screenHeight, texcoord[0], texcoord[1], texcoord[2],
                    texcoord[3], 1., 1., 1., 1., splashTexture);
    Leave2DMode();
    SDL_GL_SwapWindow(window);
    timeNow = ((double)SDL_GetTicks()) / 1000.0;
  }
  glDeleteTextures(1, &splashTexture);

  warnForGLerrors("uncaught from initialization");

  /*                 */
  /* Main event loop */
  /*                 */

  double oldTime, t, td;
  oldTime = ((double)SDL_GetTicks()) / 1000.0;

  /* Initialize random number generator */
  int seed = (int)getSystemTime();
  srand(seed);
  int keyUpReceived = 1;

  while (is_running) {
    theFrameNumber++;

    t = ((double)SDL_GetTicks()) / 1000.0;
    td = t - oldTime;
    oldTime = t;
    // reduced to 5 fps
    if (td > 0.2) td = 0.2;
    // we may also add a bottom limit to 'td' to prevent
    //  precision troubles in physic computation
    /*
    // limited to 75 fps
    if (td < 0.013) td = 0.013;
    */

    /* update font system */
    Font::tick(td);

    /* Update world */
    if (GameMode::current) GameMode::current->idle(td);

    /* Make sure music is still playing */
    soundIdle(td);

    /* Draw world */
    glViewport(0, 0, screenWidth, screenHeight);
    if (GameMode::current)
      GameMode::current->display();
    else {
      glClear(GL_COLOR_BUFFER_BIT);
    }
    warnForGLerrors("uncaught from display routine");
    SDL_GL_SwapWindow(window);

    displayStartTime = getSystemTime();
    /* Expensive computations has to be done *after* tick+draw to keep world in good
       synchronisation. */
    if (GameMode::current) GameMode::current->doExpensiveComputations();
    warnForGLerrors("uncaught from expensive computation");

    /*                */
    /* Process events */
    /*                */

    /* Joystick - this was a bug since joystick data are also queried using SDL_JoystickUpdate
     */
    /*
    if(Settings::settings->hasJoystick()) SDL_JoystickEventState(SDL_ENABLE);
    else SDL_JoystickEventState(SDL_DISABLE);
    */

    SDL_MouseButtonEvent *e = (SDL_MouseButtonEvent *)&event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        is_running = 0;
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (GameMode::current) GameMode::current->mouseDown(e->button, e->x, e->y);
        break;
      case SDL_KEYUP:
        /* Prevent repeated keys */
        keyUpReceived = 1;

        /* Use Caps lock key to determine if mouse should be hidden+grabbed */
        if (event.key.keysym.sym == SDLK_CAPSLOCK) {
          if (SDL_GetModState() & KMOD_CAPS) {
            SDL_ShowCursor(SDL_ENABLE);
          } else {
            SDL_ShowCursor(SDL_DISABLE);
          }
        } else
          GameMode::current->keyUp(event.key.keysym.sym);
        break;
      case SDL_KEYDOWN:

        /* Always quit if the 'q' key is pressed */
        if (event.key.keysym.sym == 'q' && SDL_GetModState() & KMOD_CTRL) exit(0);

        /* Change between fullscreen/windowed mode if the 'f' key
           is pressed */
        else if (event.key.keysym.sym == 'f' && SDL_GetModState() & KMOD_CTRL) {
          Settings::settings->is_windowed = Settings::settings->is_windowed ? 0 : 1;
          changeScreenResolution();
          /* Flush all events that occured while switching screen resolution */
          while (SDL_PollEvent(&event)) {}
        }

        /* Use CapsLock key to determine if mouse should be hidden+grabbed */
        else if (event.key.keysym.sym == SDLK_CAPSLOCK) {
          if (SDL_GetModState() & KMOD_CAPS) {
            SDL_ShowCursor(SDL_ENABLE);
          } else {
            SDL_ShowCursor(SDL_DISABLE);
          }
        }

        else if (event.key.keysym.sym == SDLK_ESCAPE) {
          if (editMode) {
            ((EditMode *)GameMode::current)->askQuit();
          } else if ((GameMode::current && GameMode::current == MenuMode::menuMode))
            is_running = 0;
          else {
            GameMode::activate(MenuMode::menuMode);
            /* Flush all events that occured while switching screen
             resolution */
            while (SDL_PollEvent(&event)) {}
          }

        } else if (GameMode::current) {
          /* Prevent repeated keys */
          if (!keyUpReceived) break;
          keyUpReceived = 0;

          GameMode::current->key(event.key.keysym.sym);
        }

        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          /* Only change screen resolution if resizing is enabled */
          if (Settings::settings->resolution < 0) {
            changeScreenResolution();
            /* Flush all events that occured while switching screen resolution */
            while (SDL_PollEvent(&event)) {}
          }
        }
        break;
      }
    }
  }

  Settings::settings->closeJoystick();
  Settings::settings->save();
  if (GameMode::current) delete (GameMode::current);

  /* TODO. Delete all gamemodes, including the editor */
  glHelpCleanup();
  SDL_Quit();
}

int main(int argc, char **argv) {
  char guileLoadPath[256 + 16]; /*longest effective share directory plus"GUILE_LOAD_PATH="*/
  program_name = argv[0];

  /*** Autmatic detection of SHARE_DIR ***/
  effectiveShareDir[0] = 0;
  /* From environment variable */
  char *evar = getenv("TRACKBALLS");
  if (evar && strlen(evar) > 0)
    snprintf(effectiveShareDir, sizeof(effectiveShareDir) - 1, "%s", evar);
  // printf("Looking for %s\n", effectiveShareDir);
  if (!testDir()) {
    char thisDir[256];
    /* From arg0/share/trackballs  */
    snprintf(thisDir, sizeof(thisDir), "%s", program_name);
    int i;
    for (i = strlen(thisDir) - 1; i >= 0; i--)
      if (thisDir[i] == '/'
#ifdef WIN32
          || thisDir[i] == '\\'
#endif
          )
        break;
    if (i >= 0) thisDir[i] = 0;

    /*If no directory breaks are found just use the current directory*/
    if (i <= 0) snprintf(thisDir, sizeof(thisDir), ".");

    snprintf(effectiveShareDir, sizeof(effectiveShareDir), "%s/share/trackballs", thisDir);

    if (!testDir()) {
      /* From arg0/../share/trackballs */
      snprintf(effectiveShareDir, sizeof(effectiveShareDir), "%s/../share/trackballs",
               thisDir);

      if (!testDir()) {
        /* From arg0/share */
        snprintf(effectiveShareDir, sizeof(effectiveShareDir), "%s/share", thisDir);

        if (!testDir()) {
          /* From arg0/../share */
          snprintf(effectiveShareDir, sizeof(effectiveShareDir), "%s/../share", thisDir);

          if (!testDir()) {
            /* From compilation default */
            snprintf(effectiveShareDir, sizeof(effectiveShareDir), "%s", SHARE_DIR);

            if (!testDir()) {
              error("Could not find resource directory(%s)\n", effectiveShareDir);
            }
          }
        }
      }
    }
  }

  if (NULL == getenv("GUILE_LOAD_PATH")) {
    snprintf(guileLoadPath, sizeof(guileLoadPath), "GUILE_LOAD_PATH=%s", effectiveShareDir);
    putenv(guileLoadPath);
  }

#ifdef WIN32
  if (NULL == getenv("HOME")) {
    char homePath[MAX_PATH];
    char homeEnv[MAX_PATH + 5];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, homePath);
    snprintf(homeEnv, sizeof(homeEnv), "HOME=%s", homePath);
    for (int i = strlen(homeEnv) - 1; i >= 0; i--)
      if (homeEnv[i] == '\\') homeEnv[i] = '/';
    // printf("'%s'\n", homeEnv);
    putenv(homeEnv);
  }
#endif

  /* Initialialize i18n with system defaults, before anything else (eg. settings) have been
   * loaded */
  setlocale(LC_ALL, "");
  char localedir[512];

#ifdef LOCALEDIR
  snprintf(localedir, 511, "%s", LOCALEDIR);
#else
  snprintf(localedir, 511, "%s/locale", effectiveShareDir);
#endif
  bindtextdomain(PACKAGE, localedir);
  textdomain(PACKAGE);

  scm_boot_guile(argc, argv, innerMain, 0);
  return 0;
}
