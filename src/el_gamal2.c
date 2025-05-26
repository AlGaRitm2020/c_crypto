#include "essential_func.h"
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define CHUNKSIZE 32 

void elgamal_save_key(mpz_t p, mpz_t g, mpz_t key, char* filename, int verbose) {
    if (verbose) printf("SAVING TO %s...\n", filename);
    FILE* stream = fopen(filename, "w");
    if (!stream) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char buffer[4096];
    gmp_sprintf(buffer, "%Zd\n%Zd\n%Zd\n", p, g, key);
    fprintf(stream, "%s", buffer);
    fclose(stream);
    if (verbose) printf("Success!\n");
}

void elgamal_load_key(mpz_t p, mpz_t g, mpz_t key, char* filename, int verbose) {
    if (verbose) printf("LOADING %s...\n", filename);
    FILE* stream = fopen(filename, "r");
    if (!stream) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    size_t size = 2048;
    char *line = malloc(size);

    getline(&line, &size, stream); mpz_set_str(p, line, 10);
    getline(&line, &size, stream); mpz_set_str(g, line, 10);
    getline(&line, &size, stream); mpz_set_str(key, line, 10);

    free(line);
    fclose(stream);
    if (verbose) gmp_printf("Loaded:\np=%Zd\ng=%Zd\nkey=%Zd\n", p, g, key);
}

void elgamal_gen_key(int bits, char* pubKeyFile, char* priKeyFile, int verbose) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    mpz_t p, g, x, y, k;
    mpz_inits(p, g, x, y, k, NULL);

    generate_prime(p, bits, state); // large prime
    mpz_urandomm(g, state, p);
    while (mpz_cmp_ui(g, 1) <= 0) mpz_add_ui(g, g, 1);

    mpz_urandomm(x, state, p); // private key
    fast_power_mod(y, g, x, p); // public key y = g^x mod p

    elgamal_save_key(p, g, y, pubKeyFile, verbose);
    elgamal_save_key(p, g, x, priKeyFile, verbose);

    if (verbose) {
        gmp_printf("Public Key:\np=%Zd\ng=%Zd\ny=%Zd\n", p, g, y);
        gmp_printf("Private Key:\nx=%Zd\n", x);
    }

    mpz_clears(p, g, x, y, k, NULL);
    gmp_randclear(state);
}

void pkcs7_pad(uint8_t *data, size_t data_len, uint8_t* padded_data, size_t* total_len, size_t BLOCKSIZE) {
    uint8_t pad_len = BLOCKSIZE - (data_len % BLOCKSIZE);
    *total_len = data_len + pad_len;
    memcpy(padded_data, data, data_len);
    memset(padded_data + data_len, pad_len, pad_len);
}

int pkcs7_unpad(uint8_t* padded_data, size_t total_len, uint8_t* data, size_t* data_len) {
    if (total_len == 0) return 1;
    uint8_t pad_len = padded_data[total_len - 1];
    *data_len = total_len - pad_len;
    for (size_t i = *data_len; i < total_len; i++) {
        if (padded_data[i] != pad_len) return 1;
    }
    memcpy(data, padded_data, *data_len);
    return 0;
}

void elgamal_encode(char* message, size_t size, char* pubKeyFile, char** enc_message, size_t* enc_message_len, int verbose) {
    mpz_t p, g, y, m, c1, c2, k;
    mpz_inits(p, g, y, m, c1, c2, k, NULL);

    elgamal_load_key(p, g, y, pubKeyFile, verbose);

    size_t padded_size;
    uint8_t* padded = calloc(1, (size / CHUNKSIZE + 1) * CHUNKSIZE);
    pkcs7_pad((uint8_t*)message, size, padded, &padded_size, CHUNKSIZE);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    char* result = calloc(1, padded_size * 2 * 256);
    for (int j = 0; j < padded_size; j += CHUNKSIZE) {
        mpz_import(m, CHUNKSIZE, 1, 1, 0, 0, padded + j);
        mpz_urandomm(k, state, p);
        fast_power_mod(c1, g, k, p);
        fast_power_mod(c2, y, k, p);
        mpz_mul(c2, c2, m);
        mpz_mod(c2, c2, p);

        char temp[2048];
        gmp_sprintf(temp, "%Zx:%Zx;", c1, c2);
        strcat(result, temp);
    }

    *enc_message = result;
    *enc_message_len = strlen(result);
    mpz_clears(p, g, y, m, c1, c2, k, NULL);
    gmp_randclear(state);
    free(padded);
}

void elgamal_decode(char* ciphertext, size_t size, char* priKeyFile, char** dec_message, size_t* dec_message_len, int verbose) {
    mpz_t p, g, x, m, c1, c2, s, inv_s;
    mpz_inits(p, g, x, m, c1, c2, s, inv_s, NULL);

    elgamal_load_key(p, g, x, priKeyFile, verbose);

    char* token = strtok(ciphertext, ";");
    size_t total_size = 0;
    char* output = calloc(1, size * 2);

    while (token && strlen(token) > 0) {
        char* colon = strchr(token, ':');
        if (!colon) break;
        *colon = '\0';
        mpz_set_str(c1, token, 16);
        mpz_set_str(c2, colon + 1, 16);

        fast_power_mod(s, c1, x, p);
        mpz_invert(inv_s, s, p);
        mpz_mul(m, c2, inv_s);
        mpz_mod(m, m, p);

        size_t count;
        void* chunk = mpz_export(NULL, &count, 1, 1, 0, 0, m);
        memcpy(output + total_size, chunk, count);
        total_size += count;

        free(chunk);
        token = strtok(NULL, ";");
    }

    uint8_t* unpadded = calloc(1, total_size);
    size_t data_len;
    pkcs7_unpad(output, total_size, unpadded, &data_len);

    *dec_message = realloc(unpadded, data_len + 1);
    (*dec_message)[data_len] = '\0';
    *dec_message_len = data_len;

    mpz_clears(p, g, x, m, c1, c2, s, inv_s, NULL);
    free(output);
}

#ifndef LIB
int main() {
    const char* message = "Hello, ElGamal!";
    printf("Original message: %s\n", message);

    // Generate keys
    elgamal_gen_key(1024, "elgamal.pub", "elgamal.pri", 1);

    // Encrypt
    char* encrypted = NULL;
    size_t encrypted_len = 0;
    elgamal_encode((char*)message, strlen(message), "elgamal.pub", &encrypted, &encrypted_len, 1);
    printf("Encrypted: %s\n", encrypted);

    // Decrypt
    char* decrypted = NULL;
    size_t decrypted_len = 0;
    elgamal_decode(encrypted, encrypted_len, "elgamal.pri", &decrypted, &decrypted_len, 1);
    printf("Decrypted: %s\n", decrypted);

    // Cleanup
    free(encrypted);
    free(decrypted);

    return 0;
}
#endif
