#include "common.h"
#include "../sha.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int current_iter = INITIAL_ITER;
    uint8_t *stored_hash = NULL;

    // Инициализация: h^n(password)
    char* secret = strdup(SECRET_PASSWORD);
    uint8_t *hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);

    sha256((void**)&secret, strlen(secret), (void**)&hash);
    for(int i = 1; i < INITIAL_ITER; i++) {
        uint8_t *new_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
        sha256((void**)&hash, SHA256_DIGEST_LENGTH, (void**)&new_hash);
        free(hash);
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

        // Вычисляем ожидаемый хэш
        uint8_t *expected_hash = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
        sha256((void**)&stored_hash, SHA256_DIGEST_LENGTH, (void**)&expected_hash);

        if (memcmp(received_hash, expected_hash, SHA256_DIGEST_LENGTH) != 0) {
            printf("Ошибка аутентификации. Ожидался хэш:\n");
            print_hash(expected_hash, SHA256_DIGEST_LENGTH);
            send(new_socket, "Authentication failed", 21, 0);
            free(expected_hash);
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

        free(expected_hash);
        close(new_socket);
    }

    free(stored_hash);
    close(server_fd);
    return 0;
}
