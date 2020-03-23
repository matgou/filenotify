AC_INIT([filenotify], [1.0], [mathieu.goulin@gadz.org])
AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])
AC_PROG_CC
AC_CONFIG_HEADERS([filenotify_config.h])

AM_CONDITIONAL([DEBUG_MODE], [test x$debug = xtrue])

# Check configure args
AC_CHECK_HEADER("sys/inotify.h", have_inotify=true, have_inotify=false) 
AC_ARG_ENABLE([inotify],
[  --enable-inotify    Enable inotify],
[case "${enableval}" in
  yes) inotify=true ;;
  no)  inotify=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-inotify]) ;;
esac],[inotify=$have_inotify])
AS_IF([test x$inotify = xtrue], [AC_DEFINE([USE_INOTIFY], [1], [Compile with inotify usage to detect fs change])])
AM_CONDITIONAL([USE_INOTIFY], [test x$inotify = xtrue])

# Check configure args
AC_CHECK_HEADER("curl/curl.h", have_curl=true, have_curl=false) 
AC_ARG_ENABLE([curl],
[  --enable-curl    Enable curl],
[case "${enableval}" in
  yes) curl=true ;;
  no)  curl=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-curl]) ;;
esac],[curl=$have_curl])
AS_IF([test x$curl = xtrue], [AC_DEFINE([USE_CURL], [1], [Compile with curl usage to detect fs change])])
AM_CONDITIONAL([USE_CURL], [test x$curl = xtrue])

# Check configure args
AC_CHECK_HEADER("clamav.h", have_clamav=true, have_clamav=false)
AC_ARG_ENABLE([clamav],
[  --enable-clamav    Enable clamav],
[case "${enableval}" in
  yes) clamav=true ;;
  no)  clamav=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-clamav]) ;;
esac],[clamav=$have_clamav])
AS_IF([test x$clamav = xtrue], [AC_DEFINE([USE_CLAMAV], [1], [Compile with clamav usage to detect fs change])])
AM_CONDITIONAL([USE_CLAMAV], [test x$clamav = xtrue])

# AC_CANONICAL_HOST is needed to access the 'host_os' variable    
AC_CANONICAL_HOST

AH_TEMPLATE([HOST_OS], [Target OS name])
AC_DEFINE_UNQUOTED(HOST_OS, $host_os )

AC_CONFIG_FILES([Makefile src/Makefile src/plugins/Makefile tests/Makefile])
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_PROG_LIBTOOL(libtool)
AC_OUTPUT
