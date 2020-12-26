find_package(PkgConfig)
pkg_check_modules(PC_SDL2_IMAGE QUIET SDL2_image)

set(SDL2_DEFINITIONS ${PC_SDL2_IMAGE_CFLAGS_OTHER})

find_path(SDL2_IMAGE_INCLUDE_DIR SDL2/SDL_image.h
          HINTS ${PC_SDL2_IMAGE_INCLUDEDIR} ${PC_SDL2_IMAGE_INCLUDE_DIRS}
          PATH_SUFFIXES SDL2 )

find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image libSDL2_image
             HINTS ${PC_SDL2_IMAGE_LIBDIR} ${PC_SDL2_IMAGE_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_image  REQUIRED_VARS
                                  SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR
                                  VERSION_VAR PC_SDL2_IMAGE_VERSION)

set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY} )
set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR} )
mark_as_advanced(SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)
