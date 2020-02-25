# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 
CC=gcc
CFLAGS=-W -Wall -fPIC -I../include -ggdb  
LFLAGS=-D_GNU_SOURCE=1 

SRC= $(wildcard *.c)
OBJ= $(addprefix ../../obj/plugins/,$(SRC:.c=.o))
SO= $(addprefix ../../bin/,$(SRC:.c=.so))

all: $(SO)

../../bin/%.so: ../../obj/plugins/%.o
	$(CC) -shared -Wl,-soname,$(basename $@) -o $@ $< 

../../obj/plugins/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rvf ../../obj/plugins/*.o

mrproper: clean
	rm -rvf ../../bin/*.so
