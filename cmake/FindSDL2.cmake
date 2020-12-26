find_package(PkgConfig)
pkg_check_modules(PC_SDL2 QUIET sdl2)

set(SDL2_DEFINITIONS ${PC_SDL2_CFLAGS_OTHER})

find_path(SDL2_INCLUDE_DIR SDL.h
          HINTS ${PC_SDL2_INCLUDEDIR} ${PC_SDL2_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )

find_library(SDL2_LIBRARY NAMES SDL2 libSDL2
             HINTS ${PC_SDL2_LIBDIR} ${PC_SDL2_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2  REQUIRED_VARS
                                  SDL2_LIBRARY SDL2_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_VERSION)

set(SDL2_LIBRARIES ${SDL2_LIBRARY} )
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR} )
mark_as_advanced(SDL2_LIBRARY SDL2_INCLUDE_DIR)
