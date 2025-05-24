#include "el_gamal.h"
#include "essential_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>
#include <arpa/inet.h> // для htonl / ntohl

// Структура для хранения ключей
typedef struct {
    mpz_t p;
    mpz_t g;
    mpz_t x;
    mpz_t y;
} ElGamalKey;

// Сохранение ключа в файл
static void save_key(const char* filename, const mpz_t p, const mpz_t g, const mpz_t val) {
    FILE* f = fopen(filename, "w");
    if (!f) return;

    char buffer[4096];

    int offset = gmp_sprintf(buffer, "%Zd %Zd\n", p, g);
    offset += gmp_sprintf(buffer + offset, "%Zd\n", val);

    fprintf(f, "%s", buffer);
    fclose(f);
}

// Загрузка ключа из файла
static int load_key(mpz_t p, mpz_t g, mpz_t val, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char line[4096];

    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        return 0;
    }
    if (gmp_sscanf(line, "%Zd %Zd", p, g) != 2) {
        fclose(f);
        return 0;
    }

    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        return 0;
    }
    if (gmp_sscanf(line, "%Zd", val) != 1) {
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

// Генерация ключей
void el_gamal_generate_keys(const char* pub_key_file, const char* priv_key_file, int bits, int verbose) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    ElGamalKey key;
    mpz_inits(key.p, key.g, key.x, key.y, NULL);

    // Генерируем простое число p
    generate_prime(key.p, bits, state);

    // Подбираем генератор g (обычно достаточно 2)
    mpz_set_ui(key.g, 2);

    // Генерируем закрытый ключ x ∈ [1, p-2]
    mpz_sub_ui(key.x, key.p, 2);
    mpz_urandomm(key.x, state, key.x);
    mpz_add_ui(key.x, key.x, 1);

    // Вычисляем открытый ключ: y = g^x mod p
    fast_power_mod(key.y, key.g, key.x, key.p);

    // Сохраняем ключи
    save_key(priv_key_file, key.p, key.g, key.x);
    save_key(pub_key_file, key.p, key.g, key.y);

    if (verbose) {
        gmp_printf("Сгенерированы ключи ElGamal (%d бит):\n", bits);
        gmp_printf("p = %Zd\n", key.p);
        gmp_printf("g = %Zd\n", key.g);
        gmp_printf("Приватный ключ x = %Zd\n", key.x);
        gmp_printf("Публичный ключ y = %Zd\n", key.y);
    }

    mpz_clears(key.p, key.g, key.x, key.y, NULL);
    gmp_randclear(state);
}

// Шифрование сообщения
void el_gamal_encode(char* message, size_t size, char* priv_key_file,
                     char** enc_message, size_t* enc_message_len, int verbose) {
    ElGamalKey key;
    mpz_inits(key.p, key.g, key.x, key.y, NULL);

    if (!load_key(key.p, key.g, key.x, priv_key_file)) {
        if (verbose) fprintf(stderr, "Ошибка загрузки приватного ключа\n");
        mpz_clears(key.p, key.g, key.x, key.y, NULL);
        return;
    }

    mpz_t m, k, c1, c2;
    mpz_inits(m, k, c1, c2, NULL);

    // Конвертируем сообщение в GMP-число
    mpz_import(m, size, 1, 1, 0, 0, message);

    // Генерируем случайное k ∈ [1, p-2]
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    mpz_sub_ui(k, key.p, 2);
    mpz_urandomm(k, state, k);
    mpz_add_ui(k, k, 1);

    // Вычисляем c1 = g^k mod p
    fast_power_mod(c1, key.g, k, key.p);

    // Вычисляем c2 = m * y^k mod p
    fast_power_mod(c2, key.y, k, key.p);
    mpz_mul(c2, c2, m);
    mpz_mod(c2, c2, key.p);

    char c1_str[1000], c2_str[1000], enc[4096];
    gmp_sprintf(c1_str, "%Zx",c1);
    gmp_sprintf(c2_str, "%Zx",c2);
    gmp_sprintf(enc, "%Zx%Zx",c1,c2);

    // // Экспорт c1 и c2
    // size_t c1_bits = mpz_sizeinbase(c1, 2);
    // size_t c2_bits = mpz_sizeinbase(c2, 2);
    // size_t c1_bytes = (c1_bits + 7) / 8;
    // size_t c2_bytes = (c2_bits + 7) / 8;
    //
    *enc_message_len = 1 + strlen(c1_str) + strlen(c2_str);
    *enc_message = (char*)malloc(*enc_message_len);

    // Пишем длину c1 в начало
    uint32_t len_net = strlen(c1_str);
    memcpy(*enc_message, &len_net, 4);
    memcpy(*enc_message+4, enc, *enc_message_len);
    // Экспортируем c1 и c2
    // mpz_export(*enc_message + 4, &c1_bytes, 1, 1, 0, 0, c1);
    // mpz_export(*enc_message + 4 + c1_bytes, &c2_bytes, 1, 1, 0, 0, c2);

    if (verbose) {
        printf("ElGamal: зашифровано %zu -> %zu байт\n", size, *enc_message_len);
    }

    mpz_clears(m, k, c1, c2, key.p, key.g, key.x, key.y, NULL);
    gmp_randclear(state);
}

