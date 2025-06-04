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

    mpz_t n, v, x, e, y, y_sq, x_v_e;
    gmp_randstate_t state;
    
    mpz_inits(n, v, x, e, y, y_sq, x_v_e, NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    
    // Получение модуля n от доверенного центра
    int trusted_sock = connect_to_server(argv[1], PORT_TRUSTED);
    recv_mpz(trusted_sock, n);
    close(trusted_sock);
    
    printf("Verifier: received modulus n from trusted center\n");
    
    // Подключение к prover'у
    int prover_sock = connect_to_server("127.0.0.1", PORT_PROVER);
    printf("Verifier: connected to prover\n");
    
    // Получение публичного ключа v от prover'а
    recv_mpz(prover_sock, v);
    printf("Verifier: received public key v from prover\n");
    
    // Итеративная часть протокола
    for (int i = 0; i < 10; i++) {
        // Получение x от prover'а
        recv_mpz(prover_sock, x);
        printf("Verifier: received x from prover (iteration %d)\n", i+1);
        
        // Генерация случайного e (0 или 1)
        mpz_set_ui(e, gmp_urandomm_ui(state, 2));
        
        // Отправка e prover'у
        send_mpz(prover_sock, e);
        printf("Verifier: sent e = %d to prover\n", mpz_get_ui(e));
        
        // Получение y от prover'а
        recv_mpz(prover_sock, y);
        printf("Verifier: received y from prover\n");
        
        // Проверка: y^2 ≡ x * v^e mod n
        mpz_mul(y_sq, y, y);
        mpz_mod(y_sq, y_sq, n);
        
        mpz_t v_pow_e;
        mpz_init(v_pow_e);
        fast_power_mod(v_pow_e, v, e, n);
        mpz_mul(x_v_e, x, v_pow_e);
        mpz_mod(x_v_e, x_v_e, n);
        mpz_clear(v_pow_e);
        
        if (mpz_cmp(y_sq, x_v_e) == 0) {
            printf("Verifier: verification passed (iteration %d)\n", i+1);
        } else {
            printf("Verifier: verification FAILED! Possible attack!\n");
            break;
        }
    }
    
    mpz_clears(n, v, x, e, y, y_sq, x_v_e, NULL);
    gmp_randclear(state);
    close(prover_sock);
    return 0;
}
