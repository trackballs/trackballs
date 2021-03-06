find_package(PkgConfig)
pkg_check_modules(PC_SDL2_TTF QUIET SDL2_ttf)

set(SDL2_DEFINITIONS ${PC_SDL2_TTF_CFLAGS_OTHER})

find_path(SDL2_TTF_INCLUDE_DIR SDL2/SDL_ttf.h
          HINTS ${PC_SDL2_TTF_INCLUDEDIR} ${PC_SDL2_TTF_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )

find_library(SDL2_TTF_LIBRARY NAMES SDL2_ttf libSDL2_ttf
             HINTS ${PC_SDL2_TTF_LIBDIR} ${PC_SDL2_TTF_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_ttf  REQUIRED_VARS
                                  SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_TTF_VERSION)

set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY} )
set(SDL2_TTF_INCLUDE_DIRS ${SDL2_TTF_INCLUDE_DIR} )
mark_as_advanced(SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)
