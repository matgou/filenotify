AC_INIT([filenotify], [1.0], [mathieu.goulin@gadz.org])
AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror subdir-objects])
AC_PROG_CC
AC_CONFIG_HEADERS([filenotify_config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/plugins/Makefile])
AC_ARG_ENABLE([inotify],
[  --disable-inotify    Turn of inotify (on non linux os)],
[case "${enableval}" in
  yes) inotify=false ;;
  no)  inotify=true ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --disable-inotify]) ;;
esac],[inotify=true])
AM_CONDITIONAL([INOTIFY], [test x$inotify = xtrue])
AC_ENABLE_SHARED
AC_DISABLE_STATIC
AC_PROG_LIBTOOL(libtool)
AC_OUTPUT
