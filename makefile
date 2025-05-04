CC = gcc
CFLAGS = -I./src -lgmp

rsa: obj/rsa_main.o obj/rsa.o obj/essential_func.o
	$(CC) -o rsa obj/rsa_main.o obj/rsa.o obj/essential_func.o -lgmp

base: obj/base.o obj/base_main.o
	$(CC) -o base_program obj/base_main.o obj/base.o

sha: src/sha.c
	$(CC) -o sha src/sha.c  

stribog: src/stribog.c
	$(CC) -o stribog src/stribog.c  

obj/base.o: src/base.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base.c -o obj/base.o


obj/rsa_main.o: src/rsa_main.c 
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/rsa_main.c -o obj/rsa_main.o -lgmp

obj/rsa.o: src/rsa.c src/rsa.h src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/rsa.c -o obj/rsa.o -lgmp

obj/essential_func.o:  src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/essential_func.c -o obj/essential_func.o -lgmp

obj/base_main.o: src/base_main.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base_main.c -o obj/base_main.o

clean:
	rm -rf obj base_program rsa

run:
	./base_program -v
