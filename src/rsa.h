#ifndef RSA_H
#define RSA_H

#include <gmp.h>
#include <stdbool.h>

typedef struct {
    mpz_t modulus;
    mpz_t exponent;
} RSAKey;

/* Key generation */

void rsa_gen_key(int bits, char* pubKeyFile, char* priKeyFile, int verbose); 
/* Key I/O */
// void rsa_save_key(RSAKey *key, const char *filename);
// void rsa_load_key(RSAKey *key, const char *filename);
void rsa_load_key(mpz_t n, mpz_t exp, char* filename, int verbose); 
void rsa_save_key(mpz_t n, mpz_t exp, char* filename, int verbose); 

/* Encryption/Decryption */


void rsa_encode(char* message, size_t size, char* pubKeyFile, char** enc_message, size_t* enc_message_len, int verbose); 
void rsa_decode(char* ciphertext, size_t size, char* priKeyFile,char** dec_message, size_t* dec_message_len, int verbose); 
// void rsa_decode(char* ciphertext, size_t size, char* priKeyFile, int verbose) ;

/* Padding functions */
void rsa_add_padding(char **padded, size_t *padded_len, const char *data, size_t len, size_t block_size);
void rsa_remove_padding(char **data, size_t *len, const char *padded, size_t padded_len);

#endif
