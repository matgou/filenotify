SUBDIRS = plugins
AM_CFLAGS = -I$(srcdir)/include
bin_PROGRAMS = filenotify
nobase_include_HEADERS = include/*.h
filenotify_SOURCES = filenotify.c core/log.c core/config.c core/nlist.c core/tools.c
if USE_INOTIFY
filenotify_SOURCES += inotify_engine.c
else
filenotify_SOURCES += timer_engine.c
endif
filenotify_LDFLAGS=-I./include -ldl -lpthread 
