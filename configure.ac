#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.53)
AC_INIT([Xoo], 0.8, [ross@openedhand.com])
AC_CONFIG_SRCDIR([src/main.c])

AM_INIT_AUTOMAKE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AM_CONFIG_HEADER(config.h)

AC_ARG_WITH(expat-includes,    
  [  --with-expat-includes=DIR     Use Expat includes in DIR], 
	   expat_includes=$withval, expat_includes=yes)

AC_ARG_WITH(expat-lib,         
  [  --with-expat-lib=DIR          Use Expat library in DIR], 
	   expat_lib=$withval, expat_lib=yes)

# Checks for programs.
AC_GNU_SOURCE
AC_PROG_CC
AC_PATH_XTRA
GLIB_GSETTINGS
PKG_CHECK_MODULES(APP, [gtk+-3.0])

dnl ------ XTest ------------------------------------------------------------

XTEST_LIBS=
AC_CHECK_HEADER(X11/extensions/XTest.h, [
    AC_CHECK_LIB(Xtst, XTestQueryExtension, [
      XTEST_LIBS="-lXtst"],
      AC_MSG_ERROR([*** XTEST extension libs not found and required ***])
      , $X_LIBS)
  ])
AC_SUBST(XTEST_LIBS)


dnl ------ Expat ------------------------------------------------------------


case "$expat_includes" in
 yes|no)
	EXPAT_CFLAGS=""
	;;
 *)
	EXPAT_CFLAGS="-I$expat_includes"
	;;
esac
	
case "$expat_lib" in
 yes)
	EXPAT_LIBS="-lexpat"
	;;
 no)
	;;
 *)
	EXPAT_LIBS="-L$expat_lib -lexpat"
	;;
esac

expatsaved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $EXPAT_CFLAGS"
expatsaved_LIBS="$LIBS"
LIBS="$LIBS $EXPAT_LIBS"

AC_CHECK_HEADER(expat.h)
case "$ac_cv_header_expat_h" in
  no)
	 AC_CHECK_HEADER(xmlparse.h)

	 case "$ac_cv_header_xmlparse_h" in
	   no)
		have_expat_header=no;
		;;
	   yes)
		HAVE_XMLPARSE_H=1
		AC_SUBST(HAVE_XMLPARSE_H)
		AC_DEFINE_UNQUOTED(HAVE_XMLPARSE_H,$HAVE_XMLPARSE_H,
				       [Use xmlparse.h instead of expat.h])
		have_expat_header=yes
	        ;;
	 esac
	 ;;
  yes)
	 have_expat_header=yes
	;;
esac
	
case "$have_expat_header" in
  no)
	expat=no
	;;
  yes)
	AC_CHECK_FUNCS(XML_ParserCreate)

	case "$ac_cv_func_XML_ParserCreate" in
       no)
	  	expat=no
		;;
	  yes)
		HAVE_EXPAT=1
		AC_SUBST(HAVE_EXPAT)
		AC_DEFINE_UNQUOTED(HAVE_EXPAT,$HAVE_EXPAT,
		[Found a useable expat library])
		;;
	esac
	;;
esac
	
CPPFLAGS="$saved_CPPFLAGS"
LIBS="$saved_LIBS"

if test x$expat = xno; then
      AC_MSG_ERROR([cannot find expat library])
fi

AC_DEFINE(USE_EXPAT, [1], [Use Expat XML Parser])


if test "x$GCC" = "xyes"; then
        GCC_WARNINGS="-Wall -fno-strict-aliasing"
        APP_CFLAGS="$GCC_WARNINGS $APP_CFLAGS"
fi

AC_SUBST(APP_CFLAGS)
AC_SUBST(APP_LIBS)

AC_SUBST(EXPAT_LIBS)
AC_SUBST(EXPAT_CFLAGS)

# Checks for header files.

AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T


# Checks for library functions.
 
AC_OUTPUT([Makefile src/Makefile data/Makefile])
