#ifndef SIGN_H
#define SIGN_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    HASH_SHA256,
    HASH_SHA512
} HashAlgorithm;

typedef enum {
    RSA,
    EL_GAMAL,
    FIAT_SHAMIR 
} EncodeAlgorithm;

typedef struct {
    uint8_t *signature;          // Подпись пользователя
    size_t signature_len;
    char timestamp[20];          // Временная метка в формате UTC
    HashAlgorithm hash_algo;
    EncodeAlgorithm encode_algo;
    char signer_name[128];
    uint8_t *ts_signature;       // Подпись TSA
    size_t ts_signature_len;
} CAdESSignature;

// Высокоуровневые функции для работы с подписями
bool RSA_sign(const char *filename, const char *private_key_file, 
              const char *public_key_file, HashAlgorithm hash_algo,
              const char *signer_name, CAdESSignature *sig, int verbose);

bool RSA_verify(const char *filename, const char *public_key_file,
                const CAdESSignature *sig, int verbose);

bool cades_save_signature(const char *filename, const CAdESSignature *sig);
bool cades_load_signature(const char *filename, CAdESSignature *sig);
void cades_free_signature(CAdESSignature *sig);

// Функции для работы с TSA
bool request_tsa_signature(const CAdESSignature *sig, uint8_t **ts_signature, size_t *ts_len);
bool verify_tsa_signature(const CAdESSignature *sig);

#endif // SIGN_H
