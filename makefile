CC = gcc
CFLAGS = -g -Wall -Wextra -I.
LDFLAGS = -lssl -lcrypto -lgmp

all: sign tsa_server id2 id3 id_asm pass skey fiat_id diff speke rsa el fs base hmac

# Правила для sign
sign: obj/sign.o obj/rsa.o obj/el_gamal.o obj/fiat_shamir.o obj/essential_func.o obj/sha.o obj/tsa_client.o obj/tsa_server.o
	$(CC) -o sign obj/sign.o obj/rsa.o obj/el_gamal.o obj/fiat_shamir.o obj/essential_func.o obj/sha.o $(LDFLAGS)
	$(CC) -o tsa_server obj/tsa_server.o obj/rsa.o obj/essential_func.o $(LDFLAGS)

# Правила для id2
id2: obj/client_a.o obj/client_b.o obj/common.o
	$(CC) -o id2_a obj/client_a.o obj/common.o $(LDFLAGS)
	$(CC) -o id2_b obj/client_b.o obj/common.o $(LDFLAGS)

# Правила для id3
id3: obj/client_a3.o obj/client_b3.o obj/common.o
	$(CC) -o id3_a obj/client_a3.o obj/common.o $(LDFLAGS)
	$(CC) -o id3_b obj/client_b3.o obj/common.o $(LDFLAGS)

# Правила для id_asm
id_asm: obj/client_a_asm.o obj/client_b_asm.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o
	$(CC) -o id_asm_a obj/client_a_asm.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o id_asm_b obj/client_b_asm.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o $(LDFLAGS)

# Правила для pass
pass: obj/client_pass.o obj/server_pass.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o
	$(CC) -o client_pass obj/client_pass.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o obj/sha.o $(LDFLAGS)
	$(CC) -o server_pass obj/server_pass.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o obj/sha.o $(LDFLAGS)

# Правила для skey
skey: client_a_skey client_b_skey

client_a_skey: obj/client_a_skey.o sign_lib obj/rsa.o obj/essential_func.o obj/common.o obj/sha.o obj/el_gamal.o obj/fiat_shamir.o  
	$(CC) -o client_a_skey obj/client_a_skey.o obj/sign.o obj/rsa.o obj/essential_func.o obj/common.o obj/sha.o obj/fiat_shamir.o obj/el_gamal.o $(LDFLAGS)

client_b_skey: obj/client_b_skey.o obj/sign.o obj/rsa.o obj/essential_func.o obj/common.o obj/sha.o obj/el_gamal.o obj/fiat_shamir.o  
	$(CC) -o client_b_skey obj/client_b_skey.o obj/sign.o obj/rsa.o obj/essential_func.o obj/common.o obj/sha.o obj/fiat_shamir.o obj/el_gamal.o $(LDFLAGS)

# Правила для fiat_id
fiat_id: obj/client_a_fiat.o obj/client_b_fiat.o obj/server_fiat.o obj/essential_func.o obj/common.o
	$(CC) -o fiat_id_client_a obj/client_a_fiat.o obj/common.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o fiat_id_client_b obj/client_b_fiat.o obj/common.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o fiat_id_server obj/server_fiat.o obj/common.o obj/essential_func.o $(LDFLAGS)

blom: obj/client_a_blom.o  obj/server_blom.o obj/essential_func.o obj/common.o
	$(CC) -o blom_client_a obj/client_a_blom.o obj/common.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o blom_server obj/server_blom.o obj/common.o obj/essential_func.o $(LDFLAGS)



# Правила для diff
diff: obj/client_a_diff.o obj/client_b_diff.o obj/server_diff.o obj/essential_func.o obj/common.o
	$(CC) -o diff_client_a obj/client_a_diff.o obj/common.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o diff_client_b obj/client_b_diff.o obj/common.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o diff_server obj/server_diff.o obj/common.o obj/essential_func.o $(LDFLAGS)

# Правила для speke
speke: obj/client_a_speke.o obj/client_b_speke.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o
	$(CC) -o client_a_speke obj/client_a_speke.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o $(LDFLAGS)
	$(CC) -o client_b_speke obj/client_b_speke.o obj/common.o obj/rsa.o obj/sha.o obj/essential_func.o $(LDFLAGS)

# Правила для rsa
rsa: obj/rsa_main.o obj/rsa.o obj/essential_func.o
	$(CC) -o rsa obj/rsa_main.o obj/rsa.o obj/essential_func.o $(LDFLAGS)

# Правила для el
el: obj/el_gamal_standalone.o obj/essential_func.o
	$(CC) -o el obj/el_gamal_standalone.o obj/essential_func.o $(LDFLAGS)

# Правила для fs
fs: obj/fiat_shamir_standalone.o obj/essential_func.o obj/sha.o
	$(CC) -o fs obj/fiat_shamir_standalone.o obj/essential_func.o obj/sha.o $(LDFLAGS)

