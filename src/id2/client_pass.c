#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

#define ID "Client_A"
#define SHA256_DIGEST_LENGTH 32

// Печать хэша в hex-формате
void print_hash(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

// Функция для исправления endianness (каждые 4 байта)
void fix_endian(uint8_t *hash, size_t len) {
    for (size_t i = 0; i < len; i += 4) {
        uint32_t *word = (uint32_t *)(hash + i);
        uint32_t temp = *word;

        // Переворачиваем байты внутри слова
        *word =
            ((temp >> 24) & 0x000000FF) |
            ((temp >> 8)  & 0x0000FF00) |
            ((temp << 8)  & 0x00FF0000) |
            ((temp << 24) & 0xFF000000);
    }
}

extern void sha256(void **data, uint64_t len, void **out_hash);
extern void handle_error(const char *);

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char* password = (char*)malloc(256);
    int iteration;

    printf("Введите секретный пароль: ");
    scanf("%255s", password);
    printf("Введите номер итерации: ");
    scanf("%d", &iteration);

    // Вычисляем h^iteration(password)
    uint8_t *current_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
    sha256((void**)&password, strlen(password), (void**)&current_hash);
    fix_endian(current_hash, SHA256_DIGEST_LENGTH); // ← фиксируем порядок байт
    printf("h^1: ");
    print_hash(current_hash, SHA256_DIGEST_LENGTH);

    for(int i = 1; i < iteration; i++) {
        uint8_t *new_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);

        uint8_t *current_hash_copy = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
        memcpy(current_hash_copy, current_hash, SHA256_DIGEST_LENGTH);
        fix_endian(current_hash_copy, SHA256_DIGEST_LENGTH); // ← фиксируем перед следующим хэшированием

        sha256((void**)&current_hash_copy, SHA256_DIGEST_LENGTH, (void**)&new_hash);
        fix_endian(new_hash, SHA256_DIGEST_LENGTH); // ← фиксируем результат хэширования

        printf("h^%d: ", i + 1);
        print_hash(new_hash, SHA256_DIGEST_LENGTH);

        free(current_hash);
        free(current_hash_copy);
        current_hash = new_hash;
    }

    printf("Отправляемый хэш (h^%d):\n", iteration);
    print_hash(current_hash, SHA256_DIGEST_LENGTH);

    // Подключение к серверу
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("Socket creation error");

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_B);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        handle_error("Invalid address");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        handle_error("Connection Failed");

    // Формируем сообщение: ID|iteration|hash
    char message[1024];
    int offset = snprintf(message, sizeof(message), "%s|%d|", ID, iteration);
    memcpy(message + offset, current_hash, SHA256_DIGEST_LENGTH);
    size_t msg_len = offset + SHA256_DIGEST_LENGTH;

    send(sock, message, msg_len, 0);
    printf("Отправлен пароль для итерации %d\n", iteration);

    // Получаем ответ
    char response[256];
    ssize_t resp_len = recv(sock, response, sizeof(response), 0);
    if (resp_len <= 0) handle_error("Ошибка при получении ответа");
    response[resp_len] = '\0';
    printf("Ответ сервера: %s\n", response);

    free(current_hash);
    close(sock);
    return 0;
}
