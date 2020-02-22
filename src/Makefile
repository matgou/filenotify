# Mathieu GOULIN <mathieu.goulin@gadz.org>
# 
CC=gcc
CFLAGS=-W -Wall -I./include -ggdb  
LFLAGS=-D_GNU_SOURCE=1 

EXEC=../bin/filenotify
SRC= $(wildcard *.c)
OBJ= $(addprefix ../obj/,$(SRC:.c=.o))

all: $(EXEC)

$(EXEC): $(OBJ)
	gcc -o $@ $^ $(LFLAGS)

../obj/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rvf ../obj/*.o

mrproper: clean
	rm -vrf $(EXEC)
