# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 

CC=gcc
CFLAGS=-Wall -O
LDFLAGS=
EXEC=filenotify

all:
	make -C src

clean:
	make -C src clean

mrproper: clean
	make -C src mrproper

check:
	make -C test
