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

#include "calibrateJoystickMode.h"
#include "editMode.h"
#include "enterHighScoreMode.h"
#include "font.h"
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
#include <queue>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <shlobj.h>
#include <windows.h>
#endif

/* Important globals */
static SDL_Window *window = NULL;
int debug_joystick, repair_joystick;
static int not_yet_windowed = 1;
struct timespec displayStartTime, lastDisplayStartTime;
static bool has_audio = true;
static bool startInEditMode;

int displayFrameNumber = 0;

char effectiveShareDir[256];
char effectiveLocalDir[256];
char username[256];
int screenResolutions[5][2] = {{640, 480},
                               {800, 600},
                               {1024, 768},
                               {1280, 1024},
                               {1600, 1200}},
    nScreenResolutions = 5;

void changeScreenResolution() {
  int full = Settings::settings->is_windowed == 0;
  int fixed = Settings::settings->resolution >= 0;

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

  /* Use CapsLock key to determine if mouse should be hidden */
  if (SDL_GetModState() & KMOD_CAPS) {
    SDL_ShowCursor(SDL_ENABLE);
  } else {
    SDL_ShowCursor(SDL_DISABLE);
  }

  /* Depending on platform, this might work, or not, or fail silently */
  SDL_GL_SetSwapInterval(Settings::settings->vsynced ? 1 : 0);
}

static SDL_GLContext createWindow() {
  int full = Settings::settings->is_windowed == 0;
  int fixed = Settings::settings->resolution >= 0;

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

  SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);

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
    return NULL;
  }

  /* Because windowing functions are only reliable on the main thread (e.g. X11,
   * without special thread init code), the OpenGL context is created here. In
   * order to use the OpenGL context on the off-thread, we call
   * `SDL_GL_MakeCurrent` there; because GLES (for Wayland) contexts are limited
   * to a single thread, we clear the current (thread) context; see also
   * the SDL source code. */
  SDL_GLContext ctx = SDL_GL_CreateContext(window);
  if (!ctx) { error("Failed to create OpenGL context: %s\n", SDL_GetError()); }
  SDL_GL_MakeCurrent(window, NULL);

  char str[256];
  snprintf(str, sizeof(str), "%s/icons/trackballs-128x128.png", effectiveShareDir);
  SDL_Surface *wmIcon = IMG_Load(str);
  if (wmIcon) {
    SDL_SetWindowIcon(window, wmIcon);
    SDL_FreeSurface(wmIcon);
  }

  changeScreenResolution();
  SDL_GetWindowSize(window, &screenWidth, &screenHeight);

  return ctx;
}