// Дешифрование сообщения
void el_gamal_decode(char* enc_message, size_t enc_size, char* pub_key_file,
                     char** dec_message, size_t* dec_message_len, int verbose) {
    ElGamalKey key;
    mpz_inits(key.p, key.g, key.y, NULL);

    if (!load_key(key.p, key.g, key.y, pub_key_file)) {
        if (verbose) fprintf(stderr, "Ошибка загрузки публичного ключа\n");
        mpz_clears(key.p, key.g, key.y, NULL);
        return;
    }

    // Читаем длину c1
    uint32_t c1_len_net;
    char *c1_str, *c2_str;
    memcpy(&c1_len_net, enc_message, 4);
    memcpy(&c1_str, enc_message+4, c1_len_net);
    memcpy(&c1_str, enc_message+4+c1_len_net, enc_size - c1_len_net - 4);

    size_t c1_len = ntohl(c1_len_net);
    size_t c2_len = enc_size - 4 - c1_len;

    mpz_t c1, c2, s, m;
    mpz_inits(c1, c2, s, m, NULL);

    // Импортируем c1 и c2
    // mpz_import(c1, 1, 1, 1, 0, 0, enc_message + 4);
    // mpz_import(c2, 1, 1, 1, 0, 0, enc_message + 4 + c1_len);

    // Расшифровываем: m = c2 * (c1^x)^-1 mod p
    fast_power_mod(s, c1, key.x, key.p); // s = c1^x mod p
    if (!mpz_invert(s, s, key.p)) {
        fprintf(stderr, "Ошибка инверсии\n");
        goto cleanup;
    }
    mpz_mul(m, c2, s);      // m = c2 * s
    mpz_mod(m, m, key.p);   // m = m mod p

    // Экспортируем результат
    size_t m_bits = mpz_sizeinbase(m, 2);
    *dec_message_len = (m_bits + 7) / 8;
    *dec_message = malloc(*dec_message_len);
    mpz_export(*dec_message, dec_message_len, 1, 1, 0, 0, m);

cleanup:
    if (verbose) {
        printf("ElGamal: расшифровано %zu -> %zu байт\n", enc_size, *dec_message_len);
    }

    mpz_clears(c1, c2, s, m, key.p, key.g, key.y, NULL);
}



#ifndef LIB
int main() {
    printf("1. Генерация ключей...\n");
    el_gamal_generate_keys("public.key", "private.key", 256, 1);

    const char message[] = "Hello ElGamal!";
    size_t msg_len = strlen(message) + 1;

    printf("\n2. Шифрование сообщения...\n");
    char* encrypted = NULL;
    size_t enc_len = 0;
    el_gamal_encode((char*)message, msg_len, "private.key", &encrypted, &enc_len, 1);

    printf("\n3. Дешифрование сообщения...\n");
    char* decrypted = NULL;
    size_t dec_len = 0;
    el_gamal_decode(encrypted, enc_len, "public.key", &decrypted, &dec_len, 1);

    printf("\nРезультат: %s\n", decrypted);

    free(encrypted);
    free(decrypted);
    return 0;
}
#endif
