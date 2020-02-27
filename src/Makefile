# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 
CC=gcc
CFLAGS=-W -Wall -fPIC -I./include -ggdb -std=gnu99 $(OPTARGS) 
LFLAGS=-D_GNU_SOURCE=1 -ldl $(OPTARGS)

EXEC=../bin/filenotify
SRC= $(wildcard *.c)
OBJ= $(addprefix ../obj/,$(SRC:.c=.o))
CORE_SRC=$(wildcard core/*.c)
CORE_OBJ=$(addprefix ../obj/,$(CORE_SRC:.c=.o))

all: $(EXEC)

$(EXEC): $(OBJ)
	make -C core
	make -C plugins
	gcc -o $@ $(CORE_OBJ) $^ $(LFLAGS)

../obj/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	make -C plugins clean
	make -C core clean
	rm -rvf ../obj/*.o

mrproper: clean
	make -C plugins mrproper
	rm -vrf $(EXEC)
