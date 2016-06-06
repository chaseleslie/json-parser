PACKAGE=libjson
VERSION=0.0.0

AC_INIT([libjson], 0.0.0)
AC_CONFIG_SRCDIR([src/json_types.c])
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_LIBTOOL

LT_INIT

AC_CONFIG_MACRO_DIR([m4])

AC_CHECK_PROGS([DOT], [dot])
if test -z "$DOT"; then
	AC_MSG_WARN([Dot not found - continuing without doxygen graph support])
fi

AM_CONDITIONAL([HAVE_DOT], [test -n "$DOT"])
AM_COND_IF([HAVE_DOT], [AC_SUBST([DOXYGEN_USE_DOT], [YES])], [AC_SUBST([DOXYGEN_USE_DOT], [YES])])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"; then
	AC_MSG_WARN([Doxygen not found - continuing without doxygen support])
fi

AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])

AC_OUTPUT(Makefile src/Makefile tests/Makefile docs/Makefile)
