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

install:
	systemctl stop filenotify
	[ -f /etc/filenotify/filenotify.config ] ||  cp -v filenotify.config /etc/filenotify/
	cp -v bin/$(EXEC) /usr/bin/ 
	mkdir -p /var/lib/filenotify/
	cp bin/*.so /var/lib/filenotify/
	systemctl start filenotify

package: bin/$(EXEC)
	mkdir -p package/etc/filenotify package/usr/bin package/var/lib/filenotify
	cp filenotify.config package/etc/filenotify/
	cp bin/$(EXEC) package/usr/bin/ 
	cp bin/*.so package/var/lib/filenotify/
	tar -czvf filenotify.tar.gz -C package usr etc var

clean:
	make -C src clean

mrproper: clean
	rm -fv filenotify.tar.gz
	rm -rvf docs/html docs/latex
	rm -rvf package/usr package/etc
	make -C src mrproper

check:
	make -C test

doc:
	doxygen filenotify.doxygen
