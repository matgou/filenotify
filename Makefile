# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 

CC=gcc
CFLAGS=-Wall -O
LDFLAGS=
EXEC=filenotify

all: bin/$(EXEC)

bin/$(EXEC):
	make -C src

docker: package filenotify.tar.gz
	docker build -t filenotify:latest .	

filenotify.tar.gz: package

package: bin/$(EXEC)
	mkdir -p package/etc/filenotify package/usr/bin package/usr/lib/filenotify
	cp filenotify.config package/etc/filenotify/
	cp bin/$(EXEC) package/usr/bin/ 
	cp bin/*.so package/usr/lib/filenotify/
	tar -czvf filenotify.tar.gz -C package usr etc

clean:
	make -C src clean

mrproper: clean
	rm -fv filenotify.tar.gz
	rm -rvf package/usr package/etc
	make -C src mrproper

check:
	make -C test
