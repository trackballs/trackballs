
bin_PROGRAMS = trackballs
INCLUDES=-DSHARE_DIR=\"${pkgdatadir}\" -DPACKAGE=\"${PACKAGE}\" -DVERSION=\"${VERSION}\"
trackballs_LDFLAGS=-g
trackballs_CXXFLAGS=-g
trackballs_SOURCES = \
	guile.cc.x \
	mmad.cc gameMode.cc mainMode.cc glHelp.cc general.cc map.cc \
	player.cc editMode.cc ball.cc game.cc animated.cc debris.cc black.cc guile.cc \
	gameHook.cc cyclicPlatform.cc sound.cc sign.cc scoreSign.cc flag.cc goal.cc \
	menuMode.cc enterHighScoreMode.cc highScore.cc spike.cc trigger.cc modPill.cc \
	baby.cc trampoline.cc box.cc forcefield.cc switch.cc hofMode.cc settingsMode.cc \
	settings.cc setupMode.cc gamer.cc smartTrigger.cc pipe.cc pipeConnector.cc diamond.cc \
	helpMode.cc animator.cc cactus.cc splash.cc teleport.cc bird.cc \
	fountain.cc sideSpike.cc colorModifier.cc heightModifier.cc \
	sparkle2d.cc image.cc font.cc calibrateJoystickMode.cc menusystem.cc weather.cc \
	myWindow.cc editWindows.cc \
	\
	ball.h game.h general.h mainMode.h player.h editMode.h gameMode.h glHelp.h \
	map.h animated.h debris.h black.h sound.h sign.h scoreSign.h flag.h goal.h guile.h guile.cc.x \
	cyclicPlatform.h game.h gameHook.h cactus.h \
	menuMode.h enterHighScoreMode.h highScore.h spike.h trigger.h modPill.h \
	baby.h trampoline.h box.h forcefield.h switch.h hofMode.h settingsMode.h settings.h \
	setupMode.h gamer.h smartTrigger.h pipe.h pipeConnector.h diamond.h \
	helpMode.h animator.h splash.h teleport.h bird.h fountain.h \
	sideSpike.h colorModifier.h heightModifier.h sparkle2d.h \
	image.h font.h calibrateJoystickMode.h menusystem.h weather.h \
	myWindow.h editWindows.h editMode_codes.h


EXTRA_trackballs_SOURCES = trackballsIcon.rc

trackballs_LDADD = @WINRESOURCES@
trackballs_DEPENDENCIES = @WINRESOURCES@

trackballsIcon.o : trackballsIcon.rc trackballsIcon.ico
	windres $< -O coff -o $@

trackballsIcon.ico : 
	png2ico $@ $(top_srcdir)/share/icons/*.png

snarfcppopts = $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
%.cc.x: %.cc
	( CPP=cpp ; guile-snarf -o $@ $< $(snarfcppopts) )
	@POST_SNARF@

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

