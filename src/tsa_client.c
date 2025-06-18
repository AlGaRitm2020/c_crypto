#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "sign.h"

bool request_tsa_signature(const CAdESSignature *sig, uint8_t **ts_signature, size_t *ts_len) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return false;
    }

    // Отправляем тип запроса (0 - подпись)
    uint8_t request_type = 0;
    if (write(sock, &request_type, 1) != 1) {
        perror("write request type");
        close(sock);
        return false;
    }

    // Формируем данные для подписи вручную (без использования проблемной структуры)
    if (write(sock, &sig->signature_len, sizeof(size_t)) != sizeof(size_t) ||
        write(sock, sig->signature, sig->signature_len) != sig->signature_len ||
        write(sock, sig->timestamp, sizeof(sig->timestamp)) != sizeof(sig->timestamp) ||
        write(sock, &sig->hash_algo, sizeof(HashAlgorithm)) != sizeof(HashAlgorithm) ||
        write(sock, &sig->encode_algo, sizeof(EncodeAlgorithm)) != sizeof(EncodeAlgorithm) ||
        write(sock, sig->signer_name, sizeof(sig->signer_name)) != sizeof(sig->signer_name)) {
        perror("write signature data");
        close(sock);
        return false;
    }

    // Получаем подпись TSA
    if (read(sock, ts_len, sizeof(size_t)) != sizeof(size_t)) {
        perror("read signature length");
        close(sock);
        return false;
    }

    *ts_signature = malloc(*ts_len);
    if (!*ts_signature) {
        perror("malloc");
        close(sock);
        return false;
    }

    if (read(sock, *ts_signature, *ts_len) != *ts_len) {
        perror("read signature");
        free(*ts_signature);
        *ts_signature = NULL;
        close(sock);
        return false;
    }

    close(sock);
    return true;
}
bool verify_tsa_signature(const CAdESSignature *sig) {
    // return 1;
    if (sig->ts_signature == NULL || sig->ts_signature_len == 0) {
        fprintf(stderr, "No TSA signature present\n");
        return false;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return false;
    }

    // Отправляем тип запроса (1 - верификация)
    uint8_t request_type = 1;
    if (write(sock, &request_type, 1) != 1) {
        perror("write request type");
        close(sock);
        return false;
    }

    // Отправляем данные для верификации по отдельности
    if (write(sock, &sig->signature_len, sizeof(size_t)) != sizeof(size_t) ||
        write(sock, sig->signature, sig->signature_len) != sig->signature_len ||
        write(sock, sig->timestamp, sizeof(sig->timestamp)) != sizeof(sig->timestamp) ||
        write(sock, &sig->hash_algo, sizeof(HashAlgorithm)) != sizeof(HashAlgorithm) ||
        write(sock, &sig->encode_algo, sizeof(EncodeAlgorithm)) != sizeof(EncodeAlgorithm) ||
        write(sock, sig->signer_name, sizeof(sig->signer_name)) != sizeof(sig->signer_name) ||
        write(sock, &sig->ts_signature_len, sizeof(size_t)) != sizeof(size_t) ||
        write(sock, sig->ts_signature, sig->ts_signature_len) != sig->ts_signature_len) {
        perror("write verify data");
        close(sock);
        return false;
    }

    // Получаем результат верификации
    uint8_t verification_result;
    if (read(sock, &verification_result, 1) != 1) {
        perror("read verification result");
        close(sock);
        return false;
    }

    close(sock);
    return verification_result == 1;
}