# Правила для base
base: obj/base.o obj/base_main.o
	$(CC) -o base_program obj/base_main.o obj/base.o

# Правила для hmac
hmac: obj/sha.o obj/hmac.o
	$(CC) -o hmac obj/hmac.o obj/sha.o 

# Правила компиляции объектных файлов
obj/sign.o: src/sign.c src/sign.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/sign.c -o obj/sign.o

sign_lib: src/sign.c src/sign.h
	mkdir -p obj
	$(CC) $(CFLAGS) -DLIB=1 -c src/sign.c -o obj/sign.o



obj/rsa.o: src/rsa.c src/rsa.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/rsa.c -o obj/rsa.o

obj/el_gamal.o: src/el_gamal.c src/el_gamal.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/el_gamal.c -o obj/el_gamal.o

obj/fiat_shamir.o: src/fiat_shamir.c src/fiat_shamir.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/fiat_shamir.c -o obj/fiat_shamir.o

obj/essential_func.o: src/essential_func.c src/essential_func.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/essential_func.c -o obj/essential_func.o

obj/sha.o: src/sha.c src/sha.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/sha.c -o obj/sha.o

obj/tsa_client.o: src/tsa_client.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/tsa_client.c -o obj/tsa_client.o

obj/tsa_server.o: src/tsa_server.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/tsa_server.c -o obj/tsa_server.o

obj/client_a.o: src/id2/client_a.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a.c -o obj/client_a.o

obj/client_b.o: src/id2/client_b.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b.c -o obj/client_b.o

obj/client_a3.o: src/id2/client_a3.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a3.c -o obj/client_a3.o

obj/client_b3.o: src/id2/client_b3.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b3.c -o obj/client_b3.o

obj/client_a_asm.o: src/id2/client_a_asm.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a_asm.c -o obj/client_a_asm.o

obj/client_b_asm.o: src/id2/client_b_asm.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b_asm.c -o obj/client_b_asm.o

obj/client_pass.o: src/id2/client_pass.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_pass.c -o obj/client_pass.o

obj/server_pass.o: src/id2/server_pass.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/server_pass.c -o obj/server_pass.o

obj/client_a_skey.o: src/id2/client_a_skey.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a_skey.c -o obj/client_a_skey.o

obj/client_b_skey.o: src/id2/client_b_skey.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b_skey.c -o obj/client_b_skey.o

obj/client_a_fiat.o: src/id2/client_a_fiat.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a_fiat.c -o obj/client_a_fiat.o

obj/client_b_fiat.o: src/id2/client_b_fiat.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b_fiat.c -o obj/client_b_fiat.o


obj/client_a_blom.o: src/id2/client_a_blom.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a_blom.c -o obj/client_a_blom.o

obj/client_b_blom.o: src/id2/client_b_blom.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b_blom.c -o obj/client_b_blom.o



obj/client_a_diff.o: src/id2/client_a_diff.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a_diff.c -o obj/client_a_diff.o

obj/client_b_diff.o: src/id2/client_b_diff.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b_diff.c -o obj/client_b_diff.o

obj/server_diff.o: src/id2/server_diff.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/server_diff.c -o obj/server_diff.o

obj/server_fiat.o: src/id2/server_fiat.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/server_fiat.c -o obj/server_fiat.o

obj/server_blom.o: src/id2/server_blom.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/server_blom.c -o obj/server_blom.o


obj/client_a_speke.o: src/id2/client_a_speke.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_a_speke.c -o obj/client_a_speke.o

obj/client_b_speke.o: src/id2/client_b_speke.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/client_b_speke.c -o obj/client_b_speke.o

obj/common.o: src/id2/common.c src/id2/common.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/id2/common.c -o obj/common.o

obj/hmac.o: src/hmac.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/hmac.c -o obj/hmac.o

obj/rsa_main.o: src/rsa_main.c
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/rsa_main.c -o obj/rsa_main.o

obj/el_gamal_standalone.o: src/el_gamal.c src/el_gamal.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/el_gamal.c -o obj/el_gamal_standalone.o

obj/fiat_shamir_standalone.o: src/fiat_shamir.c src/fiat_shamir.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/fiat_shamir.c -o obj/fiat_shamir_standalone.o

obj/base.o: src/base.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base.c -o obj/base.o

obj/base_main.o: src/base_main.c src/base.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/base_main.c -o obj/base_main.o

# Очистка
clean:
	rm -rf obj/* sign tsa_server id2_a id2_b id3_a id3_b id_asm_a id_asm_b \
	client_pass server_pass client_a_skey client_b_skey fiat_id_client_a \
	fiat_id_client_b fiat_id_server diff_client_a diff_client_b diff_server \
	client_a_speke client_b_speke rsa el fs base_program hmac

.PHONY: all clean
