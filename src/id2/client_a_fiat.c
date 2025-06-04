#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
#include "common.h"
#include "../essential_func.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <trusted_center_ip>\n", argv[0]);
        return 1;
    }

    mpz_t n, s, v, x, r, e, y;
    gmp_randstate_t state;
    
    mpz_inits(n, s, v, x, r, e, y, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    
    // Получение модуля n от доверенного центра
    int trusted_sock = connect_to_server(argv[1], PORT_TRUSTED);
    recv_mpz(trusted_sock, n);
    close(trusted_sock);
    
    printf("Prover: received modulus n from trusted center\n");
    
    // Генерация секретного ключа s
    do {
        mpz_urandomm(s, state, n);
    } while (mpz_cmp_ui(s, 0) == 0);
    
    // Вычисление публичного ключа v = s^2 mod n
    mpz_mul(v, s, s);
    mpz_mod(v, v, n);
    
    printf("Prover: generated secret s and public v\n");
    
    // Создание серверного сокета для verifier
    int server_fd = create_server_socket(PORT_PROVER);
    printf("Prover: waiting for verifier on port %d...\n", PORT_PROVER);
    
    int verifier_sock;
    struct sockaddr_in verifier_addr;
    socklen_t addr_len = sizeof(verifier_addr);
    
    if ((verifier_sock = accept(server_fd, (struct sockaddr *)&verifier_addr, &addr_len)) < 0) {
        handle_error("accept");
    }
    
    printf("Prover: verifier connected\n");
    
    // Отправка публичного ключа v verifier'у
    send_mpz(verifier_sock, v);
    printf("Prover: sent public key v to verifier\n");
    
    // Итеративная часть протокола
    for (int i = 0; i < 10; i++) {
        // Генерация случайного r
        do {
            mpz_urandomm(r, state, n);
        } while (mpz_cmp_ui(r, 0) == 0);
        
        // Вычисление x = r^2 mod n
        mpz_mul(x, r, r);
        mpz_mod(x, x, n);
        
        // Отправка x verifier'у
        send_mpz(verifier_sock, x);
        printf("Prover: sent x to verifier (iteration %d)\n", i+1);
        
        // Получение e от verifier'а
        recv_mpz(verifier_sock, e);
        printf("Prover: received e = %d from verifier\n", mpz_get_ui(e));
        
        // Вычисление y = r * s^e mod n
        mpz_t s_pow_e;
        mpz_init(s_pow_e);
        fast_power_mod(s_pow_e, s, e, n);
        mpz_mul(y, r, s_pow_e);
        mpz_mod(y, y, n);
        mpz_clear(s_pow_e);
        
        // Отправка y verifier'у
        send_mpz(verifier_sock, y);
        printf("Prover: sent y to verifier\n");
    }
    
    mpz_clears(n, s, v, x, r, e, y, NULL);
    gmp_randclear(state);
    close(verifier_sock);
    close(server_fd);
    return 0;
}
