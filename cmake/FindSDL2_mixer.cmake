find_package(PkgConfig)
pkg_check_modules(PC_SDL2_MIXER QUIET SDL2_mixer)

set(SDL2_DEFINITIONS ${PC_SDL2_MIXER_CFLAGS_OTHER})

find_path(SDL2_MIXER_INCLUDE_DIR SDL2/SDL_mixer.h
          HINTS ${PC_SDL2_MIXER_INCLUDEDIR} ${PC_SDL2_MIXER_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )

find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer libSDL2_mixer
             HINTS ${PC_SDL2_MIXER_LIBDIR} ${PC_SDL2_MIXER_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer  REQUIRED_VARS
                                  SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_MIXER_VERSION)

set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY} )
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR} )
mark_as_advanced(SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)
