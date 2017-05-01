#!/bin/sh

# Run to update translation files from the source code

DOMAIN=trackballs
COPYRIGHT_HOLDER="Mathias Broxvall"

xgettext --default-domain=$DOMAIN \
	  --add-comments=TRANSLATORS: \
	  --keyword=_ --keyword=N_ \
	  --copyright-holder="$COPYRIGHT_HOLDER" \
	  --output=trackballs.pot \
	  ../src/*.h ../src/*.cc

for POFILE in *.po
do
    msgmerge --update $POFILE $DOMAIN.pot
done
