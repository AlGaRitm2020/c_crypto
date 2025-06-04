#include "common.h"

int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    mpz_t p, g, a, A, B, key;
    mpz_inits(p, g, a, A, B, key, NULL);

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

    // Генерация личного ключа
    mpz_urandomb(a, state, 256);
    fast_power_mod(A, g, a, p);

    // Запуск сервера
    int server_fd = create_server_socket(8080);
    printf("ClientA waits for second peer...\n");
    int client_fd = accept(server_fd, NULL, NULL);

    // Отправляем A
    send_mpz(client_fd, A);

    // Получаем B
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
