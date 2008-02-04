#
# Most of this configure script was ripped from gaim or the
# gaim-encryption configure script, thanks guys ;)
#

AC_INIT([pidgin-blinklight], [0.10], [mail@joachim-breitner.de])
AC_CANONICAL_SYSTEM
AC_GNU_SOURCE
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([1.8])

AC_PREREQ([2.50])

AC_PATH_PROG(sedpath, sed)
AC_DISABLE_STATIC
#
# Setup libtool
#
AM_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"


#
# Good o' gettext
#
#ALL_LINGUAS="de en_AU en_GB es fr he it ja nl no pt pt_BR sk sv zh_CN zh_TW"
ALL_LINGUAS="de sv"
AM_GNU_GETTEXT(external)
#AM_GNU_GETTEXT(external)
AM_GNU_GETTEXT_VERSION(0.16.1)

#
# Look for the C compiler
#
CFLAGS_save="$CFLAGS"
AC_PROG_CC_STDC
CFLAGS="$CFLAGS_save"

AC_ARG_ENABLE(debug, [  --enable-debug          compile with debugging support],,enable_debug=no)

if test "x$enable_debug" = "xyes" ; then
	AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])
fi

AC_SUBST(DEBUG_CFLAGS)

if test "x$GCC" = "xyes"; then
	CFLAGS="$CFLAGS -Wall -g3"
fi
AC_SUBST(CFLAGS)

#
# Check for gaim
#

PKG_CHECK_MODULES(PURPLE, purple,
[
	AC_DEFINE(HAVE_PURPLE, 1, [Define if we've found purple.])
])

AC_SUBST(PURPLE)
PURPLE_LIBDIR=`pkg-config --variable=libdir purple`
PURPLE_DATADIR=`pkg-config --variable=datadir purple`
AC_SUBST(PURPLE_LIBDIR)
AC_SUBST(PURPLE_DATADIR)


#
# Check if plugins are enabled
#
want_plugins=yes
AM_CONDITIONAL(PLUGINS, test "$want_plugins" = "yes")

#
# Finish up
#
AC_OUTPUT([Makefile
	   VERSION
	   po/Makefile.in
	   src/Makefile
	  ])

echo;
echo Configuration complete
echo;
echo Debugging enabled..............: $enable_debug
echo;
echo Type make to compile
echo Thank you for using pidgin-blinklight
echo;