static void print_usage(FILE *stream, const char *program_name) {
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

static int testDir() {
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

static void *mainLoop(void *data) {
  /* OpenGL work is now *only* performed on this thread. */
  SDL_GLContext context = (SDL_GLContext)data;
  int r = SDL_GL_MakeCurrent(window, context);
  if (r < 0) { error("Failed to make OpenGL context current: %s", SDL_GetError()); }
  SDL_GL_SetSwapInterval(Settings::settings->vsynced ? 1 : 0);

  const GLubyte *gl_version = glGetString(GL_VERSION);
  if (!gl_version) {
    error("OpenGL context does not work (cannot even obtain GL_VERSION string)");
  }

  /* initialize OpenGL setup before we draw anything */
  glHelpInit();

  // set the name of the window
  struct timespec bootStart = getMonotonicTime();
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
                    texcoord[3], Color(1., 1., 1., 1.), splashTexture);
    Leave2DMode();
    SDL_GL_SwapWindow(window);
  }

  /* Initialize all modules */
  initGuileInterface();
  generalInit();

  if (has_audio) soundInit();
  Settings::settings->loadLevelSets();

  /* Initialize and activate the correct gameModes */
  if (startInEditMode) {
    GameMode::activate(EditMode::init());
  } else {
    /* Activate initial mode */
    if (Settings::settings->doSpecialLevel) {
      GameMode::activate(SetupMode::init());
    } else {
      GameMode::activate(MenuMode::init());
    }
    volumeChanged();
  }

  /* Make sure splashscreen has been shown for atleast 1.5 seconds */
  struct timespec timeNow = getMonotonicTime();
  while (getTimeDifference(bootStart, timeNow) < 1.5) {
    glClear(GL_COLOR_BUFFER_BIT);
    Enter2DMode();
    draw2DRectangle(0, 0, screenWidth, screenHeight, texcoord[0], texcoord[1], texcoord[2],
                    texcoord[3], Color(1., 1., 1., 1.), splashTexture);
    Leave2DMode();
    SDL_GL_SwapWindow(window);
    timeNow = getMonotonicTime();
  }
  glDeleteTextures(1, &splashTexture);

  warnForGLerrors("uncaught from initialization");

  /*                 */
  /* Main event loop */
  /*                 */

  /* Initialize random number generator */
  int seed = getMonotonicTime().tv_nsec;
  srand(seed);

  double logic_time = 0.;
  Uint32 zero_sdl_tick = SDL_GetTicks();
  double render_time = 0.;
  while (GameMode::current) {
    double td = PHYSICS_RESOLUTION;

    /* When game logic has caught up to real time, display */
    render_time = (SDL_GetTicks() - zero_sdl_tick) * 0.001;
    /* Never process more than 100msec of updates before drawing, to ensure
     * the window is still somewhat responsive in extreme lag scenarios. */
    logic_time = std::max(logic_time, render_time - 0.100);

    while (logic_time < render_time) {
      logic_time += td / timeDilationFactor;

      if (GameMode::current) { GameMode::current->tick(td); }
    }

    {
      displayFrameNumber++;

      /* Make sure music is still playing */
      soundIdle(td);

      /* update font system */
      update2DStringCache(false);

      /* Draw world */
      glViewport(0, 0, screenWidth, screenHeight);
      if (GameMode::current) {
        GameMode::current->display();
      } else {
        glClear(GL_COLOR_BUFFER_BIT);
      }
      warnForGLerrors("uncaught from display routine");
      SDL_GL_SetSwapInterval(Settings::settings->vsynced ? 1 : 0);
      SDL_GL_SwapWindow(window);

      lastDisplayStartTime = displayStartTime;
      displayStartTime = getMonotonicTime();
      /* Expensive computations has to be done *after* tick+draw to keep world in good
         synchronisation. */
      if (GameMode::current) GameMode::current->doExpensiveComputations();
      warnForGLerrors("uncaught from expensive computation");
    }

    /* Respond to events between this logic update and the next */
    // TODO: react to window events *immediately* -- use two queues?
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        GameMode::activate(NULL);
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (GameMode::current)
          GameMode::current->mouseDown(event.button.button, event.button.x, event.button.y);
        break;
      case SDL_KEYUP:
        /* Use Caps lock key to determine if mouse should be hidden+grabbed */
        if (event.key.keysym.sym == SDLK_CAPSLOCK) {
          if (SDL_GetModState() & KMOD_CAPS) {
            SDL_ShowCursor(SDL_ENABLE);
          } else {
            SDL_ShowCursor(SDL_DISABLE);
          }
        } else if (GameMode::current) {
          GameMode::current->keyUp(event.key.keysym.sym);
        }
        break;
      case SDL_KEYDOWN:

        /* Always quit if the 'q' key is pressed */
        if (event.key.keysym.sym == 'q' && SDL_GetModState() & KMOD_CTRL) {
          GameMode::activate(NULL);
        }

        /* Change between fullscreen/windowed mode if the 'f' key
           is pressed */
        else if (event.key.keysym.sym == 'f' && SDL_GetModState() & KMOD_CTRL) {
          Settings::settings->is_windowed = Settings::settings->is_windowed ? 0 : 1;
          changeScreenResolution();
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
          if (EditMode::editMode && GameMode::current == EditMode::editMode) {
            EditMode::editMode->askQuit();
          } else if ((GameMode::current && GameMode::current == MenuMode::menuMode))
            GameMode::activate(NULL);
          else { GameMode::activate(MenuMode::init()); }

        } else if (GameMode::current) {
          /* Prevent repeated keys */
          if (event.key.repeat) { break; }

          GameMode::current->key(event.key.keysym.sym);
        }

        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          if (Settings::settings->resolution >= 0) {
            // Assume fixed resolution unconditionally
            screenWidth = screenResolutions[Settings::settings->resolution][0];
            screenHeight = screenResolutions[Settings::settings->resolution][1];
          } else {
            // Region drawn resizes with window
            screenWidth = event.window.data1;
            screenHeight = event.window.data2;
          }

          /* Adjust for size change in editmode */
          if (EditMode::editMode) { EditMode::editMode->resizeWindows(); }
        }
        break;
      }
    }
  }

  Settings::settings->closeJoystick();
  Settings::settings->save();

  /* Delete all game modes */
  CalibrateJoystickMode::cleanup();
  EditMode::cleanup();
  EnterHighScoreMode::cleanup();
  HallOfFameMode::cleanup();
  HelpMode::cleanup();
  MainMode::cleanup();
  MenuMode::cleanup();
  SettingsMode::cleanup();
  SetupMode::cleanup();

  HighScore::cleanup();
  glHelpCleanup();

  SDL_GL_DeleteContext(context);

  return (void *)EXIT_SUCCESS;
}

static bool setupLocaleAndTranslations() {
  /* Initialialize i18n with system defaults, before anything else (eg. settings) have been
   * loaded */
  setlocale(LC_ALL, "");
  char localedir[512];

#ifdef LOCALEDIR
  snprintf(localedir, 511, "%s", LOCALEDIR);
#else
  snprintf(localedir, 511, "%s/locale", effectiveShareDir);
#endif
  if (bindtextdomain(PACKAGE, localedir) == NULL) {
    warning("Failed to set text domain directory to %s", localedir);
    return false;
  }
  if (bind_textdomain_codeset(PACKAGE, "UTF-8") == NULL) {
    warning("Failed to set text codeset to UTF-8");
    return false;
  }
  if (textdomain(PACKAGE) == NULL) {
    warning("Failed to set message domain");
    return false;
  }
  return true;
}

