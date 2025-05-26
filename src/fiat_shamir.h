#ifndef FIAT_SHAMIR_H
#define FIAT_SHAMIR_H

#include <gmp.h>
#include <stdbool.h>

void fs_gen_key(int bits, char* pubKeyFile, char* priKeyFile, int verbose);
void fs_load_key(mpz_t n, mpz_t key, char* filename, int verbose);
void fs_sign(char* message, size_t size, char* priKeyFile, char** signature, size_t* signature_len, int verbose);
int fs_verify(char* message, size_t size, char* pubKeyFile, char* signature, size_t signature_len, int verbose);

#endif
