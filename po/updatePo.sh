#!/bin/sh

# Run to update translation files from the source code

DOMAIN=trackballs
COPYRIGHT_HOLDER="Mathias Broxvall"

# todo: make this a proper loop over types/files
base="xgettext --default-domain=$DOMAIN  --add-comments=TRANSLATORS: \
	  --keyword=_ --keyword=N_  --copyright-holder=\"$COPYRIGHT_HOLDER\" \
	  --output=trackballs.pot"
comm1="$base  -L C++ ../src/*.h ../src/*.cc"
comm2="$base --join-existing  -L Scheme ../share/levels/*.set ../share/levels/*.scm"
echo $comm1
eval $comm1
echo $comm2
eval $comm2

for POFILE in *.po
do
    msgmerge --update $POFILE $DOMAIN.pot
done
