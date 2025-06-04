#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
#include "common.h"
#include "../essential_func.h"

void load_public_key(mpz_t n, mpz_t v) {
    FILE *pub = fopen("fiat_id.pub", "r");
    if (!pub) handle_error("Failed to open public key");
    
    char buf_n[MAX_BUF], buf_v[MAX_BUF];
    fgets(buf_n, sizeof(buf_n), pub);
    fgets(buf_v, sizeof(buf_v), pub);
    
    mpz_set_str(n, buf_n, 16);
    mpz_set_str(v, buf_v, 16);
    
    fclose(pub);
}

int main() {
    mpz_t n, v, x, e, y, check;
    gmp_randstate_t state;
    
    mpz_inits(n, v, x, e, y, check, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Загрузка публичного ключа
    load_public_key(n, v);
    gmp_printf("Verifier: loaded n = %Zx\nv = %Zx\n", n, v);

    // Подключение к Prover
    int prover_sock = connect_to("127.0.0.1", PORT_PROVER);

    // Протокол идентификации
    uint8_t validated[1]; 
    validated[0] = 1;

    for (int i = 0; i < 20; i++) {
        // Шаг 2: Получение x
        recv_mpz(prover_sock, x);

        // Шаг 3: Отправка случайного e (0 или 1)
        mpz_set_ui(e, gmp_urandomm_ui(state, 2));
        send_mpz(prover_sock, e);

        // Шаг 5: Получение y
        recv_mpz(prover_sock, y);

        // Проверка y² ≡ x * v^e mod n
        mpz_powm_ui(check, y, 2, n);
        
        mpz_t v_pow_e, x_v_e;
        mpz_inits(v_pow_e, x_v_e, NULL);
        fast_power_mod(v_pow_e, v, e, n);
        mpz_mul(x_v_e, x, v_pow_e);
        mpz_mod(x_v_e, x_v_e, n);
        
        if (mpz_cmp(check, x_v_e) != 0) {
            printf("Verification failed at iteration %d!\n", i+1);
            validated[0] = 0;
            break;
        }
        printf("Iteration %d: success\n", i+1);
        
        mpz_clears(v_pow_e, x_v_e, NULL);
    }
    send(prover_sock, validated, 1, 0);
    if (validated)
        printf("SUCCESSFUL IDENTIFICATION!\n");

    mpz_clears(n, v, x, e, y, check, NULL);
    gmp_randclear(state);
    close(prover_sock);
    return 0;
}
