# Locate Guile. 
# Note: `guile-config` ultimately calls pkg-config anyway
# Nothing gets marked `advanced` since there aren't that many variables

find_program(GUILE_SNARF NAMES guile-snarf guile-snarf2.2 guile-snarf2.0)

# PkgConfig is only there to provide hints
find_package(PkgConfig)
pkg_check_modules(PC_GUILE QUIET guile)
if (NOT PC_GUILE_FOUND)
  pkg_check_modules(PC_GUILE QUIET guile-2.2)
  if (NOT PC_GUILE_FOUND)
    pkg_check_modules(PC_GUILE QUIET guile-2.0)
  endif(NOT PC_GUILE_FOUND)
endif(NOT PC_GUILE_FOUND)


set(GUILE_DEFINITIONS ${PC_GUILE_CFLAGS_OTHER})

find_path(GUILE_INCLUDE_DIR libguile.h
          HINTS ${PC_GUILE_INCLUDEDIR} ${PC_GUILE_INCLUDE_DIRS}
          PATH_SUFFIXES guile guile/2.2 guile/2.0)

find_library(GUILE_LIBRARY NAMES guile guile-2.2 guile-2.0
             HINTS ${PC_GUILE_LIBDIR} ${PC_GUILE_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libguile REQUIRED_VARS
                                  GUILE_LIBRARY GUILE_INCLUDE_DIR
                                  VERSION_VAR PC_GUILE_VERSION)

set(GUILE_LIBRARIES ${GUILE_LIBRARY} )
set(GUILE_INCLUDE_DIRS ${GUILE_INCLUDE_DIR} )

mark_as_advanced(GUILE_LIBRARY GUILE_INCLUDE_DIR GUILE_SNARF)
