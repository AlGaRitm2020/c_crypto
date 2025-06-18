#include "common.h"
#include "../sha.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ID "Client_A"
#define PASSWORD_FILE "weak_password.txt"
#define MAX_ITER 1000
#define SHA256_DIGEST_LENGTH 32

void fix_endian(uint8_t *hash, size_t len) {
    for (size_t i = 0; i < len; i += 4) {
        uint32_t *word = (uint32_t *)(hash + i);
        *word = htonl(*word);
    }
}

void compute_hash_chain(const char *password, int iterations, uint8_t *result) {
    uint8_t *temp_hash = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
    char *password_copy = strdup(password);
    
    // Первый хэш
    sha256((void **)&password_copy, strlen(password), (void **)&temp_hash);
    fix_endian(temp_hash, SHA256_DIGEST_LENGTH);

    // Последующие итерации
    for (int i = 1; i < iterations; i++) {
        uint8_t *new_hash = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
        uint8_t *temp_copy = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
        memcpy(temp_copy, temp_hash, SHA256_DIGEST_LENGTH);
        
        sha256((void **)&temp_copy, SHA256_DIGEST_LENGTH, (void **)&new_hash);
        fix_endian(new_hash, SHA256_DIGEST_LENGTH);

        free(temp_hash);
        free(temp_copy);
        temp_hash = new_hash;
    }
    memcpy(result, temp_hash, SHA256_DIGEST_LENGTH);
    free(temp_hash);
}

int main() {
    FILE *pw_file = fopen(PASSWORD_FILE, "r");
    if (!pw_file) handle_error("Failed to open password file");
    char password[256];
    fscanf(pw_file, "%255s", password);
    fclose(pw_file);

    int server_fd = create_server_socket(8080);
    printf("[A] Waiting for client B...\n");
    int client_fd = accept(server_fd, NULL, NULL);

    uint8_t stored_hash[SHA256_DIGEST_LENGTH];
    compute_hash_chain(password, MAX_ITER, stored_hash);
    int current_iter = 1;

    while (current_iter <= MAX_ITER) {
        printf("\n[A] Waiting for iteration %d...\n", current_iter);

        char auth_msg[1024];
        ssize_t recv_len = recv(client_fd, auth_msg, sizeof(auth_msg), 0);
        if (recv_len <= 0) handle_error("Failed to receive auth data");

        char *iter_ptr = strchr(auth_msg, '|') + 1;
        int received_iter = atoi(iter_ptr);
        uint8_t *received_hash = (uint8_t *)(strchr(iter_ptr, '|') + 1);

        if (received_iter != current_iter) {
            send(client_fd, "Invalid iteration", 17, 0);
            continue;
        }

        uint8_t *computed_hash = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
        uint8_t *hash_copy = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
        memcpy(hash_copy, received_hash, SHA256_DIGEST_LENGTH);
        
        sha256((void **)&hash_copy, SHA256_DIGEST_LENGTH, (void **)&computed_hash);
        fix_endian(computed_hash, SHA256_DIGEST_LENGTH);

        if (memcmp(computed_hash, stored_hash, SHA256_DIGEST_LENGTH) != 0) {
            send(client_fd, "Authentication failed", 21, 0);
            free(computed_hash);
            free(hash_copy);
            continue;
        }

        send(client_fd, "OK", 2, 0);

        // Диффи-Хеллман
        mpz_t p, g, a, A, B, key;
        mpz_inits(p, g, a, A, B, key, NULL);

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

        mpz_import(g, SHA256_DIGEST_LENGTH, -1, 1, 0, 0, received_hash);
        mpz_mod(g, g, p);

        gmp_randstate_t state;
        gmp_randinit_default(state);
        mpz_urandomb(a, state, 256);
        mpz_powm(A, g, a, p);

        send_mpz(client_fd, A);
        recv_mpz(client_fd, B);

        mpz_powm(key, B, a, p);
        char key_str[1024];
        mpz_get_str(key_str, 16, key);
        printf("[A] Shared key (iter %d): %s\n", current_iter, key_str);

        memcpy(stored_hash, received_hash, SHA256_DIGEST_LENGTH);
        current_iter++;

        mpz_clears(p, g, a, A, B, key, NULL);
        free(computed_hash);
        free(hash_copy);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
