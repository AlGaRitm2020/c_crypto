CC = gcc
CFLAGS = -I./src

all: obj/base.o obj/base_main.o
	$(CC) -o base_program obj/base_main.o obj/base.o

obj/base.o: src/base.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base.c -o obj/base.o

obj/base_main.o: src/base_main.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base_main.c -o obj/base_main.o

clean:
	rm -rf obj base_program
