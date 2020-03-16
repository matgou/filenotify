AM_CFLAGS = -I$(srcdir)/../include
AM_LDFLAGS = -no-undefined -lpthread
noinst_PROGRAMS = $(DBG)
lib_LTLIBRARIES = libplg_notify_exec.la libplg_notify_log.la
libplg_notify_exec_la_SOURCES = plg_notify_exec.c ../core/log.c ../core/config.c ../core/nlist.c
libplg_notify_log_la_SOURCES = plg_notify_log.c ../core/log.c ../core/config.c ../core/nlist.c
if USE_CURL
lib_LTLIBRARIES += libplg_http_post.la
libplg_http_post_la_SOURCES = plg_http_post.c ../core/log.c ../core/config.c ../core/nlist.c
endif