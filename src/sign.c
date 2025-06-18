#include "sign.h"
#include "el_gamal.h"
#include "fiat_shamir.h"
#include "sha.h"
#include "rsa.h"
#include "tsa_client.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

static bool read_file(const char *filename, uint8_t **data, size_t *len) {
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);

    *data = malloc(*len);
    if (!*data) {
        fclose(f);
        return false;
    }
    
    size_t read = fread(*data, 1, *len, f);
    fclose(f);
    return read == *len;
}

static void get_utc_timestamp(char *timestamp, size_t len) {
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(timestamp, len, "%Y%m%d%H%M%SZ", tm);
}

static bool ensure_directory_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755)) {
            perror("mkdir");
            return false;
        }
    }
    return true;
}

static void print_formatted_time(const char *timestamp) {
    int year, month, day, hour, min, sec;
    sscanf(timestamp, "%4d%2d%2d%2d%2d%2dZ", &year, &month, &day, &hour, &min, &sec);
    printf("%02d:%02d:%02d %02d.%02d.%04d", hour, min, sec, day, month, year);
}

bool cades_view_signature(const CAdESSignature *sig) {
    if (!sig) return false;

    printf("\n=== Информация о подписи ===\n");
    printf("Автор:        %s\n", sig->signer_name);
    printf("Алгоритм хеша:     %s\n", sig->hash_algo == HASH_SHA256 ? "SHA-256" : "SHA-512");
    if (sig->encode_algo == RSAA)
      printf("Алгоритм шифрования: RSA \n");
    else if (sig->encode_algo == EL_GAMAL)
      printf("Алгоритм шифрования: EL_GAMAL\n");
    else if (sig->encode_algo == FIAT_SHAMIR)
      printf("Алгоритм шифрования: FIAT_SHAMIR\n");
    else
      printf("Алгоритм шифрования: UNKNOWN\n");

  
    // printf("Алгоритм шифрования:     %s\n", sig->encode_algo == RSA ? "RSA" : "El-Gamal");
    printf("Время:        ");
    print_formatted_time(sig->timestamp);
    printf("\n");
    printf("Длина подписи: %zu байт\n", sig->signature_len);
    if (sig->ts_signature_len > 0) {
        printf("Метка времени: %zu байт\n", sig->ts_signature_len);
    }
    printf("============================\n");
    return true;
}

