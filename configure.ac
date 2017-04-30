# Process this file with autoconf to produce a configure script.
AC_INIT
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([trackballs], [1.1.2], [matbr@home.se])
AC_CONFIG_SRCDIR([src/mmad.cc])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

# Checks for some standard libraries.
AC_CHECK_LIB([e], [main])
AC_CHECK_LIB([inks], [main])
AC_CHECK_LIB([kenny], [life_signs],,[echo "oh my god! They killed kenny"; echo "You bastards"])

case $host in
  *mingw32) 
	WINRESOURCES='trackballsIcon.$(OBJEXT)' 
	AC_CHECK_LIB(iberty, main,,[AC_MSG_ERROR([Libiberty needed for getopt support.  See http://www.mingw.org])])
	AC_SEARCH_LIBS(gzclose, z zlib zlib.dll,,[AC_MSG_ERROR([No zlib library found.  See http://www.zlib.org])])
	AC_CHECK_LIB(opengl32,main,,[AC_MSG_ERROR([you need OpenGL])])
	AC_CHECK_LIB(glu32,main,,[AC_MSG_ERROR([missing libGLU, it should be a part of your OpenGL installation])])
;;
  *)       
	WINRESOURCES=''
	AC_CHECK_LIB(z,main,,[AC_MSG_ERROR([zlib should be included in most unix distributions, this is a strange error])])
	AC_CHECK_LIB(GL,main,,[AC_MSG_ERROR([you need OpenGL])])
	LIBS="${LIBS} -L/usr/X11R6/lib/"     ## Problem with some slack distributions not including all X11R6 stuff properly.
	AC_CHECK_LIB(GLU,main,,[AC_MSG_ERROR([missing libGLU, it should be a part of your OpenGL installation])])
;;
esac
AC_SUBST([WINRESOURCES])

AC_CHECK_LIB(m,main,,[AC_MSG_ERROR([download libHighSchool])])
dnl LIBS="-lGLU $LIBS"

dnl Check for SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,:,AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

dnl Check for SDL_ttf
AC_CHECK_LIB(SDL_ttf,main,,[AC_MSG_ERROR([need SDL_ttf. See http://www.libsdl.org])])

dnl Check for SDL_mixer
AC_CHECK_LIB(SDL_mixer,main,,[AC_MSG_ERROR([need SDL_mixer. See http://www.libsdl.org])])

dnl Check for SDL_image
AC_CHECK_LIB(SDL_image,main,,[AC_MSG_ERROR([need SDL_image. See http://www.libsdl.org])])

dnl Check for GUILE
GUILE_FLAGS
LIBS="${GUILE_LDFLAGS} ${LIBS}"
CFLAGS="${GUILE_CFLAGS} ${CFLAGS} -I./ -I../"
CXXFLAGS="${CXXFLAGS} ${GUILE_CFLAGS} -I./ -I../"


dnl For precompiled binaries only..
dnl Light semi-static linking
dnl LIBS =  /opt/guile/lib/libguile.a -lcrypt /usr/lib/libSDL_mixer.a /usr/lib/libartsc.a -lGLU -lGL -lm  -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread /usr/lib/libsmpeg.a /usr/lib/libvorbisfile.a /usr/lib/libvorbis.a /usr/lib/libogg.a /usr/lib/libSDL_ttf.a -lfreetype /usr/lib/libSDL_image.a /usr/lib/libpng.a -ltiff

dnl TODO: Statically link libpng also!

dnl Heavy semi-static linking (almost the real stuff)
dnl LIBS =  /opt/guile/lib/libguile.a -lcrypt /usr/lib/libSDL_mixer.a /usr/lib/libartsc.a -lGLU -lGL -lm  -L/usr/lib -Wl,-rpath,/usr/lib -lSDL -lpthread /usr/lib/libsmpeg.a /usr/lib/libvorbisfile.a /usr/lib/libvorbis.a /usr/lib/libogg.a /usr/lib/libSDL_ttf.a /usr/lib/libfreetype.a /usr/lib/libSDL_image.a /usr/lib/libpng.a /usr/lib/libtiff.a /usr/lib/libz.a /usr/lib/libjpeg.a 

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_CHECK_FUNCS([atexit sqrt strdup])



# Allow for extra options
AC_ARG_WITH([highscores],
			AC_HELP_STRING([--with-highscores=PATH],[Alternative pathname to highscores file. The "~" symbol by itself is substituted for the current user's $HOME/.trackballs/highscores file. "~/path/to/highscores" is substituted for $HOME/path/to/highscores.]),
		[CXXFLAGS="$CXXFLAGS -DALT_HIGHSCORES=\\\"$withval\\\""],[with_highscores='${pkgdatadir}/highScores'])
AC_ARG_WITH([resolution],
		AC_HELP_STRING([--with-resolution=RES],[Default resolution, 0=640x480, 1=800x600, 2=1024x768, 3=1280x1024]),
		[CXXFLAGS="$CXXFLAGS -DDEFAULT_RESOLUTION=$withval"],[CXXFLAGS="$CXXFLAGS -DDEFAULT_RESOLUTION=1"])
AC_ARG_WITH([my-static],
		AC_HELP_STRING([--with-my-static],[Used for semistatic linking]),
		[LIBS="/usr/lib/libguile.a /usr/lib/libguile-ltdl.a -lcrypt /usr/lib/libSDL_mixer.a /usr/lib/libmikmod.a /usr/lib/libsmpeg.a  /usr/lib/libqthreads.a -L/usr/kde/3.4/lib/ -lartsc -lGLU -lGL /usr/lib/libm.a -L/usr/lib -Wl,-rpath,/usr/lib -lSDL  /usr/lib/libvorbisfile.a /usr/lib/libvorbis.a /usr/lib/libogg.a /usr/lib/libSDL_ttf.a /usr/lib/libfreetype.a /usr/lib/libSDL_image.a /usr/lib/libpng.a /usr/lib/libtiff.a /usr/lib/libz.a /usr/lib/libjpeg.a"])

# Option for fixing a problem with some snarf versions on solaris
POST_SNARF='true'
AC_ARG_WITH([broken-snarf],
		AC_HELP_STRING([--with-broken-snarf],[Fixes a problem with snarf output incompatible with some compilers]),
		[POST_SNARF='sed -e "s/)()/)(...)/" < $@ > $@.tmp ; mv $@.tmp $@'])
AC_SUBST([POST_SNARF])
		
dnl		[LIBS="/usr/lib/libguile.a /usr/lib/libqthreads.a -lcrypt /usr/lib/libSDL_mixer.a -lartsc -lGLU -lGL /usr/lib/libm.a -L/usr/lib -Wl,-rpath,/usr/lib -lSDL  /usr/lib/libvorbisfile.a /usr/lib/libvorbis.a /usr/lib/libogg.a /usr/lib/libSDL_ttf.a /usr/lib/libfreetype.a /usr/lib/libSDL_image.a /usr/lib/libpng.a /usr/lib/libtiff.a /usr/lib/libz.a /usr/lib/libjpeg.a"])


ALL_LINGUAS="sv de it fr"
AM_GNU_GETTEXT([external])


AC_CONFIG_FILES([Makefile po/Makefile.in m4/Makefile 
                 share/Makefile
                 share/fonts/Makefile
                 share/images/Makefile
                 share/levels/Makefile
                 share/music/Makefile
                 share/sfx/Makefile
                 share/icons/Makefile
                 src/Makefile
		 intl/Makefile])



AC_SUBST(with_highscores)
AC_CONFIG_COMMANDS([default],[[ echo timestamp > stamp-h ]],[[]])
AC_OUTPUT


