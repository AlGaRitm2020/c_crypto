#include "common.h"


int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    mpz_t p, g, b, B, A, key;
    mpz_inits(p, g, b, B, A, key, NULL);

    // Подключаемся к серверу
    int sock = connect_to("127.0.0.1", 8080);

    // Получаем p и g от сервера
    recv_mpz(sock, p);
    recv_mpz(sock, g);
    recv_mpz(sock, A);

    // Генерируем закрытый ключ
    mpz_urandomb(b, state, 256);
    fast_power_mod(B, g, b, p);

    // Отправляем B серверу
    send_mpz(sock, B);

    // Вычисляем общий секрет
    fast_power_mod(key, A, b, p);
    char key_str[1024];
    mpz_get_str(key_str, 16, key);
    printf("Общий ключ (клиент): %s\n", key_str);

    close(sock);
    mpz_clears(p, g, b, B, A, key, NULL);
    return 0;
}