bool cades_sign_file(
    const char *filename,
    const char *private_key_file,
    const char *public_key_file,
    HashAlgorithm hash_algo,
    EncodeAlgorithm encode_algo,
    const char *signer_name,
    CAdESSignature *sig,
    int verbose
) {
    uint8_t *file_data;
    size_t file_len;

    if (!read_file(filename, &file_data, &file_len)) {
        if (verbose) fprintf(stderr, "Ошибка чтения файла %s\n", filename);
        return false;
    }

    size_t hash_len = (hash_algo == HASH_SHA256) ? 32 : 64;
    uint8_t* hash = malloc(hash_len);
    if (!hash) {
        free(file_data);
        return false;
    }

    if (hash_algo == HASH_SHA256)
        sha256((void**)&file_data, file_len, (void**)&hash);
    else
        sha256((void**)&file_data, file_len, (void**)&hash);

    char *enc_sig = NULL;
    size_t enc_len = 0;
    
    if (encode_algo == RSAA) {
        rsa_encode((char *)hash, hash_len, (char *)private_key_file, &enc_sig, &enc_len, verbose);
        if (!enc_sig || enc_len == 0) {
            if (verbose) fprintf(stderr, "Ошибка RSA подписи\n");
            free(hash);
            free(file_data);
            return false;
        }
    }
    else if(encode_algo == EL_GAMAL) {
        el_gamal_sign((char *)hash, hash_len, (char *)private_key_file, &enc_sig, &enc_len, verbose);
        if (!enc_sig || enc_len == 0) {
            if (verbose) fprintf(stderr, "Ошибка ElGamal подписи\n");
            free(hash);
            free(file_data);
            return false;
        }
    }
   else if(encode_algo == FIAT_SHAMIR) {
        fs_sign((char *)hash, hash_len, (char *)private_key_file, 20, &enc_sig, &enc_len, verbose);
        if (!enc_sig || enc_len == 0) {
            if (verbose) fprintf(stderr, "Ошибка Fiat-Shamir подписи\n");
            free(hash);
            free(file_data);
            return false;
        }
    }
    else {
        fprintf(stderr, "Неподдерживаемый алгоритм шифрования\n");
        free(hash);
        free(file_data);
        return false;
    }

    char timestamp[20];
    get_utc_timestamp(timestamp, sizeof(timestamp));

    // Заполняем структуру для отправки в TSA
    sig->signature = (uint8_t*)enc_sig;
    sig->signature_len = enc_len;
    memcpy(sig->timestamp, timestamp, sizeof(timestamp));
    sig->hash_algo = hash_algo;
    sig->encode_algo = encode_algo;
    strncpy(sig->signer_name, signer_name, sizeof(sig->signer_name)-1);
    sig->signer_name[sizeof(sig->signer_name)-1] = '\0';

    // Получаем подпись TSA для всей структуры
    if (!request_tsa_signature(sig, &sig->ts_signature, &sig->ts_signature_len)) {
        if (verbose) fprintf(stderr, "Ошибка получения подписи TSA\n");
        free(enc_sig);
        free(hash);
        free(file_data);
        return false;
    }

    free(hash);
    free(file_data);
    return true;
}
bool cades_verify_file(
    const char *filename,
    const char *public_key_file,
    const char *private_key_file,
    const CAdESSignature *sig,
    int verbose
) {
    // Сначала проверяем подпись TSA
    if (!verify_tsa_signature(sig)) {
        if (verbose) fprintf(stderr, "Недействительная подпись TSA\n");
        return false;
    }

    // Затем проверяем подпись пользователя
    uint8_t *file_data;
    size_t file_len;

    if (!read_file(filename, &file_data, &file_len)) {
        if (verbose) fprintf(stderr, "Ошибка чтения файла %s\n", filename);
        return false;
    }

    size_t hash_len = (sig->hash_algo == HASH_SHA256) ? 32 : 64;
    uint8_t* hash = (uint8_t*)malloc(hash_len);
    if (!hash) {
        free(file_data);
        return false;
    }

    if (sig->hash_algo == HASH_SHA256)
        sha256((void**)&file_data, file_len, (void**)&hash);
    else
        sha256((void**)&file_data, file_len, (void**)&hash);

    bool valid = false;
    
    if (sig->encode_algo == RSAA) {
        char *decrypted_hash = NULL;
        size_t decrypted_len = 0;
        rsa_decode((char *)sig->signature, sig->signature_len, (char *)public_key_file, &decrypted_hash, &decrypted_len, verbose);
        valid = decrypted_hash && (memcmp(decrypted_hash, hash, hash_len) == 0);
        free(decrypted_hash);
    }   
    // ... остальные алгоритмы

    free(hash);
    free(file_data);
    return valid;
}

bool cades_save_signature(const char *filename, const CAdESSignature *sig) {
    char dir_path[256] = {0};
    strncpy(dir_path, filename, sizeof(dir_path)-1);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (!ensure_directory_exists(dir_path)) {
            return false;
        }
    }

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("fopen");
        return false;
    }

    bool success = true;
    success &= fwrite(&sig->hash_algo, sizeof(HashAlgorithm), 1, f) == 1;
    success &= fwrite(&sig->encode_algo, sizeof(EncodeAlgorithm), 1, f) == 1;
    success &= fwrite(&sig->signature_len, sizeof(size_t), 1, f) == 1;
    success &= fwrite(sig->signature, 1, sig->signature_len, f) == sig->signature_len;
    success &= fwrite(sig->timestamp, 1, 20, f) == 20;
    success &= fwrite(sig->signer_name, 1, sizeof(sig->signer_name), f) == sizeof(sig->signer_name);
    success &= fwrite(&sig->ts_signature_len, sizeof(size_t), 1, f) == 1;
    if (sig->ts_signature_len > 0) {
        success &= fwrite(sig->ts_signature, 1, sig->ts_signature_len, f) == sig->ts_signature_len;
    }

    if (!success) {
        perror("fwrite");
    }

    fclose(f);
    return success;
}

