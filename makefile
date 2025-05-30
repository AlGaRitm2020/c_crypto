CC = gcc -g
CFLAGS =  -I./src -lgmp -O0  

sign: obj/sign.o obj/rsa.o obj/el_gamal.o obj/fiat_shamir.o obj/essential_func.o obj/sha.o src/tsa_client.c obj/tsa_server.o
	$(CC) -o sign obj/sign.o obj/rsa.o obj/el_gamal.o obj/fiat_shamir.o obj/sha.o obj/essential_func.o -lgmp
	$(CC) -o tsa_server obj/tsa_server.o obj/rsa.o  obj/essential_func.o -lgmp

id2: obj/client_a.o obj/client_b.o obj/common.o
	$(CC) -o id2_a obj/client_a.o obj/common.o -lssl -lcrypto 
	$(CC) -o id2_b obj/client_b.o obj/common.o -lssl -lcrypto

tsa_server: obj/tsa_server.o obj/rsa.o obj/essential_func.o 
	$(CC) -o tsa_server obj/tsa_server.o obj/rsa.o  obj/essential_func.o -lgmp

rsa: obj/rsa_main.o obj/rsa.o obj/essential_func.o
	$(CC) -o rsa obj/rsa_main.o obj/rsa.o obj/essential_func.o -lgmp


el: obj/el_gamal_standalone.o obj/essential_func.o
	$(CC) -o el obj/el_gamal_standalone.o  obj/essential_func.o -lgmp

fs: obj/fiat_shamir_standalone.o obj/essential_func.o obj/sha.o
	$(CC) -o fs obj/fiat_shamir_standalone.o  obj/essential_func.o obj/sha.o -lgmp

base: obj/base.o obj/base_main.o
	$(CC) -o base_program obj/base_main.o obj/base.o

hmac: obj/sha.o obj/hmac.o
	$(CC) -o hmac obj/hmac.o obj/sha.o 

obj/sign.o: src/sign.c src/sign.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/sign.c  -o obj/sign.o

obj/client_a.o: src/id2/client_a.c 	
	$(CC) -c src/id2/client_a.c -o obj/client_a.o 

obj/client_b.o: src/id2/client_b.c 	
	$(CC) -c src/id2/client_b.c -o obj/client_b.o 

obj/common.o: src/id2/common.c src/id2/common.h
	$(CC) -c src/id2/common.c -o obj/common.o 


obj/hmac.o: src/hmac.c 
	$(CC) $(CFLAGS) -c src/hmac.c -o obj/hmac.o

obj/tsa_server.o: src/tsa_server.c 
	$(CC) $(CFLAGS) -c src/tsa_server.c -o obj/tsa_server.o

obj/tsa_client.o: src/tsa_client.c 
	$(CC) $(CFLAGS) -c src/tsa_client.c -o obj/tsa_client.o




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

obj/el_gamal.o: src/el_gamal.c src/el_gamal.h src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -DLIB=1 -c src/el_gamal.c -o obj/el_gamal.o -lgmp

obj/fiat_shamir.o: src/fiat_shamir.c src/fiat_shamir.h src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -DLIB=1 -c src/fiat_shamir.c -o obj/fiat_shamir.o -lgmp




obj/el_gamal_standalone.o: src/el_gamal.c src/el_gamal.h src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/el_gamal.c -o obj/el_gamal_standalone.o -lgmp

obj/fiat_shamir_standalone.o: src/fiat_shamir.c src/fiat_shamir.h src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/fiat_shamir.c -o obj/fiat_shamir_standalone.o -lgmp






obj/essential_func.o:  src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/essential_func.c -o obj/essential_func.o -lgmp

obj/base_main.o: src/base_main.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base_main.c -o obj/base_main.o

clean:
	rm -rf obj/*

.PHONY: clean
