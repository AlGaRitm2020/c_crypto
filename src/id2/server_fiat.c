#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
#include "common.h"
#include "../essential_func.h"

void generate_key_pair() {
    mpz_t n, s, v;
    gmp_randstate_t state;
    
    mpz_inits(n, s, v, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Генерация модуля RSA (2048 бит)
    mpz_t p, q;
    mpz_inits(p, q, NULL);
    generate_prime(p, 1024, state);
    generate_prime(q, 1024, state);
    mpz_mul(n, p, q);
    mpz_clears(p, q, NULL);

    // Генерация секретного ключа s
    do {
        mpz_urandomm(s, state, n);
    } while (mpz_cmp_ui(s, 0) == 0);

    // Вычисление публичного ключа v = s^2 mod n
    mpz_powm_ui(v, s, 2, n);

    // Сохранение ключей в файлы
    FILE *priv = fopen("fiat_id.pri", "w");
    FILE *pub = fopen("fiat_id.pub", "w");
    
    if (!priv || !pub) {
        handle_error("Failed to open key files");
    }

    mpz_out_str(priv, 16, n);
    fputc('\n', priv);
    mpz_out_str(priv, 16, s);
    
    mpz_out_str(pub, 16, n);
    fputc('\n', pub);
    mpz_out_str(pub, 16, v);

    fclose(priv);
    fclose(pub);

    printf("Generated keys:\n");
    gmp_printf("n = %Zx\ns = %Zx\nv = %Zx\n", n, s, v);

    mpz_clears(n, s, v, NULL);
    gmp_randclear(state);
}

int main() {
    generate_key_pair();
    //
    // int server_fd = create_server_socket(PORT_TRUSTED);
    // printf("Trusted Center: waiting for connections on port %d...\n", PORT_TRUSTED);
    //
    // while (1) {
    //     int client_socket = accept(server_fd, NULL, NULL);
    //     if (client_socket < 0) {
    //         handle_error("accept");
    //     }
    //
    //     FILE *pub = fopen("fiat_id.pub", "r");
    //     if (!pub) handle_error("Failed to open pub key");
    //
    //     char buf[MAX_BUF];
    //     fgets(buf, sizeof(buf), pub); // Чтение n
    //     send(client_socket, buf, strlen(buf), 0);
    //
    //     fgets(buf, sizeof(buf), pub); // Чтение v
    //     send(client_socket, buf, strlen(buf), 0);
    //
    //     fclose(pub);
    //     close(client_socket);
    // }
    //
    // close(server_fd);
    return 0;
}
