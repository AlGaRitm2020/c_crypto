#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <openssl/rand.h>

#include "../sign.h"
#include "../rsa.h"
#include "common.h"

#define KEY_SIZE 16
#define TEMP_DATA_FILE "temp_data.bin"
#define SIGNATURE_FILE "sent.sig"

void generate_session_key(uint8_t key[KEY_SIZE]) {
    if (!RAND_bytes(key, KEY_SIZE)) {
        perror("Failed to generate session key");
        exit(EXIT_FAILURE);
    }
    printf("[A] Generated session key (%d bytes):\n", KEY_SIZE);
    for (int i = 0; i < KEY_SIZE; i++) {
        printf("%02x ", key[i]);
    }
    printf("\n");
}

int main() {
    // 1. Генерация сеансового ключа
    uint8_t session_key[KEY_SIZE];
    generate_session_key(session_key);

    // 2. Получаем текущее время
    uint64_t timestamp = (uint64_t)time(NULL);
    printf("[A] Timestamp: %lu\n", timestamp);

    // 3. Записываем ключ + время во временный файл
    FILE* f = fopen(TEMP_DATA_FILE, "wb");
    if (!f) {
        perror("Failed to create temp data file");
        exit(EXIT_FAILURE);
    }

    fwrite(session_key, 1, KEY_SIZE, f);
    fwrite(&timestamp, 1, sizeof(timestamp), f);
    fclose(f);

    // 4. Подписываем файл
    CAdESSignature sig = {0};
    if (!RSA_sign(TEMP_DATA_FILE, "keys/a_private", "keys/a_public", HASH_SHA256, "Client A", &sig, 1)) {
        fprintf(stderr, "[A] Failed to sign data\n");
        unlink(TEMP_DATA_FILE);
        exit(EXIT_FAILURE);
    }

    // 5. Сохраняем подпись в файл sent.sig
    if (!cades_save_signature(SIGNATURE_FILE, &sig)) {
        fprintf(stderr, "[A] Failed to save signature\n");
        cades_free_signature(&sig);
        unlink(TEMP_DATA_FILE);
        exit(EXIT_FAILURE);
    }

    printf("[A] Signature saved to %s\n", SIGNATURE_FILE);

    // 6. Читаем содержимое файла данных для шифрования
    uint8_t* data;
    size_t data_len;
    if (!read_file(TEMP_DATA_FILE, &data, &data_len)) {
        fprintf(stderr, "[A] Failed to read back signed data\n");
        cades_free_signature(&sig);
        unlink(TEMP_DATA_FILE);
        exit(EXIT_FAILURE);
    }

    unlink(TEMP_DATA_FILE); // больше не нужен

    // 7. Шифруем данные открытым ключом B
    char* encrypted_msg = NULL;
    size_t encrypted_len = 0;

    if (!rsa_encode((char*)data, data_len, "keys/b_public", &encrypted_msg, &encrypted_len, 1)) {
        fprintf(stderr, "[A] Encryption failed\n");
        free(data);
        exit(EXIT_FAILURE);
    }

    printf("[A] Encrypted data length: %zu bytes\n", encrypted_len);

    // 8. Подключаемся к серверу B
    int sock = connect_to("127.0.0.1", PORT_B);
    if (sock < 0) {
        free(data);
        free(encrypted_msg);
        exit(EXIT_FAILURE);
    }

    // Сначала отправляем размер зашифрованных данных
    if (write(sock, &encrypted_len, sizeof(size_t)) != sizeof(size_t)) {
        perror("[A] Failed to send data size");
        close(sock);
        free(data);
        free(encrypted_msg);
        exit(EXIT_FAILURE);
    }

    // Затем отправляем сами зашифрованные данные
    if (write(sock, encrypted_msg, encrypted_len) != (ssize_t)encrypted_len) {
        perror("[A] Failed to send encrypted data");
        close(sock);
        free(data);
        free(encrypted_msg);
        exit(EXIT_FAILURE);
    }

    printf("[A] Data sent successfully\n");

    // Очистка
    free(data);
    free(encrypted_msg);
    close(sock);

    return 0;
}
