dnl $Id$
dnl config.m4 for extension octopus

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(octopus, for octopus support,
dnl Make sure that the comment is aligned:
dnl [  --with-octopus             Include octopus support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(octopus, whether to enable octopus support,
[  --enable-octopus           Enable octopus support])
dnl Make sure that the comment is aligned:

if test "$PHP_OCTOPUS" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-octopus -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/octopus.h"  # you most likely want to change this
  dnl if test -r $PHP_OCTOPUS/$SEARCH_FOR; then # path given as parameter
  dnl   OCTOPUS_DIR=$PHP_OCTOPUS
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for octopus files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       OCTOPUS_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$OCTOPUS_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the octopus distribution])
  dnl fi

  dnl # --with-octopus -> add include path
  dnl PHP_ADD_INCLUDE($OCTOPUS_DIR/include)

  dnl # --with-octopus -> check for lib and symbol presence
  dnl LIBNAME=octopus # you may want to change this
  dnl LIBSYMBOL=octopus # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $OCTOPUS_DIR/lib, OCTOPUS_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_OCTOPUSLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong octopus lib version or lib not found])
  dnl ],[
  dnl   -L$OCTOPUS_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(OCTOPUS_SHARED_LIBADD)

  PHP_NEW_EXTENSION(octopus, octopus.c oct_cli.c, $ext_shared)
fi
