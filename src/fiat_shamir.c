#include "fiat_shamir.h"
#include "essential_func.h"
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "sha.h"

// Вспомогательная функция: конкатенация строк
static char* concat(const char* s1, size_t len1, const char* s2) {
    char* result = malloc(len1 + strlen(s2) + 1);
    memcpy(result, s1, len1);
    strcpy(result + len1, s2);
    return result;
}

// Генерация ключей (многораундовая версия)
void fs_gen_key(int bits, int rounds, const char* pubKeyFile, const char* priKeyFile, int verbose) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    mpz_t n, p, q;
    mpz_inits(n, p, q, NULL);
    generate_prime(p, bits/2, state);
    generate_prime(q, bits/2, state);
    mpz_mul(n, p, q);

    // Генерация s₁,...,sₖ и v₁,...,vₖ
    mpz_t s[rounds], v[rounds];
    for (int i = 0; i < rounds; i++) {
        mpz_inits(s[i], v[i], NULL);
        do {
            mpz_urandomm(s[i], state, n);
            mpz_gcd(v[i], s[i], n);  // Проверка взаимной простоты
        } while (mpz_cmp_ui(v[i], 1) != 0);
        mpz_powm_ui(v[i], s[i], 2, n);  // vᵢ = sᵢ² mod n
    }

    // Сохранение ключей
    FILE* pub = fopen(pubKeyFile, "w");
    FILE* pri = fopen(priKeyFile, "w");
    if (!pub || !pri) {
        perror("Error opening key files");
        exit(EXIT_FAILURE);
    }

    // pubKeyFile: n, v₁,...,vₖ
    char buffer[128];
    gmp_sprintf(buffer, "%Zx\n", n);
    // gmp_sprintf()
    // fprintf(pri, "%Zx\n", n);
    fprintf(pub, "%s", buffer); 
    for (int i = 0; i < rounds; i++) {
        gmp_sprintf(buffer, "%Zx\n", v[i]);
        fprintf(pub, "%s", buffer); 
        // fprintf(pub, "%Zx\n", v[i]);
    }

    // priKeyFile: n, s₁,...,sₖ
    // fprintf(pri, "%Zx\n", n);
    gmp_sprintf(buffer, "%Zx\n", n);
    fprintf(pri, "%s", buffer); 
    for (int i = 0; i < rounds; i++) {
        gmp_sprintf(buffer, "%Zx\n", s[i]);
        fprintf(pri, "%s", buffer); 
        // fprintf(pri, "%Zx\n", s[i]);
    }

    fclose(pub);
    fclose(pri);

    if (verbose) {
        gmp_printf("Public Key (n, v₁,...,vₖ):\nn=%Zx\n", n);
        for (int i = 0; i < rounds; i++) {
            gmp_printf("v%d=%Zx\n", i+1, v[i]);
        }
    }

    // Очистка
    for (int i = 0; i < rounds; i++) {
        mpz_clears(s[i], v[i], NULL);
    }
    mpz_clears(n, p, q, NULL);
    gmp_randclear(state);
}

// Подпись сообщения
void fs_sign(const char* message, size_t size, const char* priKeyFile, int rounds, char** signature, size_t* sig_len, int verbose) {
    mpz_t n, r, x, y;
    mpz_t* s = malloc(rounds * sizeof(mpz_t));
    mpz_inits(n, r, x, y, NULL);

    // Загрузка приватного ключа (n, s₁,...,sₖ)
    FILE* pri = fopen(priKeyFile, "r");
    if (!pri) {
        perror("Error opening private key file");
        exit(EXIT_FAILURE);
    }
    char line[4096];
    fgets(line, sizeof(line), pri); mpz_set_str(n, line, 16);
    for (int i = 0; i < rounds; i++) {
        mpz_init(s[i]);
        fgets(line, sizeof(line), pri); mpz_set_str(s[i], line, 16);
    }
    fclose(pri);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // 1. Генерация rᵢ и xᵢ = rᵢ² mod n
    char* x_concat = malloc(1);
    x_concat[0] = '\0';
    mpz_t* y_vals = malloc(rounds * sizeof(mpz_t));
    for (int i = 0; i < rounds; i++) {
        mpz_inits(y_vals[i], NULL);
        do {
            mpz_urandomm(r, state, n);
            mpz_gcd(x, r, n);
        } while (mpz_cmp_ui(x, 1) != 0);
        mpz_powm_ui(x, r, 2, n);

        char* x_str = mpz_get_str(NULL, 16, x);
        char* new_x_concat = concat(x_concat, strlen(x_concat), x_str);
        free(x_concat);
        free(x_str);
        x_concat = new_x_concat;
    }

    uint8_t *hash= (uint8_t*)malloc(32*sizeof(uint8_t));
    char* combined = concat(message, size, x_concat);
    sha256((void**)&combined, strlen(combined), (void**)&hash);
    free(combined);
    

    //yᵢ = rᵢ * sᵢ^eᵢ mod n
    char* sig = malloc(1);
    sig[0] = '\0';
    for (int i = 0; i < rounds; i++) {
        int e_i = (hash[i / 8] >> (i % 8)) & 1;
        mpz_powm_ui(y, s[i], e_i, n);
        mpz_mul(y, y, r);
        mpz_mod(y, y, n);

        // Добавляем yᵢ к подписи
        char* y_str = mpz_get_str(NULL, 16, y);
        char* new_sig = concat(sig, strlen(sig), (i == 0) ? "" : ":");
        new_sig = concat(new_sig, strlen(new_sig), y_str);
        free(sig);
        free(y_str);
        sig = new_sig;
    }

    *signature = sig;
    *sig_len = strlen(sig);

    for (int i = 0; i < rounds; i++) {
        mpz_clears(s[i], y_vals[i], NULL);
    }
    free(s);
    free(y_vals);
    mpz_clears(n, r, x, y, NULL);
    gmp_randclear(state);
}

