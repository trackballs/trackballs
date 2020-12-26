# Locate libgc (BDW-gc)

# PkgConfig is only there to provide hints
find_package(PkgConfig)
pkg_check_modules(PC_GC QUIET bdw-gc)

set(GC_DEFINITIONS ${PC_GC_CFLAGS_OTHER})

# Former needed by Guile2.0, latter by 2.2
find_path(GC_INCLUDE_DIR NAMES gc/gc.h gc/gc_inline.h
          HINTS ${PC_GC_INCLUDEDIR} ${PC_GC_INCLUDE_DIRS})

find_library(GC_LIBRARY NAMES gc
             HINTS ${PC_GC_LIBDIR} ${PC_GC_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GC REQUIRED_VARS
                                  GC_LIBRARY GC_INCLUDE_DIR
                                  VERSION_VAR PC_GC_VERSION)

set(GC_LIBRARIES ${GC_LIBRARY})
set(GC_INCLUDE_DIRS ${GC_INCLUDE_DIR})

mark_as_advanced(GC_LIBRARY GC_INCLUDE_DIR)

