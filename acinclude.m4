dnl -*- Mode: M4 -*- 
dnl $Id: acinclude.m4,v 1.1 2000/01/06 18:36:33 pfkeb Exp $
dnl 
dnl Copyright (C) 2000 The Board of Trustees of 
dnl The Leland Stanford Junior University.  All Rights Reserved.  
dnl 
dnl To be called like
dnl
dnl   glast_CHECK_LIB( LIB_XXX, libxxx.la, DIR )
dnl
dnl Searchs for libxxx.la in one of two places.  
dnl Picks the first of  DIR or $glast_prefix/lib
dnl Sets the output variable LIBXXX to the first found
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
dnl -*- Mode: M4 -*- 
dnl $Id: acinclude.m4,v 1.1 2000/01/06 18:36:33 pfkeb Exp $
dnl 
dnl Copyright (C) 2000 The Board of Trustees of 
dnl The Leland Stanford Junior University.  All Rights Reserved.  
dnl 
dnl To be called like
dnl
dnl   glast_CHECK_LIB( LIB_XXX, libxxx.la, DIR )
dnl
dnl Searchs for libxxx.la in one of two places.  
dnl Picks the first of  DIR/src or $glast_prefix/lib
dnl Sets the output variable LIBXXX to the first found
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



