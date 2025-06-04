#include "common.h"

int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    mpz_t p, g, a, A, B, key;
    mpz_inits(p, g, a, A, B, key, NULL);

    // Шаг 1: клиентА генерирует параметры
    printf("ClientA: Generate prime p...\n");
    generate_prime(p, 512, state);

    printf("ClientA: Generate primitive root g...\n");
    generate_primitive_root(g, p, state);

    mpz_urandomb(a, state, 256);
    fast_power_mod(A, g, a, p);

    // Запуск сервера
    int server_fd = create_server_socket(8080);
    printf("ClientA waits for second peer...\n");
    int client_fd = accept(server_fd, NULL, NULL);

    send_mpz(client_fd, p);
    send_mpz(client_fd, g);
    send_mpz(client_fd, A);

    recv_mpz(client_fd, B);

    // Вычисляем общий секрет
    fast_power_mod(key, B, a, p);
    char key_str[1024];
    mpz_get_str(key_str, 16, key);
    printf("Shared key with B : %s\n", key_str);

    close(client_fd);
    close(server_fd);
    mpz_clears(p, g, a, A, B, key, NULL);
    return 0;
}
