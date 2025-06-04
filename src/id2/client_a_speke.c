#include "common.h"
#include <openssl/sha.h>

#define ID "Client_A"
#define PASSWORD_FILE "weak_password.txt"
#define SHA256_DIGEST_LENGTH 32

void fix_endian(uint8_t *hash, size_t len) {
    for (size_t i = 0; i < len; i += 4) {
        uint32_t *word = (uint32_t *)(hash + i);
        uint32_t temp = *word;
        *word = ((temp >> 24) & 0xFF) | ((temp >> 8) & 0xFF00) | ((temp << 8) & 0xFF0000) | ((temp << 24) & 0xFF000000);
    }
}

void compute_hash(const char *password, int iterations, uint8_t *result) {
    uint8_t *hash = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
    SHA256((const uint8_t *)password, strlen(password), hash);
    fix_endian(hash, SHA256_DIGEST_LENGTH);

    for (int i = 1; i < iterations; i++) {
        uint8_t *new_hash = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
        SHA256(hash, SHA256_DIGEST_LENGTH, new_hash);
        fix_endian(new_hash, SHA256_DIGEST_LENGTH);
        free(hash);
        hash = new_hash;
    }
    memcpy(result, hash, SHA256_DIGEST_LENGTH);
    free(hash);
}

int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    // Чтение пароля из файла
    FILE *pw_file = fopen(PASSWORD_FILE, "r");
    if (!pw_file) handle_error("Failed to open password file");
    char password[256];
    fscanf(pw_file, "%255s", password);
    fclose(pw_file);

    // Вычисление g = H(password)^2 mod p
    uint8_t hash[SHA256_DIGEST_LENGTH];
    compute_hash(password, 1, hash);
    mpz_t p, g, a, A, B, key;
    mpz_inits(p, g, a, A, B, key, NULL);

    // Чтение p из файла (должно быть заранее сгенерировано)
    FILE *fp = fopen("diff.key", "r");
    if (!fp) handle_error("Failed to open diff.key");
    char line[4096];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "p=", 2) == 0) {
            mpz_set_str(p, line + 2, 16);
            break;
        }
    }
    fclose(fp);

    // Преобразование хэша пароля в g
    mpz_import(g, SHA256_DIGEST_LENGTH, -1, 1, 0, 0, hash);
    mpz_powm_ui(g, g, 2, p); // g = H(password)^2 mod p

    // Генерация личного ключа
    mpz_urandomb(a, state, 256);
    mpz_powm(A, g, a, p); // A = g^a mod p

    // Запуск сервера
    int server_fd = create_server_socket(8080);
    printf("Client A waits for connection...\n");
    int client_fd = accept(server_fd, NULL, NULL);

    // Обмен ключами
    send_mpz(client_fd, A);
    recv_mpz(client_fd, B);

    // Вычисление общего ключа
    mpz_powm(key, B, a, p); // key = B^a mod p
    char key_str[1024];
    mpz_get_str(key_str, 16, key);
    printf("Shared key: %s\n", key_str);

    // Очистка
    close(client_fd);
    close(server_fd);
    mpz_clears(p, g, a, A, B, key, NULL);
    return 0;
}
