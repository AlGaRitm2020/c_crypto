#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

bool request_tsa_signature(const uint8_t *hash, size_t hash_len, const char *timestamp,
                          uint8_t **ts_signature, size_t *ts_len) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    printf("Подключение к TSA серверу...\n");
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return false;
    }

    if (write(sock, &hash_len, sizeof(size_t)) != sizeof(size_t)) {
        perror("write hash_len");
        close(sock);
        return false;
    }

    if (write(sock, hash, hash_len) != hash_len) {
        perror("write hash");
        close(sock);
        return false;
    }

    size_t timestamp_len = strlen(timestamp) + 1;
    if (write(sock, &timestamp_len, sizeof(size_t)) != sizeof(size_t)) {
        perror("write timestamp_len");
        close(sock);
        return false;
    }

    if (write(sock, timestamp, timestamp_len) != timestamp_len) {
        perror("write timestamp");
        close(sock);
        return false;
    }

    printf("Данные отправлены, ожидание ответа...\n");

    if (read(sock, ts_len, sizeof(size_t)) != sizeof(size_t)) {
        perror("read signature length");
        close(sock);
        return false;
    }

    // Получение подписи
    *ts_signature = malloc(*ts_len);
    if (!*ts_signature) {
        perror("malloc signature");
        close(sock);
        return false;
    }

    if (read(sock, *ts_signature, *ts_len) != *ts_len) {
        perror("read signature");
        free(*ts_signature);
        close(sock);
        return false;
    }

    printf("Подпись получена (%zu байт)\n", *ts_len);

    close(sock);
    return true;
}

bool verify_tsa_signature(const uint8_t *hash, size_t hash_len, const char *timestamp,
                         const uint8_t *ts_sig, size_t ts_sig_len) {
    
    return true;
}
