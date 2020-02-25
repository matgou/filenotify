# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 

CC=gcc
CFLAGS=-Wall -O
LDFLAGS=
EXEC=filenotify

all:
	make -C src

package:
	tar -czvf filenotify.tar.gz bin/* filenotify.config

clean:
	make -C src clean

mrproper: clean
	rm -fv filenotify.tar.gz
	make -C src mrproper

check:
	make -C test
