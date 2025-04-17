#ifndef RSA_H
#define RSA_H

#include <gmp.h>
#include <stdbool.h>

typedef struct {
    mpz_t modulus;
    mpz_t exponent;
} RSAKey;

/* Key generation */

void rsa_gen_key(int bits, char* pubKeyFile, char* priKeyFile); 
/* Key I/O */
void rsa_save_key(RSAKey *key, const char *filename);
void rsa_load_key(RSAKey *key, const char *filename);

/* Encryption/Decryption */
void rsa_encrypt(mpz_t ciphertext, const char *plaintext, size_t len, RSAKey *public_key);
void rsa_decrypt(char **plaintext, size_t *len, mpz_t ciphertext, RSAKey *private_key);

/* Padding functions */
void rsa_add_padding(char **padded, size_t *padded_len, const char *data, size_t len, size_t block_size);
void rsa_remove_padding(char **data, size_t *len, const char *padded, size_t padded_len);

#endif
