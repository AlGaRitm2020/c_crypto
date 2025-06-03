#include "common.h"
#include "../sha.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

#define ID "Client_A"
#define INITIAL_ITER 2 
#define SECRET_PASSWORD "my_secure_password"
#define SHA256_DIGEST_LENGTH 32

// Печать хэша в hex-формате
void print_hash(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

// Функция исправления endianness
void fix_endian(uint8_t *hash, size_t len) {
    for (size_t i = 0; i < len; i += 4) {
        uint32_t *word = (uint32_t *)(hash + i);
        uint32_t temp = *word;

        *word =
            ((temp >> 24) & 0x000000FF) |
            ((temp >> 8)  & 0x0000FF00) |
            ((temp << 8)  & 0x00FF0000) |
            ((temp << 24) & 0xFF000000);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int current_iter = INITIAL_ITER;
    uint8_t *stored_hash = NULL;

    // Инициализация: h^n(password)
    char* secret = strdup(SECRET_PASSWORD);
    printf("secret: %s\n", secret);

    // Выделяем память под первый хэш
    uint8_t *hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);

    // Копируем secret в динамически выделенную память
    char *data_copy = (char*)malloc(strlen(secret) + 1);
    strcpy(data_copy, secret);

    sha256((void**)&data_copy, strlen(secret), (void**)&hash);
    fix_endian(hash, SHA256_DIGEST_LENGTH); // ← фиксируем endianness
    printf("h^1: ");
    print_hash(hash, SHA256_DIGEST_LENGTH);

    free(data_copy);

    for(int i = 1; i < INITIAL_ITER; i++) {
        uint8_t *new_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);

        uint8_t *current_hash_copy = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
        memcpy(current_hash_copy, hash, SHA256_DIGEST_LENGTH);
        fix_endian(current_hash_copy, SHA256_DIGEST_LENGTH); // ← фиксируем перед хэшированием

        sha256((void**)&current_hash_copy, SHA256_DIGEST_LENGTH, (void**)&new_hash);
        fix_endian(new_hash, SHA256_DIGEST_LENGTH); // ← фиксируем после хэширования

        printf("h^%d: ", i + 1);
        print_hash(new_hash, SHA256_DIGEST_LENGTH);

        free(hash);
        free(current_hash_copy);
        hash = new_hash;
    }

    stored_hash = hash;

    printf("Серверный начальный хэш (h^%d):\n", INITIAL_ITER);
    print_hash(stored_hash, SHA256_DIGEST_LENGTH);

    // Серверный сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        handle_error("Socket failed");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT_B);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        handle_error("Bind failed");

    if (listen(server_fd, 3) < 0)
        handle_error("Listen");

    printf("[СЕРВЕР] Ожидание подключений...\n");

    while(1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        char buffer[1024];
        ssize_t recv_len = recv(new_socket, buffer, sizeof(buffer), 0);

        if (recv_len <= 0) {
            close(new_socket);
            continue;
        }

        // Парсинг: ID|iter|hash
        char *id_end = memchr(buffer, '|', recv_len);
        if (!id_end) {
            send(new_socket, "Invalid format", 14, 0);
            close(new_socket);
            continue;
        }

        char *iter_end = memchr(id_end + 1, '|', recv_len - (id_end + 1 - buffer));
        if (!iter_end) {
            send(new_socket, "Invalid format", 14, 0);
            close(new_socket);
            continue;
        }

        // Получаем ID
        char received_id[256];
        size_t id_len = id_end - buffer;
        memcpy(received_id, buffer, id_len);
        received_id[id_len] = '\0';

        // Получаем итерацию
        char iter_str[16];
        size_t iter_len = iter_end - (id_end + 1);
        memcpy(iter_str, id_end + 1, iter_len);
        iter_str[iter_len] = '\0';
        int received_iter = atoi(iter_str);

        // Получаем хэш
        uint8_t *received_hash = (uint8_t*)(iter_end + 1);
        fix_endian(received_hash, SHA256_DIGEST_LENGTH); // ← фиксируем endianness

        printf("Получено от клиента:\nID: %s\nИтерация: %d\nХэш:\n", received_id, received_iter);
        print_hash(received_hash, SHA256_DIGEST_LENGTH);

        // Проверка ID
        if (strcmp(received_id, ID) != 0) {
            send(new_socket, "Invalid ID", 10, 0);
            close(new_socket);
            continue;
        }

        // Проверка итерации
        if (received_iter != current_iter - 1) {
            send(new_socket, "Invalid iteration", 17, 0);
            close(new_socket);
            continue;
        }

        // Вычисляем sha256 от полученного хэша
        uint8_t *computed_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);

        uint8_t *hash_copy = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
        memcpy(hash_copy, received_hash, SHA256_DIGEST_LENGTH);

        sha256((void**)&hash_copy, SHA256_DIGEST_LENGTH, (void**)&computed_hash);
        fix_endian(computed_hash, SHA256_DIGEST_LENGTH); // ← фиксируем endianness

        printf("Вычисленный хэш от полученного значения:\n");
        print_hash(computed_hash, SHA256_DIGEST_LENGTH);

        printf("Сохранённый сервером хэш (ожидался):\n");
        print_hash(stored_hash, SHA256_DIGEST_LENGTH);

        free(hash_copy);

        if (memcmp(computed_hash, stored_hash, SHA256_DIGEST_LENGTH) != 0) {
            send(new_socket, "Authentication failed", 21, 0);
            free(computed_hash);
            close(new_socket);
            continue;
        }

        // Успешная аутентификация
        free(stored_hash);
        stored_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
        memcpy(stored_hash, received_hash, SHA256_DIGEST_LENGTH);
        current_iter--;

        send(new_socket, "Authentication successful", 25, 0);
        printf("Успешная аутентификация, следующая итерация: %d\n", current_iter);

        free(computed_hash);
        close(new_socket);
    }

    free(stored_hash);
    close(server_fd);
    return 0;
}
