## Notes on the Free Translation Project

   Free software is going international!  The Free Translation Project
is a way to get maintainers of free software, translators, and users all
together, so that will gradually become able to speak many languages.
A few packages already provide translations for their messages.

   If you found this `ABOUT-NLS` file inside a distribution, you may
assume that the distributed package does use GNU `gettext` internally,
itself available at your nearest GNU archive site.  But you do _not_
need to install GNU `gettext` prior to configuring, installing or using
this package with messages translated.

   Installers will find here some useful hints.  These notes also
explain how users should proceed for getting the programs to use the
available translations.  They tell how people wanting to contribute and
work at translations should contact the appropriate team.

## Using This Package

   As a user, if your language has been installed for this package, you
only have to set the `LANG` environment variable to the appropriate
`LL_CC` combination.  Here `LL` is an ISO 639 two-letter language code,
and `CC` is an ISO 3166 two-letter country code.  For example, let's
suppose that you speak German and live in Germany.  At the shell
prompt, merely execute `setenv LANG de_DE` (in `csh`),
`export LANG; LANG=de_DE` (in `sh`) or `export LANG=de_DE` (in `bash`).
This can be done from your `.login` or `.profile` file, once and for
all.

   You might think that the country code specification is redundant.
But in fact, some languages have dialects in different countries.  For
example, `de_AT` is used for Austria, and `pt_BR` for Brazil.  The
country code serves to distinguish the dialects.

   The locale naming convention of `LL_CC`, with `LL` denoting the
language and `CC` denoting the country, is the one use on systems based
on GNU libc.  On other systems, some variations of this scheme are
used, such as `LL` or `LL_CC.ENCODING`.  You can get the list of
locales supported by your system for your country by running the command
`locale -a | grep `^LL``.

   Not all programs have translations for all languages.  By default, an
English message is shown in place of a nonexistent translation.  If you
understand other languages, you can set up a priority list of languages.
This is done through a different environment variable, called
`LANGUAGE`.  GNU `gettext` gives preference to `LANGUAGE` over `LANG`
for the purpose of message handling, but you still need to have `LANG`
set to the primary language; this is required by other parts of the
system libraries.  For example, some Swedish users who would rather
read translations in German than English for when Swedish is not
available, set `LANGUAGE` to `sv:de` while leaving `LANG` to `sv_SE`.

   In the `LANGUAGE` environment variable, but not in the `LANG`
environment variable, `LL_CC` combinations can be abbreviated as `LL`
to denote the language's main dialect.  For example, `de` is equivalent
to `de_DE` (German as spoken in Germany), and `pt` to `pt_PT`
(Portuguese as spoken in Portugal) in this context.

## Translating Teams

   For the Free Translation Project to be a success, we need interested
people who like their own language and write it well, and who are also
able to synergize with other translators speaking the same language.
Each translation team has its own mailing list.  The up-to-date list of
teams can be found at the Free Translation Project's homepage, 
[https://translationproject.org/html/welcome.html](https://translationproject.org/html/welcome.html), in the "National teams"
area.

   If you'd like to volunteer to _work_ at translating messages, you
should become a member of the translating team for your own language.
The subscribing address is _not_ the same as the list itself, it has
`-request` appended.  For example, speakers of Swedish can send a
message to `sv-request@li.org`, having this message body:

     subscribe

   Keep in mind that team members are expected to participate
_actively_ in translations, or at solving translational difficulties,
rather than merely lurking around.  If your team does not exist yet and
you want to start one, or if you are unsure about what to do or how to
get started, please write to `translation@iro.umontreal.ca` to reach the
coordinator for all translator teams.

   The English team is special.  It works at improving and uniformizing
the terminology in use.  Proven linguistic skill are praised more than
programming skill, here.

## Using `gettext` in new packages

   If you are writing a freely available program and want to
internationalize it you are welcome to use GNU `gettext` in your
package.  Of course you have to respect the GNU Library General Public
License which covers the use of the GNU `gettext` library.  This means
in particular that even non-free programs can use `libintl` as a shared
library, whereas only free software can use `libintl` as a static
library or use modified versions of `libintl`.

   Once the sources are changed appropriately and the setup can handle
to use of `gettext` the only thing missing are the translations.  The
Free Translation Project is also available for packages which are not
developed inside the GNU project.  Therefore the information given above
applies also for every other Free Software Project.  Contact
`translation@iro.umontreal.ca` to make the `.pot` files available to
the translation teams.
