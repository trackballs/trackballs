pkgdatadir = $(datadir)/@PACKAGE@/images
pkgdata_DATA = $(wildcard *.png) trackballs.desktop installIcons
EXTRA_DIST = $(pkgdata_DATA) 

install-pkgdataDATA:
	./installIcons $(bindir) ${DESTDIR}
