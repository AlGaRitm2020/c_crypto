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
    RABBIN
} EncodeAlgorithm;

typedef struct {
    uint8_t *signature;
    size_t signature_len;
    char timestamp[20];
    HashAlgorithm hash_algo;
    EncodeAlgorithm encode_algo;
    char signer_name[128];
    uint8_t *ts_signature;
    size_t ts_signature_len;
} CAdESSignature;

// bool cades_sign_file(
//     const char *filename,
//     const char *private_key_file,
//     HashAlgorithm hash_algo,
//     EncodeAlgorithm encode_algo,
//     const char *signer_name,
//     CAdESSignature *sig,
//     int verbose
// );
//
// bool cades_verify_file(
//     const char *filename,
//     const char *public_key_file,
//     const CAdESSignature *sig,
//     int verbose
// );
//
bool cades_save_signature(const char *filename, const CAdESSignature *sig);
bool cades_load_signature(const char *filename, CAdESSignature *sig);
void cades_free_signature(CAdESSignature *sig);
//
#endif // SIGN_H
