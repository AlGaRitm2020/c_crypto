#include "fiat_shamir.h"
#include "essential_func.h"
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "sha.h"

void fs_gen_key(int bits, char* pubKeyFile, char* priKeyFile, int verbose) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    mpz_t n, v, s;
    mpz_inits(n, v, s, NULL);

    // Генерируем два простых числа p и q
    mpz_t p, q;
    mpz_inits(p, q, NULL);
    generate_prime(p, bits/2, state);
    generate_prime(q, bits/2, state);
    
    // Вычисляем n = p*q
    mpz_mul(n, p, q);
    
    // Генерируем секретное число s (взаимно простое с n)
    do {
        mpz_urandomm(s, state, n);
        mpz_gcd(v, s, n);
    } while (mpz_cmp_ui(v, 1) != 0);
    
    // Вычисляем v = s^2 mod n (публичный ключ)
    mpz_mul(v, s, s);
    mpz_mod(v, v, n);

    // Сохраняем ключи
    FILE* pub = fopen(pubKeyFile, "w");
    FILE* pri = fopen(priKeyFile, "w");
    if (!pub || !pri) {
        perror("Error opening key files");
        exit(EXIT_FAILURE);
    }
    
    char buffer[4096];
    gmp_sprintf(buffer, "%Zd\n%Zd\n", n, v);
    fprintf(pub, "%s", buffer);
    
    gmp_sprintf(buffer, "%Zd\n%Zd\n", n, s);
    fprintf(pri, "%s", buffer);
    
    fclose(pub);
    fclose(pri);
    
    if (verbose) {
        gmp_printf("Public Key (n, v):\n%Zd\n%Zd\n", n, v);
        gmp_printf("Private Key (n, s):\n%Zd\n%Zd\n", n, s);
    }

    mpz_clears(n, v, s, p, q, NULL);
    gmp_randclear(state);
}

void fs_load_key(mpz_t n, mpz_t key, char* filename, int verbose) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Error opening key file");
        exit(EXIT_FAILURE);
    }

    char line[2048];
    fgets(line, sizeof(line), f); mpz_set_str(n, line, 10);
    fgets(line, sizeof(line), f); mpz_set_str(key, line, 10);
    
    fclose(f);
    
    if (verbose) {
        gmp_printf("Loaded key:\nn=%Zd\nkey=%Zd\n", n, key);
    }
}

void fs_sign(char* message, size_t size, char* priKeyFile, char** signature, size_t* signature_len, int verbose) {
    mpz_t n, s, r, x, e, y;
    mpz_inits(n, s, r, x, e, y, NULL);

    // Загружаем приватный ключ
    fs_load_key(n, s, priKeyFile, verbose);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Выбираем случайное r взаимно простое с n
    mpz_t tmp;
    mpz_init(tmp);
    do {
        mpz_urandomm(r, state, n);
        mpz_gcd(tmp, r, n);
    } while (mpz_cmp_ui(tmp, 1) != 0);
    mpz_clear(tmp);

    // Вычисляем x = r^2 mod n
    mpz_mul(x, r, r);
    mpz_mod(x, x, n);

    // Вычисляем e = hash(message || x)
    char* x_str = mpz_get_str(NULL, 16, x);
    size_t x_len = strlen(x_str);
    size_t total_len = size + x_len;
    char* combined = malloc(total_len);
    memcpy(combined, message, size);
    memcpy(combined + size, x_str, x_len);
    
    uint8_t* hash = (uint8_t*)malloc(1 * 32);
    sha256((void**)&combined, total_len, (void**)&hash);
    free(combined);
    free(x_str);

    mpz_import(e, 32, 1, 1, 0, 0, hash);
    free(hash);

    // Вычисляем y = r * s^e mod n
    mpz_powm(y, s, e, n);
    mpz_mul(y, y, r);
    mpz_mod(y, y, n);

    // Формируем подпись в виде "x:y:e"
    char* result = malloc(2048);
    gmp_sprintf(result, "%Zx:%Zx:%Zx", x, y, e);
    
    *signature = result;
    *signature_len = strlen(result);

    mpz_clears(n, s, r, x, e, y, NULL);
    gmp_randclear(state);
}

int fs_verify(char* message, size_t size, char* pubKeyFile, char* signature, size_t signature_len, int verbose) {
    mpz_t n, v, x, y, e, x_verify, left, right;
    mpz_inits(n, v, x, y, e, x_verify, left, right, NULL);

    // Загружаем публичный ключ
    fs_load_key(n, v, pubKeyFile, verbose);

    // Парсим подпись
    char* copy = strdup(signature);
    char* token = strtok(copy, ":");
    if (!token) {
        free(copy);
        return 0;
    }
    mpz_set_str(x, token, 16);
    
    token = strtok(NULL, ":");
    if (!token) {
        free(copy);
        return 0;
    }
    mpz_set_str(y, token, 16);
    
    token = strtok(NULL, ":");
    if (!token) {
        free(copy);
        return 0;
    }
    mpz_set_str(e, token, 16);
    free(copy);

    // Проверяем, что x, y, e в допустимых пределах
    if (mpz_cmp_ui(x, 0) <= 0 || mpz_cmp(x, n) >= 0 ||
        mpz_cmp_ui(y, 0) <= 0 || mpz_cmp(y, n) >= 0 ||
        mpz_cmp_ui(e, 0) < 0) {
        return 0;
    }

    // Вычисляем x_verify = y^2 * v^e mod n
    mpz_powm_ui(left, y, 2, n); // left = y^2 mod n
    mpz_powm(right, v, e, n);   // right = v^e mod n
    mpz_div(x_verify, left, right);
    mpz_mod(x_verify, x_verify, n); // x' = (y^2 * v^e ) (mod n)
                                    // x = r^2 mod n
    // Вычисляем ожидаемое e = hash(message || x)
    char* x_str = mpz_get_str(NULL, 16, x);
    size_t x_len = strlen(x_str);
    size_t total_len = size + x_len;
    char* combined = malloc(total_len);
    memcpy(combined, message, size);
    memcpy(combined + size, x_str, x_len);
    
    // uint8_t hash[32];
    uint8_t *hash = (uint8_t* )malloc(32*1);
    sha256((void**)&combined, total_len, (void**)&hash);
    free(combined);
    free(x_str);

    mpz_t e_expected;
    mpz_init(e_expected);
    mpz_import(e_expected, 32, 1, 1, 0, 0, hash);

    // Подпись верна, если x == x_verify и e == e_expected
    int result = (mpz_cmp(e, e_expected) == 0);

    mpz_clears(n, v, x, y, e, x_verify, left, right, e_expected, NULL);
    return result;
}

#ifndef LIB
int main() {
    const char* message = "Test";
    printf("Testsignature scheme\n");
    
    // Generate keys
    fs_gen_key(1024, "fs.pub", "fs.pri", 1);
    
    // Sign message
    char* signature = NULL;
    size_t sig_len = 0;
    fs_sign((char*)message, strlen(message), "fs.pri", &signature, &sig_len, 1);
    printf("Signature: %s\n", signature);
    
    // Verify signature
    int valid = fs_verify((char*)message, strlen(message), "fs.pub", signature, sig_len, 1);
    printf("Verification: %s\n", valid ? "SUCCESS" : "FAILED");
    
    // Cleanup
    free(signature);
    
    return 0;
}
#endif
