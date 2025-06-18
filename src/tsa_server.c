#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>
#include "rsa.h"
#include "sign.h"

bool is_timestamp_valid(const char* timestamp) {
    time_t now = time(NULL);
    struct tm tm_time = {0};
    
    char* end = strptime(timestamp, "%Y%m%d%H%M%S", &tm_time);
    if (end == NULL || *end != '\0') {
        return false;
    }
    
    time_t ts_time = timegm(&tm_time);
    double diff_seconds = difftime(now, ts_time);
    
    return (diff_seconds >= 0 && diff_seconds <= 120);
}

void handle_sign_request(int sock) {
    CAdESSignature sig = {0};

    // Читаем данные подписи
    if (read(sock, &sig.signature_len, sizeof(size_t)) != sizeof(size_t) ||
        !(sig.signature = malloc(sig.signature_len)) ||
        read(sock, sig.signature, sig.signature_len) != sig.signature_len ||
        read(sock, sig.timestamp, sizeof(sig.timestamp)) != sizeof(sig.timestamp) ||
        read(sock, &sig.hash_algo, sizeof(HashAlgorithm)) != sizeof(HashAlgorithm) ||
        read(sock, &sig.encode_algo, sizeof(EncodeAlgorithm)) != sizeof(EncodeAlgorithm) ||
        read(sock, sig.signer_name, sizeof(sig.signer_name)) != sizeof(sig.signer_name)) {
        perror("read signature data");
        free(sig.signature);
        return;
    }

    // Проверяем временную метку
    if (!is_timestamp_valid(sig.timestamp)) {
        printf("Invalid timestamp\n");
        free(sig.signature);
        return;
    }

    // Создаем данные для подписи TSA (вся структура кроме ts_signature)
    size_t tbs_len = sizeof(sig) - sizeof(sig.ts_signature) - sizeof(sig.ts_signature_len);
    uint8_t tbs[tbs_len];
    memcpy(tbs, &sig, tbs_len);

    // Подписываем данные
    char *ts_sig = NULL;
    size_t ts_sig_len = 0;
    rsa_encode((char*)tbs, tbs_len, "keys/ts", &ts_sig, &ts_sig_len, 1);

    // Отправляем подпись
    if (write(sock, &ts_sig_len, sizeof(size_t)) != sizeof(size_t) ||
        write(sock, ts_sig, ts_sig_len) != ts_sig_len) {
        perror("write signature");
    }

    free(sig.signature);
    free(ts_sig);
}

void handle_verify_request(int sock) {
    CAdESSignature sig = {0};

    // Читаем данные для верификации
    if (read(sock, &sig.signature_len, sizeof(size_t)) != sizeof(size_t) ||
        !(sig.signature = malloc(sig.signature_len)) ||
        read(sock, sig.signature, sig.signature_len) != sig.signature_len ||
        read(sock, sig.timestamp, sizeof(sig.timestamp)) != sizeof(sig.timestamp) ||
        read(sock, &sig.hash_algo, sizeof(HashAlgorithm)) != sizeof(HashAlgorithm) ||
        read(sock, &sig.encode_algo, sizeof(EncodeAlgorithm)) != sizeof(EncodeAlgorithm) ||
        read(sock, sig.signer_name, sizeof(sig.signer_name)) != sizeof(sig.signer_name) ||
        read(sock, &sig.ts_signature_len, sizeof(size_t)) != sizeof(size_t) ||
        !(sig.ts_signature = malloc(sig.ts_signature_len)) ||
        read(sock, sig.ts_signature, sig.ts_signature_len) != sig.ts_signature_len) {
        perror("read verify data");
        free(sig.signature);
        free(sig.ts_signature);
        return;
    }

    // Проверяем временную метку
    bool timestamp_valid = is_timestamp_valid(sig.timestamp);

    // Проверяем подпись TSA
    size_t tbs_len = sizeof(sig) - sizeof(sig.ts_signature) - sizeof(sig.ts_signature_len);
    uint8_t tbs[tbs_len];
    memcpy(tbs, &sig, tbs_len);

    char *decrypted = NULL;
    size_t decrypted_len = 0;
    rsa_decode((char*)sig.ts_signature, sig.ts_signature_len, "keys/ts", &decrypted, &decrypted_len, 1);

    bool signature_valid = (decrypted_len == tbs_len) && 
                         (memcmp(decrypted, tbs, tbs_len) == 0);

    // Отправляем результат
    uint8_t result = timestamp_valid && signature_valid;
    write(sock, &result, 1);

    free(sig.signature);
    free(sig.ts_signature);
    free(decrypted);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(8080)
    };

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TSA server running on port 8080\n");

    while (1) {
        int client_sock = accept(server_fd, NULL, NULL);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        uint8_t request_type;
        if (read(client_sock, &request_type, 1) != 1) {
            perror("read request type");
            close(client_sock);
            continue;
        }

        if (request_type == 0) {
            handle_sign_request(client_sock);
        } else if (request_type == 1) {
            handle_verify_request(client_sock);
        }

        close(client_sock);
    }

    close(server_fd);
    return 0;
}
