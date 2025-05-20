CC = gcc
CFLAGS =  -g -I./src -lgmp -O0  

sign: obj/sign.o obj/rsa.o obj/essential_func.o obj/sha.o 
	$(CC) -o sign obj/sign.o obj/rsa.o obj/sha.o obj/essential_func.o -lgmp

rsa: obj/rsa_main.o obj/rsa.o obj/essential_func.o
	$(CC) -o rsa obj/rsa_main.o obj/rsa.o obj/essential_func.o -lgmp

base: obj/base.o obj/base_main.o
	$(CC) -o base_program obj/base_main.o obj/base.o

hmac: obj/sha.o obj/hmac.o
	$(CC) -o hmac obj/hmac.o obj/sha.o 

obj/sign.o: src/sign.c src/sign.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/sign.c  -o obj/sign.o

obj/hmac.o: src/hmac.c 
	$(CC) $(CFLAGS) -c src/hmac.c -o obj/hmac.o

obj/sha.o: src/sha.c src/sha.h 
	$(CC) $(CFLAGS) -DSHA_LIB=1 -c src/sha.c -o obj/sha.o

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
	rm obj/*

run:
	./base_program -v
