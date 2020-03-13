AC_INIT([filenotify], [1.0], [mathieu.goulin@gadz.org])
AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])
AC_PROG_CC

# Check configure args
AC_ARG_ENABLE([inotify],
[  --enable-inotify    Enable inotify],
[case "${enableval}" in
  yes) inotify=true ;;
  no)  inotify=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-inotify]) ;;
esac],[inotify=false])
AM_CONDITIONAL([INOTIFY], [test x$inotify = xtrue])

AC_CONFIG_HEADERS([filenotify_config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/plugins/Makefile])
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_PROG_LIBTOOL(libtool)
AC_OUTPUT
