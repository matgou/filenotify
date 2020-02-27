# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 
CC=gcc
CFLAGS=-W -shared -Wall -fPIC -I../include -ggdb -std=gnu99 $(OPTARGS)
LFLAGS= -shared -D_GNU_SOURCE=1 

SRC= $(wildcard *.c)
OBJ= $(addprefix ../../obj/plugins/,$(SRC:.c=.o))
SO= $(addprefix ../../bin/,$(SRC:.c=.so))
CORE_SRC=$(wildcard ../core/*.c)
CORE_OBJ=$(addprefix ../../obj/core/,$(CORE_SRC:.c=.o))

all: $(SO)

../../bin/%.so: ../../obj/plugins/%.o
	make -C ../core/
	$(CC) -shared -Wl,-soname,$(basename $@) -o $@ $(CORE_OBJ) $< 

../../obj/plugins/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rvf ../../obj/plugins/*.o

mrproper: clean
	rm -rvf ../../bin/*.so
