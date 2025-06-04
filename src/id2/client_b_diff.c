#include "common.h"

int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    mpz_t p, g, b, B, A, key;
    mpz_inits(p, g, b, B, A, key, NULL);

    // Чтение p и g из файла
    FILE *fp = fopen("diff.key", "r");
    if (!fp) {
        handle_error("Failed to open diff.key");
    }

    char line[4096];
    char *p_str = NULL, *g_str = NULL;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "p=", 2) == 0) {
            p_str = strdup(line + 2);
            p_str[strcspn(p_str, "\n")] = 0;
        } else if (strncmp(line, "g=", 2) == 0) {
            g_str = strdup(line + 2);
            g_str[strcspn(g_str, "\n")] = 0;
        }
    }

    fclose(fp);

    mpz_set_str(p, p_str, 16);
    mpz_set_str(g, g_str, 16);

    free(p_str);
    free(g_str);

    // Подключение к клиенту А
    int sock = connect_to("127.0.0.1", 8080);

    // Получаем A
    recv_mpz(sock, A);

    // Генерируем закрытый ключ
    mpz_urandomb(b, state, 256);
    fast_power_mod(B, g, b, p);

    // Отправляем B
    send_mpz(sock, B);

    // Вычисляем общий секрет
    fast_power_mod(key, A, b, p);
    char key_str[1024];
    mpz_get_str(key_str, 16, key);
    printf("Shared key with A : %s\n", key_str);

    close(sock);
    mpz_clears(p, g, b, B, A, key, NULL);
    return 0;
}