int fs_verify(const char* message, size_t size, const char* pubKeyFile, int rounds, const char* signature, size_t sig_len, int verbose) {
    mpz_t n, y, x_prime, tmp;
    mpz_t* v = malloc(rounds * sizeof(mpz_t));
    mpz_inits(n, y, x_prime, tmp, NULL);

    FILE* pub = fopen(pubKeyFile, "r");
    if (!pub) {
        perror("Error opening public key file");
        exit(EXIT_FAILURE);
    }
    char line[4096];
    fgets(line, sizeof(line), pub); mpz_set_str(n, line, 16);
    for (int i = 0; i < rounds; i++) {
        mpz_init(v[i]);
        fgets(line, sizeof(line), pub); mpz_set_str(v[i], line, 16);
    }
    fclose(pub);

    // Парсинг y₁:y₂:...:yₖ
    char* sig_copy = strdup(signature);
    char* token = strtok(sig_copy, ":");
    mpz_t* y_vals = malloc(rounds * sizeof(mpz_t));
    for (int i = 0; i < rounds; i++) {
        mpz_init(y_vals[i]);
        if (!token) {
            free(sig_copy);
            return 0;
        }
        mpz_set_str(y_vals[i], token, 16);
        token = strtok(NULL, ":");
    }
    free(sig_copy);

    // x'ᵢ = yᵢ² * vᵢ^{-eᵢ} mod n
    char* x_concat = malloc(1);
    x_concat[0] = '\0';
    uint8_t hash[32];
    for (int i = 0; i < rounds; i++) {
        int e_i = (hash[i / 8] >> (i % 8)) & 1;
        mpz_powm_ui(x_prime, y_vals[i], 2, n);
        if (e_i == 1) {
            mpz_invert(tmp, v[i], n);
            mpz_mul(x_prime, x_prime, tmp);
            mpz_mod(x_prime, x_prime, n);
        }

        char* x_str = mpz_get_str(NULL, 16, x_prime);
        char* new_x_concat = concat(x_concat, strlen(x_concat), x_str);
        free(x_concat);
        free(x_str);
        x_concat = new_x_concat;
    }

    // Проверяем хэш
    uint8_t *new_hash= (uint8_t*)malloc(32*sizeof(uint8_t));
    char* combined = concat(message, size, x_concat);
    sha256((void**)&combined, strlen(combined), (void**)&new_hash);

    free(combined);

    for (int i = 0; i < rounds; i++) {
        int new_e_i = (new_hash[i / 8] >> (i % 8)) & 1;
        int old_e_i = (hash[i / 8] >> (i % 8)) & 1;
        if (new_e_i != old_e_i) {
            return 0;
        }
    }

    return 1;
}

#ifndef LIB
int main() {
    const char* message = "Test";
    printf("Testsignature scheme\n");
    
    // Generate keys
    fs_gen_key(1024, 20, "fs.pub", "fs.pri", 1);
    
    // Sign message
    char* signature = NULL;
    size_t sig_len = 0;
    fs_sign((char*)message, strlen(message), "fs.pri", 20, &signature, &sig_len, 1);
    printf("Signature: %s\n", signature);
    
    // Verify signature
    int valid = fs_verify((char*)message, strlen(message), "fs.pub",20, signature, sig_len, 1);
    printf("Verification: %s\n", valid ? "SUCCESS" : "FAILED");
    
    // Cleanup
    free(signature);
    
    return 0;
}
#endif