static bool setupEnvAndPaths(const char *program_name) {
  /* Store the username, for use as a default player/level author name */
  const char *env_user = getenv("USER");
  if (env_user) {
    snprintf(username, sizeof(username) - 1, "%s", env_user);
  }
#ifdef WIN32
  else {
    const char *env_username = getenv("USERNAME");
    if (env_username) { snprintf(name, 20, "%s", env_username); }
  }
#endif

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

  snprintf(effectiveLocalDir, sizeof(effectiveLocalDir), "%s/.trackballs", getenv("HOME"));
  if (pathIsLink(effectiveLocalDir)) {
    warning("Error, %s is a symbolic link. Cannot save settings", effectiveLocalDir);
    return EXIT_FAILURE;
  }
  if (!pathIsDir(effectiveLocalDir))
    mkdir(effectiveLocalDir, S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IWGRP);

  if (NULL == getenv("GUILE_LOAD_PATH")) {
    static char
        guileLoadPath[256 + 16]; /*longest effective share directory plus"GUILE_LOAD_PATH="*/
    snprintf(guileLoadPath, sizeof(guileLoadPath), "GUILE_LOAD_PATH=%s", effectiveShareDir);
    putenv(guileLoadPath);
  }

#ifdef WIN32
  if (NULL == getenv("HOME")) {
    static char homeEnv[MAX_PATH + 5];
    char homePath[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, homePath);
    snprintf(homeEnv, sizeof(homeEnv), "HOME=%s", homePath);
    for (int i = strlen(homeEnv) - 1; i >= 0; i--)
      if (homeEnv[i] == '\\') homeEnv[i] = '/';
    // printf("'%s'\n", homeEnv);
    putenv(homeEnv);
  }
#endif
  return true;
}

int main(int argc, char **argv) {
  const char *program_name = argv[0];

  if (!setupEnvAndPaths(program_name)) { return EXIT_FAILURE; }

  if (!setupLocaleAndTranslations()) { return EXIT_FAILURE; }

  /* This calls scm_with_guile which initializes guile which requires env setup */
  Settings::init();

  startInEditMode = false;
  bool touchMode = false;
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

  displayStartTime = getMonotonicTime();
  lastDisplayStartTime = displayStartTime;
  lastDisplayStartTime.tv_sec -= 1;
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

    int i;
    switch (next_option) {
    case 'h':
      print_usage(stdout, program_name);
      return EXIT_SUCCESS;
    case 'l':
      snprintf(Settings::settings->specialLevel, sizeof(Settings::settings->specialLevel) - 1,
               "%s", optarg);
      Settings::settings->doSpecialLevel = 1;
      break;
    case 't':
      touchMode = true;
      touchName = optarg;
      has_audio = false;  // no audio
      break;
    case 'w':
      settings->is_windowed = 1;
      break;
    case 'm':
      has_audio = false;
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
      print_usage(stderr, program_name);
      return EXIT_FAILURE;
    case -1:
      break;
    case 'v':
      printf("%s v%s\n", PACKAGE, VERSION);
      return EXIT_SUCCESS;
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
      print_usage(stderr, program_name);
      return EXIT_FAILURE;
    }
  } while (next_option != -1);

  printf("%s\n", _("Welcome to Trackballs."));
  char str[256];
  snprintf(str, 255, _("Using %s as gamedata directory."), effectiveShareDir);
  printf("%s\n", str);

  if (touchMode) {
    /* We do not need any of SDL to load and save a map */
    char mapname[512];

    snprintf(mapname, sizeof(mapname) - 1, "%s/levels/%s.map", effectiveLocalDir, touchName);
    if (!fileExists(mapname))
      snprintf(mapname, sizeof(mapname), "%s/levels/%s.map", effectiveShareDir, touchName);
    if (!fileExists(mapname)) snprintf(mapname, sizeof(mapname), "%s", touchName);
    printf("Touching map %s\n", mapname);
    Map *map = new Map(mapname);
    map->save(mapname, (int)map->startPosition[0], (int)map->startPosition[1]);
    delete map;
    return EXIT_SUCCESS;
  }

  /* Initialize SDL */
  if ((SDL_Init(SDL_INIT_VIDEO | (has_audio ? SDL_INIT_AUDIO : 0) | SDL_INIT_JOYSTICK) ==
       -1)) {
    error("Could not initialize libSDL. Error message: '%s'. Try '-m' if audio is at fault.",
          SDL_GetError());
  }
  atexit(SDL_Quit);

  SDL_GLContext gl_context = createWindow();
  if (!gl_context) {
    error("Could not initialize screen resolution (message: '%s')", SDL_GetError());
  }

  if (SDL_GetModState() & KMOD_CAPS) {
    warning("capslock is on, the mouse will be visible and not grabbed");
  }

  void *v = mainLoop(gl_context);
  int retval = (int)(ptrdiff_t)v;

  SDL_Quit();
  return retval;
}
