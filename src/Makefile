# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 
CC=gcc
CFLAGS=-W -Wall -I./include -ggdb  
LFLAGS=-D_GNU_SOURCE=1 -ldl

EXEC=../bin/filenotify
SRC= $(wildcard *.c)
OBJ= $(addprefix ../obj/,$(SRC:.c=.o))

all: $(EXEC)
	make -C plugins

$(EXEC): $(OBJ)
	gcc -o $@ $^ $(LFLAGS)

../obj/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	make -C plugins clean
	rm -rvf ../obj/*.o

mrproper: clean
	make -C plugins mrproper
	rm -vrf $(EXEC)
