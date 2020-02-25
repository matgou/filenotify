# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 
CC=gcc
CFLAGS=-W -Wall -fPIC -I./../include -ggdb  
LFLAGS=-D_GNU_SOURCE=1 -ldl

SRC= $(wildcard *.c)
OBJ= $(addprefix ../../obj/core/,$(SRC:.c=.o))

all: $(OBJ)

../../obj/core/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rvf ../../obj/core/*.o

mrproper: clean