bool cades_load_signature(const char *filename, CAdESSignature *sig) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return false;
    }

    memset(sig, 0, sizeof(CAdESSignature));
    
    bool success = true;
    success &= fread(&sig->hash_algo, sizeof(HashAlgorithm), 1, f) == 1;
    success &= fread(&sig->encode_algo, sizeof(EncodeAlgorithm), 1, f) == 1;
    success &= fread(&sig->signature_len, sizeof(size_t), 1, f) == 1;
    
    if (success) {
        sig->signature = malloc(sig->signature_len);
        success &= sig->signature != NULL;
        success &= fread(sig->signature, 1, sig->signature_len, f) == sig->signature_len;
    }
    
    success &= fread(sig->timestamp, 1, 20, f) == 20;
    success &= fread(sig->signer_name, 1, sizeof(sig->signer_name), f) == sizeof(sig->signer_name);
    success &= fread(&sig->ts_signature_len, sizeof(size_t), 1, f) == 1;
    
    if (success && sig->ts_signature_len > 0) {
        sig->ts_signature = malloc(sig->ts_signature_len);
        success &= sig->ts_signature != NULL;
        success &= fread(sig->ts_signature, 1, sig->ts_signature_len, f) == sig->ts_signature_len;
    }

    if (!success) {
        perror("fread");
        cades_free_signature(sig);
    }

    fclose(f);
    return success;
}

void cades_free_signature(CAdESSignature *sig) {
    if (sig->signature) free(sig->signature);
    if (sig->ts_signature) free(sig->ts_signature);
}

#ifndef LIB

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [sign|verify|view]\n", argv[0]);
        return 1;
    }

    const char *filename = "env.sh";
    const char *private_key = "hello";
    const char *public_key = "hello.pub";
    const char *signature_file = "signatures/signature.bin";

    if (strcmp(argv[1], "sign") == 0) {
        CAdESSignature sig = {0};
        if (!RSA_sign(filename, private_key, public_key, HASH_SHA256, "Ivan Ivanov", &sig, 1)) {
            printf("Signing failed!\n");
            return 1;
        }

        if (!cades_save_signature(signature_file, &sig)) {
            printf("Failed to save signature!\n");
            cades_free_signature(&sig);
            return 1;
        }

        printf("File signed successfully.\n");
        cades_view_signature(&sig);
        cades_free_signature(&sig);

    } else if (strcmp(argv[1], "verify") == 0) {
        CAdESSignature loaded_sig;
        if (!cades_load_signature(signature_file, &loaded_sig)) {
            printf("Failed to load signature\n");
            return 1;
        }

        bool valid = RSA_verify(filename, public_key, &loaded_sig, 1);
        printf("Verification result: %s\n", valid ? "Valid signature" : "Invalid signature");
        cades_view_signature(&loaded_sig);
        cades_free_signature(&loaded_sig);

    } else if (strcmp(argv[1], "view") == 0) {
        CAdESSignature loaded_sig;
        if (!cades_load_signature(signature_file, &loaded_sig)) {
            printf("Failed to load signature\n");
            return 1;
        }
        cades_view_signature(&loaded_sig);
        cades_free_signature(&loaded_sig);

    } else {
        printf("Unknown mode: %s\n", argv[1]);
        return 1;
    }

    return 0;
}

#endif // LIB

// Новые высокоуровневые функции RSA
bool RSA_sign(const char *filename, const char *private_key_file, 
              const char *public_key_file, HashAlgorithm hash_algo,
              const char *signer_name, CAdESSignature *sig, int verbose) {
    return cades_sign_file(filename, private_key_file, public_key_file, 
                         hash_algo, RSAA, signer_name, sig, verbose);
}

bool RSA_verify(const char *filename, const char *public_key_file,
                const CAdESSignature *sig, int verbose) {
    return cades_verify_file(filename, public_key_file, NULL, sig, verbose);
}
