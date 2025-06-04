#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <gmp.h>
#include "common.h"
#include "../essential_func.h"

void load_private_key(mpz_t n, mpz_t s) {
    FILE *priv = fopen("fiat_id.pri", "r");
    if (!priv) handle_error("Failed to open private key");
    
    char buf_n[MAX_BUF], buf_s[MAX_BUF];
    fgets(buf_n, sizeof(buf_n), priv);
    fgets(buf_s, sizeof(buf_s), priv);
    
    mpz_set_str(n, buf_n, 16);
    mpz_set_str(s, buf_s, 16);
    
    fclose(priv);
}

int main() {
    mpz_t n, s, x, r, e, y;
    gmp_randstate_t state;
    
    mpz_inits(n, s, x, r, e, y, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Загрузка секретного ключа
    load_private_key(n, s);
    gmp_printf("Prover: loaded n = %Zx\ns = %Zx\n", n, s);

    // Создание серверного сокета
    int server_fd = create_server_socket(PORT_PROVER);
    printf("Prover: waiting for verifier on port %d...\n", PORT_PROVER);

    int verifier_sock = accept(server_fd, NULL, NULL);
    if (verifier_sock < 0) handle_error("accept");

    // Протокол идентификации
    for (int i = 0; i < 20; i++) { // 20 итераций
        // Шаг 1: Генерация r и x = r² mod n
        do {
            mpz_urandomm(r, state, n);
        } while (mpz_cmp_ui(r, 0) == 0);
        
        mpz_powm_ui(x, r, 2, n);
        send_mpz(verifier_sock, x);

        // Шаг 3: Получение e
        recv_mpz(verifier_sock, e);

        // Шаг 4: Вычисление y = r * s^e mod n
        mpz_t s_pow_e;
        mpz_init(s_pow_e);
        fast_power_mod(s_pow_e, s, e, n);
        mpz_mul(y, r, s_pow_e);
        mpz_mod(y, y, n);
        mpz_clear(s_pow_e);

        send_mpz(verifier_sock, y);
    }
    uint8_t verified[1]; 
    
    // recv(verifier_sock, e);
    recv(verifier_sock, verified, 1, 0);
    if (verified[0])
      printf("Verification successful\n");
    else

      printf("Verification failed \n");

    mpz_clears(n, s, x, r, e, y, NULL);
    gmp_randclear(state);
    close(verifier_sock);
    close(server_fd);
    return 0;
}
