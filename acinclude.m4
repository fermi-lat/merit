dnl -*- Mode: M4 -*- 
dnl $Id: acinclude.m4,v 1.1 2000/01/06 19:05:08 pfkeb Exp $
dnl 
dnl Copyright (C) 2000 The Board of Trustees of 
dnl The Leland Stanford Junior University.  All Rights Reserved.  
dnl 
dnl Author: Paul_Kunz@slac.stanford.edu
dnl
dnl
dnl glast_CHECK_ROOT
dnl
dnl Configure the root of the GLAST installation
dnl

AC_DEFUN(glast_CHECK_ROOT,
[
AC_ARG_WITH(glast, --with-glast=DIR (path to GLAST libraries),
		   gl_with_glast=$withval, gl_with_glast=no )
	if test x"$gl_with_glast" != xno ; then
	    glast_prefix=$gl_with_glast
	fi
AC_SUBST(glast_prefix)
AC_MSG_RESULT( GLAST installation at $glast_prefix assumed)

dnl Check that it is really there
AC_CHECK_PROG( MERIT, merit, merit, no, $glast_prefix/bin)
if test "$MERIT" = no; then
   AC_MSG_WARN([GLAST installation not found in $glast_prefix
])
fi

])

dnl glast_CHECK_LIB

dnl To be called like
dnl
dnl   glast_CHECK_LIB( LIB_XXX, libxxx.la, DIR )
dnl
dnl Searchs for libxxx.la in one of two places.  
dnl Picks the first of DIR or $glast_prefix/lib
dnl Sets the output variable LIB_XXX to the first found
dnl
dnl Author: Paul_Kunz@slac.stanford.edu
dnl         (my first M4 macro)
dnl

AC_DEFUN(glast_CHECK_LIB,
[
  AC_CHECK_PROG(glast_result, autogen, yes, no, $srcdir/$3)
  if test x"$glast_result" = xyes ; then
    $1="\$(top_builddir)/$3/$2"
  else
    $1=$glast_prefix/lib/$2
  fi
  AC_SUBST($1)
])
dnl  glast_CHECK_LIB_SRC 
dnl 
dnl To be called like
dnl
dnl   glast_CHECK_LIB_SRC( LIB_XXX, libxxx.la, DIR )
dnl
dnl Searchs for libxxx.la in one of two places.  
dnl Picks the first of  DIR/src or $glast_prefix/lib
dnl Sets the output variable LIB_XXX to the first found
dnl
dnl Author: Paul_Kunz@slac.stanford.edu
dnl

AC_DEFUN(glast_CHECK_LIB_SRC,
[
  AC_CHECK_PROG(glast_result, autogen, yes, no, $srcdir/$3)
  if test x"$glast_result" = xyes ; then
    $1="\$(top_builddir)/$3/src/$2"
  else
    $1=$glast_prefix/lib/$2
  fi
  AC_SUBST($1)
])



