#ifndef SIGN_H
#define SIGN_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Тип хеш-функции (SHA-256/SHA-512)
typedef enum {
    HASH_SHA256,
    HASH_SHA512
} HashAlgorithm;

// Структура подписи CAdES (упрощённая)
typedef struct {
    uint8_t *signature;      // Подпись (зашифрованный хеш)
    size_t signature_len;    // Длина подписи
    char timestamp[20];      // Время в формате "YYYYMMDDHHMMSSZ"
    HashAlgorithm hash_algo; // Использованный алгоритм хеширования
    char signer_name[128];   // Имя подписанта
} CAdESSignature;

// Подписать файл
bool cades_sign_file(
    const char *filename,
    const char *private_key_file,
    HashAlgorithm hash_algo,
    const char *signer_name,
    CAdESSignature *sig,
    int verbose
);

// Проверить подпись
bool cades_verify_file(
    const char *filename,
    const char *public_key_file,
    const CAdESSignature *sig,
    int verbose
);

// Сохранить подпись в файл
bool cades_save_signature(const char *filename, const CAdESSignature *sig);

// Загрузить подпись из файла
bool cades_load_signature(const char *filename, CAdESSignature *sig);

// Освободить память подписи
void cades_free_signature(CAdESSignature *sig);

#endif // SIGN_H
