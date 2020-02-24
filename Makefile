# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 

CC=gcc
CFLAGS=-Wall -O
LDFLAGS=
EXEC=filenotify

all:
	make -C src

clean:
	rm -fv obj/*

mrproper: clean
	rm -fv bin/$(EXEC)

check:
	make -C test
