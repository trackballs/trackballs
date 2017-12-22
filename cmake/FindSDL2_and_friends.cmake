# Locate SDL2, SDL2_image, SDL_mixer, and SDL_ttf
# Nothing gets marked `advanced` since there aren't that many variables

# PkgConfig is only there to provide hints
find_package(PkgConfig)
pkg_check_modules(PC_SDL2 QUIET sdl2)
pkg_check_modules(PC_SDL2_IMAGE QUIET SDL2_image)
pkg_check_modules(PC_SDL2_MIXER QUIET SDL2_mixer)
pkg_check_modules(PC_SDL2_TTF QUIET SDL2_ttf)

set(SDL2_DEFINITIONS ${PC_SDL2_CFLAGS_OTHER} ${PC_SDL2_IMAGE_CFLAGS_OTHER}
            ${PC_SDL2_MIXER_CFLAGS_OTHER} ${PC_SDL2_TTF_CFLAGS_OTHER})

find_path(SDL2_INCLUDE_DIR SDL.h
          HINTS ${PC_SDL2_INCLUDEDIR} ${PC_SDL2_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )
find_path(SDL2_IMAGE_INCLUDE_DIR SDL2/SDL_image.h
          HINTS ${PC_SDL2_IMAGE_INCLUDEDIR} ${PC_SDL2_IMAGE_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )
find_path(SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h
          HINTS ${PC_SDL2_MIXER_INCLUDEDIR} ${PC_SDL2_MIXER_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )
find_path(SDL2_TTF_INCLUDE_DIR SDL2/SDL_ttf.h
          HINTS ${PC_SDL2_TTF_INCLUDEDIR} ${PC_SDL2_TTF_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )

find_library(SDL2_LIBRARY NAMES SDL2 libSDL2
             HINTS ${PC_SDL2_LIBDIR} ${PC_SDL2_LIBRARY_DIRS} )
find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image libSDL2_image
             HINTS ${PC_SDL2_IMAGE_LIBDIR} ${PC_SDL2_IMAGE_LIBRARY_DIRS} )
find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer libSDL2_mixer
             HINTS ${PC_SDL2_MIXER_LIBDIR} ${PC_SDL2_MIXER_LIBRARY_DIRS} )
find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf libSDL2_ttf
             HINTS ${PC_SDL2_TTF_LIBDIR} ${PC_SDL2_TTF_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libSDL2  REQUIRED_VARS
                                  SDL2_LIBRARY SDL2_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_VERSION)
find_package_handle_standard_args(libSDL2_image  REQUIRED_VARS
                                  SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_IMAGE_VERSION)
find_package_handle_standard_args(libSDL2_mixer  REQUIRED_VARS
                                  SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_MIXER_VERSION)
find_package_handle_standard_args(libSDL2_ttf  REQUIRED_VARS
                                  SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_TTF_VERSION)

set(SDL2_LIBRARIES ${SDL2_LIBRARY} )
set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY} )
set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY} )
set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY} )
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR} )
set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR} )
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR} )
set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR} )
mark_as_advanced(SDL2_LIBRARY SDL2_INCLUDE_DIR
                 SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR
                 SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR
                 SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)
