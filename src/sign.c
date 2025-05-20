#include "sign.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "rsa.h"
#include "sha.h"

// Ваши функции RSA (предполагается, что они уже есть)
// extern void rsa_encode(char *message, size_t size, char *pubKeyFile, char **enc_message, size_t *enc_message_len, int verbose);
// extern void rsa_decode(char *ciphertext, size_t size, char *priKeyFile, char **dec_message, size_t *dec_message_len, int verbose);
//
// // Ваши функции хеширования (предполагается, что они уже есть)
// extern void sha256(const uint8_t *data, size_t len, uint8_t *out);
// extern void sha512(const uint8_t *data, size_t len, uint8_t *out);

// Чтение файла в память
static bool read_file(const char *filename, uint8_t **data, size_t *len) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);

    *data = (uint8_t*)malloc(*len);
    if (!*data) {
        fclose(f);
        return false;
    }

    fread(*data, 1, *len, f);
    fclose(f);
    return true;
}

// Получение текущего времени в формате UTC
static void get_timestamp(char *timestamp) {
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(timestamp, 20, "%Y%m%d%H%M%SZ", tm);
}

// Подписать файл
bool cades_sign_file(
    const char *filename,
    const char *private_key_file,
    HashAlgorithm hash_algo,
    const char *signer_name,
    CAdESSignature *sig,
    int verbose
) {
    uint8_t *file_data;
    size_t file_len;

    // 1. Читаем файл
    if (!read_file(filename, &file_data, &file_len)) {
        if (verbose) fprintf(stderr, "Ошибка чтения файла %s\n", filename);
        return false;
    }
    printf("file data: %s\n", file_data);

    // 2. Вычисляем хеш
    // uint8_t hash[64]={0}; // SHA-512 требует 64 байта
    size_t hash_len = (hash_algo == HASH_SHA256) ? 32 : 64;
    uint8_t* hash = (uint8_t*)malloc(hash_len);

    printf("debugbefore SHA\n");
    if (hash_algo == HASH_SHA256) {
        sha256((void**)&file_data, file_len, (void**)&hash);
    } else {
        sha256((void**)&file_data, file_len, (void**)&hash);
    }

    printf("debug0.3\n");
    // 3. Подписываем хеш (шифруем закрытым ключом)
    char *encrypted_hash = NULL;
    size_t encrypted_hash_len = 0;

    rsa_encode((char *)hash, hash_len, (char *)private_key_file, &encrypted_hash, &encrypted_hash_len, verbose);

    printf("debug0.4\n");
    if (!encrypted_hash) {
        free(file_data);
        return false;
    }

    printf("debug0.5\n");
    // 4. Заполняем структуру подписи
    sig->signature = (uint8_t *)encrypted_hash;
    sig->signature_len = encrypted_hash_len;
    get_timestamp(sig->timestamp);
    sig->hash_algo = hash_algo;
    strncpy(sig->signer_name, signer_name, sizeof(sig->signer_name) - 1);

    printf("file data pointer: %p\n", file_data);
    free(file_data);
    printf("debug0.fin\n");
    return true;
}

// Проверить подпись
bool cades_verify_file(
    const char *filename,
    const char *public_key_file,
    const CAdESSignature *sig,
    int verbose
) {
    uint8_t *file_data;
    size_t file_len;

    if (!read_file(filename, &file_data, &file_len)) {
        if (verbose) fprintf(stderr, "Ошибка чтения файла %s\n", filename);
        return false;
    }

    // 1. Вычисляем хеш
    size_t hash_len = (sig->hash_algo == HASH_SHA256) ? 32 : 64;
    uint8_t* hash = (uint8_t*)malloc(hash_len);

    if (sig->hash_algo == HASH_SHA256) {
        sha256((void**)&file_data, file_len, (void**)&hash);

    } else {
        sha256((void**)&file_data, file_len, (void**)&hash);
    }

    // 2. Расшифровываем подпись (открытым ключом)
    char *decrypted_hash = NULL;
    size_t decrypted_hash_len = 0;

    rsa_decode((char *)sig->signature, sig->signature_len, (char *)public_key_file, &decrypted_hash, &decrypted_hash_len, verbose);

    if (!decrypted_hash) {
        free(file_data);
        return false;
    }

    // 3. Сравниваем хеши
    printf("decr_hash: %s, tested_hash: %s\n", decrypted_hash, (char*)hash);
    bool valid = (memcmp(hash, decrypted_hash, hash_len) == 0);

    free(file_data);
    free(decrypted_hash);
    return valid;
}

// Сохранить подпись в файл
bool cades_save_signature(const char *filename, const CAdESSignature *sig) {
    FILE *f = fopen(filename, "wb");
    if (!f) return false;

    // Формат: [алгоритм][длина подписи][подпись][timestamp][имя]
    fwrite(&sig->hash_algo, sizeof(HashAlgorithm), 1, f);
    fwrite(&sig->signature_len, sizeof(size_t), 1, f);
    fwrite(sig->signature, 1, sig->signature_len, f);
    fwrite(sig->timestamp, 1, 20, f);
    fwrite(sig->signer_name, 1, 128, f);

    fclose(f);
    return true;
}

// Загрузить подпись из файла
bool cades_load_signature(const char *filename, CAdESSignature *sig) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    // Читаем структуру
    fread(&sig->hash_algo, sizeof(HashAlgorithm), 1, f);
    fread(&sig->signature_len, sizeof(size_t), 1, f);

    sig->signature = (uint8_t*)malloc(sig->signature_len);
    if (!sig->signature) {
        fclose(f);
        return false;
    }

    
    fread(sig->signature, 1, sig->signature_len, f);
    fread(sig->timestamp, 1, 20, f);
    fread(sig->signer_name, 1, 128, f);

    fclose(f);
    return true;
}

// Освободить память подписи
void cades_free_signature(CAdESSignature *sig) {
    if (sig->signature) {
        free(sig->signature);
        sig->signature = NULL;
    }
}


// #ifndef SIGN_LIB

int main() {
    const char *filename = "env.sh";
    const char *private_key = "hello";
    const char *public_key = "hello.pub";
    const char *signature_file = "signature.bin";

    CAdESSignature signature;
    int verbose = 1;
    printf("debug1\n");
    // 1. Подписать файл
    if (!cades_sign_file(filename, private_key, HASH_SHA256, "Иван Иванов", &signature, verbose)) {
        printf("Ошибка подписи!\n");
        return 1;
    }

    printf("debug1.1\n");
    printf("Файл подписан!\n");
    printf("Автор: %s\n", signature.signer_name);
    printf("Время: %s\n", signature.timestamp);

    // 2. Сохранить подпись в файл
    if (!cades_save_signature(signature_file, &signature)) {
        printf("Ошибка сохранения подписи!\n");
        cades_free_signature(&signature);
        return 1;
    }

    // 3. Проверить подпись
    CAdESSignature loaded_sig;
    if (!cades_load_signature(signature_file, &loaded_sig)) {
        printf("Ошибка загрузки подписи!\n");
        return 1;
    }

    if (cades_verify_file(filename, public_key, &loaded_sig, verbose)) {
        printf("Подпись верна!\n");
    } else {
        printf("Подпись недействительна!\n");
    }

    cades_free_signature(&signature);
    cades_free_signature(&loaded_sig);
    return 0;
}
// #endif /* ifndef SIGN_LIB */
