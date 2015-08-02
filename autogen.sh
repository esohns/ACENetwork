#!/bin/sh
# author:      Erik Sohns <eriksohns@123mail.org>
# this script creates the configure script
# *TODO*: it is neither portable nor particularly stable !
# parameters:   - none
# return value: - 0 success, 1 failure

# sanity checks
command -v which >/dev/null 2>&1 || { echo "which is not installed, aborting" >&2; exit 1; }
command -v basename >/dev/null 2>&1 || { echo "basename is not installed, aborting" >&2; exit 1; }
command -v awk >/dev/null 2>&1 || { echo "awk is not installed, aborting" >&2; exit 1; }

# make
MAKE=$(which gnumake 2>/dev/null)
if test ! -x "$MAKE" ; then MAKE=$(which gmake 2>/dev/null) ; fi
if test ! -x "$MAKE" ; then MAKE=$(which make 2>/dev/null) ; fi
HAVE_GNU_MAKE=`$MAKE --version|grep -c "Free Software Foundation"`

if test "$HAVE_GNU_MAKE" != "1"; then 
echo only non-GNU make found: $MAKE
else
echo $($MAKE --version | head -1) found
fi

# autoreconf
AUTORECONF=$(which autoreconf 2>/dev/null)
if $AUTORECONF2.58 >/dev/null 2>&1
then AC_POSTFIX=2.58
elif $AUTORECONF --version >/dev/null 2>&1
then AC_POSTFIX=""
else
  echo 'you need autoreconf (2.58+ recommended) to generate the Makefile'
  exit 1
fi
echo $(autoreconf$AC_POSTFIX --version | head -1) found

#AUTOCONF=`which autoconf`
#if $AUTOCONF2.50 >/dev/null 2>&1
#then AC_POSTFIX=2.50
#elif $AUTOCONF >/dev/null 2>&1
#then AC_POSTFIX=""
#else 
#  echo 'you need autoconf (2.58+ recommended) to generate the Makefile'
#  exit 1
#fi
#echo `autoconf$AC_POSTFIX --version | head -1` found

## automake
#AUTOMAKE=$(which automake)
#if ! test -x $AUTOMAKE; then
#  echo 'you need automake (1.14+ recommended) to generate the Makefile'
#  exit 1
#fi
#echo $(automake --version | head -1) found

# gettextize
#GETTEXTIZE=$(which gettextize)
#if ! test -x $GETTEXTIZE; then
#  echo 'you need gettextize (0.19.3+ recommended) to generate the Makefile'
#  exit 1
#fi
#VERSION_STRING=$($GETTEXTIZE --version | head -1)
#COMMAND=$(echo $VERSION_STRING | cut -d ' ' -f 1)
#REST=$(echo $VERSION_STRING | awk -F' ' '{for (i=2;i<=NF;i++) printf("%s ", $i)}')
##read COMMAND REST <<<$(IFS=" "; echo $VERSION_STRING)
#echo $(basename $COMMAND) $REST found

# autopoint
AUTOPOINT=$(which autopoint)
if ! test -x $AUTOPOINT; then
  echo 'you need autopoint (0.18+ recommended) to generate the Makefile'
  exit 1
fi
VERSION_STRING=$($AUTOPOINT --version | head -1)
COMMAND=$(echo $VERSION_STRING | cut -d ' ' -f 1)
REST=$(echo $VERSION_STRING | awk -F' ' '{for (i=2;i<=NF;i++) printf("%s ", $i)}')
#read COMMAND REST <<<$(IFS=" "; echo $VERSION_STRING)
echo $(basename $COMMAND) $REST found

# libtool
LIBTOOL=$(which libtoolize)
if ! test -x $LIBTOOL; then
  echo 'you need libtool (2.4.2+ recommended) to generate the Makefile'
  exit 1
fi
echo $($(basename $LIBTOOL) --version | head -1) found

# pkg-config
PKGCONFIG=$(which pkg-config)
if ! test -x $PKGCONFIG; then
  echo 'you need pkg-config (0.28+ recommended) to generate the Makefile'
  exit 1
fi
echo $(basename $PKGCONFIG) $($(basename $PKGCONFIG) --version | head -1) found

autoreconf$AC_POSTFIX --force --install --symlink --warnings=all
[ $? -ne 0 ] && echo 'failed to autoreconf, aborting' && exit 1
#autoconf$AC_POSTFIX
#autoheader$AC_POSTFIX
#aclocal$AM_POSTFIX
#automake$AM_POSTFIX --add-missing --force-missing --all
#gettextize --force --symlink
#[ $? -ne 0 ] && echo 'failed to gettextize, aborting' && exit 1
#autopoint
#if test ! -e po/Makevars
#then cp po/Makevars.template po/Makevars
#fi
#if test ! -e po/LINGUAS
#then touch po/LINGUAS
#fi
#libtoolize --force --install
#[ $? -ne 0 ] && echo 'failed to libtoolize, aborting' && exit 1

