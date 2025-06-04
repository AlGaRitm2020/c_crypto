#include "common.h"
#include <openssl/sha.h>

#define ID "Client_B"
#define PASSWORD_FILE "weak_password.txt"
#define SHA256_DIGEST_LENGTH 32
#define MAX_ITER 999 

void fix_endian(uint8_t *hash, size_t len) {
    for (size_t i = 0; i < len; i += 4) {
        uint32_t *word = (uint32_t *)(hash + i);
        *word = htonl(*word);
    }
}

void compute_hash_chain(const char *password, int iterations, uint8_t *result) {
    uint8_t temp_hash[SHA256_DIGEST_LENGTH];
    SHA256((const uint8_t *)password, strlen(password), temp_hash);
    fix_endian(temp_hash, SHA256_DIGEST_LENGTH);

    for (int i = 1; i < iterations; i++) {
        SHA256(temp_hash, SHA256_DIGEST_LENGTH, temp_hash);
        fix_endian(temp_hash, SHA256_DIGEST_LENGTH);
    }
    memcpy(result, temp_hash, SHA256_DIGEST_LENGTH);
}

int main() {
    FILE *pw_file = fopen(PASSWORD_FILE, "r");
    if (!pw_file) handle_error("Failed to open password file");
    char password[256];
    fscanf(pw_file, "%255s", password);
    fclose(pw_file);

    int sock = connect_to("127.0.0.1", 8080);

    while (1) {
        int iteration;
        printf("[B] Enter iteration (from 1): ");
        scanf("%d", &iteration);
        if (iteration <= 0) break;

        // Вычисляем h^{MAX_ITER - iteration + 1}(password)
        uint8_t current_hash[SHA256_DIGEST_LENGTH];
        compute_hash_chain(password, MAX_ITER - iteration + 1, current_hash);

        char auth_msg[1024];
        int msg_len = snprintf(auth_msg, sizeof(auth_msg), "%s|%d|", ID, iteration);
        memcpy(auth_msg + msg_len, current_hash, SHA256_DIGEST_LENGTH);
        send(sock, auth_msg, msg_len + SHA256_DIGEST_LENGTH, 0);

        char response[256];
        ssize_t resp_len = recv(sock, response, sizeof(response), 0);
        if (resp_len <= 0) handle_error("Connection error");
        response[resp_len] = '\0';

        if (strcmp(response, "OK") != 0) {
            printf("[B] Auth failed: %s\n", response);
            continue;
        }

        // Диффи-Хеллман
        mpz_t p, g, b, B, A, key;
        mpz_inits(p, g, b, B, A, key, NULL);

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

        mpz_import(g, SHA256_DIGEST_LENGTH, -1, 1, 0, 0, current_hash);
        mpz_mod(g, g, p);

        gmp_randstate_t state;
        gmp_randinit_default(state);
        mpz_urandomb(b, state, 256);
        mpz_powm(B, g, b, p);

        recv_mpz(sock, A);
        send_mpz(sock, B);

        mpz_powm(key, A, b, p);
        char key_str[1024];
        mpz_get_str(key_str, 16, key);
        printf("[B] Shared key (iter %d): %s\n", iteration, key_str);

        mpz_clears(p, g, b, B, A, key, NULL);
    }

    close(sock);
    return 0;
}
