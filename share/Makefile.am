SUBDIRS = fonts levels sfx images icons music
pkgdata_DATA = highScores
man_MANS = trackballs.6
EXTRA_DIST = $(pkgdata_DATA) trackballs.6

install-pkgdataDATA:
	-install -D -m664 -g games highScores		$(DESTDIR)@with_highscores@
	-chgrp -R games			$(DESTDIR)$(pkgdatadir)/levels
	chmod -R g+r			$(DESTDIR)$(pkgdatadir)/levels
