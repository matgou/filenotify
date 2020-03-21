SUBDIRS = plugins
AM_CFLAGS = -I$(srcdir)/include
AM_LDFLAGS =
if DEBUG_MODE
AM_CFLAGS += -fno-omit-frame-pointer -fsanitize=address -g3
AM_LDFLAGS += -fsanitize=address -g3
endif
bin_PROGRAMS = filenotify
nobase_include_HEADERS = include/*.h
filenotify_SOURCES = filenotify.c core/log.c core/config.c core/nlist.c core/tools.c
if USE_INOTIFY
filenotify_SOURCES += inotify_engine.c
else
filenotify_SOURCES += timer_engine.c
endif
filenotify_LDFLAGS=-I./include -ldl -lpthread 
