#include "sign.h"
#include "el_gamal.h"
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
    if (sig->encode_algo == RSA)

      printf("RSSSAAA A:        \n");
    else
      printf("ELLL:        \n");
    printf("Автор:        %s\n", sig->signer_name);
    printf("Алгоритм хеша:     %s\n", sig->hash_algo == HASH_SHA256 ? "SHA-256" : "SHA-512");
    printf("Алгоритм шифрования:     %s\n", sig->encode_algo == RSA ? "RSA" : "El-Gamal");
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
    
    // Вызываем rsa_encode как void функцию
    if (encode_algo == RSA) {
      rsa_encode((char *)hash, hash_len, (char *)private_key_file, &enc_sig, &enc_len, verbose);
      if (!enc_sig || enc_len == 0) {
        if (verbose) fprintf(stderr, "Ошибка RSA подписи\n");
        free(hash);
        free(file_data);
        return false;
      }
  }
    else if(encode_algo == EL_GAMAL) {
      el_gamal_encode((char *)hash, hash_len, (char *)public_key_file, &enc_sig, &enc_len, verbose);
      if (!enc_sig || enc_len == 0) {
        if (verbose) fprintf(stderr, "Ошибка el_gamal подписи\n");
        free(hash);
        free(file_data);
        return false;
      }
  }
    else {
       fprintf(stderr, "Неподдерживаемый алгоритм шифрования\n");
       return false;
    }

    char timestamp[20];
    get_utc_timestamp(timestamp, sizeof(timestamp));

    uint8_t *ts_signature = NULL;
    size_t ts_signature_len = 0;
    if (!request_tsa_signature(hash, hash_len, timestamp, &ts_signature, &ts_signature_len)) {
        if (verbose) fprintf(stderr, "Ошибка получения временной метки\n");
        free(hash);
        free(file_data);
        free(enc_sig);
        return false;
    }

    sig->signature = (uint8_t*)enc_sig;
    sig->signature_len = enc_len;
    memcpy(sig->timestamp, timestamp, sizeof(timestamp));
    sig->hash_algo = hash_algo;
    
    sig->encode_algo= encode_algo;
    strncpy(sig->signer_name, signer_name, sizeof(sig->signer_name)-1);
    sig->signer_name[sizeof(sig->signer_name)-1] = '\0';
    sig->ts_signature = ts_signature;
    sig->ts_signature_len = ts_signature_len;
  
    printf("HASH ALGO: %s\n", (char*)hash_algo);
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
    uint8_t *file_data;
    size_t file_len;

    if (!read_file(filename, &file_data, &file_len)) {
        if (verbose) fprintf(stderr, "Ошибка чтения файла %s\n", filename);
        return false;
    }

    size_t hash_len = (sig->hash_algo == HASH_SHA256) ? 32 : 64;
    uint8_t* hash = malloc(hash_len);
    if (sig->hash_algo == HASH_SHA256)
        sha256((void**)&file_data, file_len, (void**)&hash);
    else
        sha256((void**)&file_data, file_len, (void**)&hash);

    char *decrypted_hash = NULL;
    size_t decrypted_len = 0;
    // rsa_decode((char *)sig->signature, sig->signature_len, (char *)public_key_file, &decrypted_hash, &decrypted_len, verbose);

    // printf("ENCR ALGO: %s \n",(char*)sig->encode_algo);
    if (sig->encode_algo == RSA) {
    rsa_decode((char *)sig->signature, sig->signature_len, (char *)public_key_file, &decrypted_hash, &decrypted_len, verbose);
  }
    else if(sig->encode_algo == EL_GAMAL) {
    el_gamal_decode((char *)sig->signature, sig->signature_len, (char *)public_key_file, &decrypted_hash, &decrypted_len, verbose);

  }




    if (!decrypted_hash) {
        free(hash);
        free(file_data);
        return false;
    }

    bool valid = memcmp(decrypted_hash, hash, hash_len) == 0;

    if (sig->ts_signature && sig->ts_signature_len > 0) {
        bool ts_valid = verify_tsa_signature(hash, hash_len, sig->timestamp, sig->ts_signature, sig->ts_signature_len);
        valid &= ts_valid;
    }

    free(hash);
    free(file_data);
    free(decrypted_hash);

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s [sign|verify|view]\n", argv[0]);
        return 1;
    }

    const char *filename = "env.sh";
    const EncodeAlgorithm encode_algo = EL_GAMAL;
    // const char *private_key = "hello";
    // const char *public_key = "hello.pub";
    //
    const char *private_key = "elgamal.pri";
    const char *public_key = "elgamal.pub";


    const char *signature_file = "signatures/signature.bin";

    if (strcmp(argv[1], "sign") == 0) {
        CAdESSignature sig = {0};
        if (!cades_sign_file(filename, private_key, HASH_SHA256, encode_algo, "Ivan Ivanov", &sig, 1)) {
            printf("Ошибка подписания!\n");
            return 1;
        }

        if (!cades_save_signature(signature_file, &sig)) {
            printf("Ошибка сохранения подписи!\n");
            cades_free_signature(&sig);
            return 1;
        }

        printf("Файл успешно подписан.\n");
        cades_view_signature(&sig);
        cades_free_signature(&sig);

    } else if (strcmp(argv[1], "verify") == 0) {
        CAdESSignature loaded_sig;
        if (!cades_load_signature(signature_file, &loaded_sig)) {
            printf("Не удалось загрузить подпись\n");
            return 1;
        }

        bool valid = cades_verify_file(filename, public_key, &loaded_sig, 1);
        printf("Результат проверки: %s\n", valid ? "Подпись верна" : "Подпись неверна");
        cades_view_signature(&loaded_sig);
        cades_free_signature(&loaded_sig);

    } else if (strcmp(argv[1], "view") == 0) {
        CAdESSignature loaded_sig;
        if (!cades_load_signature(signature_file, &loaded_sig)) {
            printf("Не удалось загрузить подпись\n");
            return 1;
        }
        cades_view_signature(&loaded_sig);
        cades_free_signature(&loaded_sig);

    } else {
        printf("Неизвестный режим: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
