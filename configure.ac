#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ(2.60)

AC_INIT( [octopusd], [0.1], [upidea@gmail.com])

AC_CONFIG_SRCDIR([src/octopusd.cpp])
AC_CONFIG_HEADER([config.h])

CFLAGS="-O2 -Wimplicit -Wunused -Wcomment -Wchar-subscripts -Wuninitialized -Wshadow -Wcast-qual -Wcast-align -Wwrite-strings"
# Checks for programs.
AC_PATH_PROG(MV, mv, mv)
AC_PATH_PROG(RM, rm, rm)
AC_PATH_PROG(CP, cp, cp)
AC_PATH_PROG(SED, sed, sed)
AC_PATH_PROG(CMP, cmp, cmp)
AC_PATH_PROG(CHMOD, chmod, chmod)
AC_PATH_PROG(HOSTNAME, hostname, hostname)
# Check for a GNU tar named 'gtar', or 'gnutar' (MacOS X) and
# fall back to 'tar' otherwise and hope that it's a GNU tar as well
AC_CHECK_PROGS(TAR, gnutar gtar tar)

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

AC_LANG_C

AM_INIT_AUTOMAKE([no-dependencies])
# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([dirent.h sys/select.h sys/shm.h sys/sem.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_C_CONST
AC_C_INLINE
AC_C_LONG_DOUBLE
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(unsigned long long)

# Checks for library functions.
AC_CHECK_FUNCS([flock setgroups sigprocmask mkfifo sigaction waitpid poll])

# Evaluate options. Example:
AC_MSG_CHECKING([turn on debug])
AC_ARG_ENABLE(debug,
 [[  --enable-debug[=no/yes]      turn on debugging [default=no]]],, enable_debug=no)
if test x$enable_debug = xyes; then
 test "$cflags_set" = set
 CPPFLAGS="$CPPFLAGS -DDEBUG -g"
 AC_MSG_RESULT([ on])
else
 if test x$enable_debug = xno; then
# CPPFLAGS="$CPPFLAGS -DNDEBUG"
 AC_MSG_RESULT([ off])
 fi
fi

 CPPFLAGS="$CPPFLAGS -DNODB"

AC_ARG_ENABLE(mysqlconfig,
[  --enable-mysqlconfig   Set siteinfo config in mysql database],
[case "${enableval}" in
  yes) mysqlconfig=true ;;
  no)  mysqlconfig=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-mysqlconfig) ;;
esac],[mysqlconfig=false])
AM_CONDITIONAL(WITHMYSQLCONFIG, test x$mysqlconfig = xtrue)

AC_MSG_CHECKING([Weather use mysql config setttings ])
if test x$mysqlconfig = xfalse; then
        AC_DEFINE(NODB,,[Define this if not use mysql config. ])
        AC_MSG_RESULT([ NODB])
else
        AC_MSG_RESULT([ WITHMYSQL])
fi


# Include php extions
AC_CONFIG_SUBDIRS([octopusd_phpext])


AC_CONFIG_FILES([Makefile
		src/Makefile
		cfg/Makefile
		log/Makefile
		dat/Makefile
		sharelib/Makefile])
AC_OUTPUT
