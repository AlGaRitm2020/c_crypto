#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "../sign.h"
#include "../rsa.h"
#include "common.h"

#define MAX_BUFFER_SIZE 4096
#define KEY_SIZE 16
#define SIGNATURE_FILE "sent.sig"

void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s (%zu bytes):\n", label, len);
    for(size_t i = 0; i < len; i++) {
        printf("%02x ", data[i]);
        if((i+1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main() {
    int server_fd = create_server_socket(PORT_B);
    printf("[B] Server socket created, waiting for connection...\n");

    int client_sock;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if ((client_sock = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("[B] Client connected\n");

    // 1. Получаем размер зашифрованных данных
    size_t encrypted_len = 0;
    ssize_t received = recv(client_sock, &encrypted_len, sizeof(size_t), MSG_WAITALL);

    if(received != sizeof(size_t)) {
        perror("failed to receive data size");
        close(client_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("[B] Received encrypted data size: %zu bytes\n", encrypted_len);

    // 2. Получаем сами зашифрованные данные
    uint8_t* encrypted_msg = malloc(encrypted_len);
    if(!encrypted_msg) {
        perror("malloc failed for encrypted data");
        close(client_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    size_t total_received = 0;
    while(total_received < encrypted_len) {
        ssize_t n = recv(client_sock, encrypted_msg + total_received, encrypted_len - total_received, 0);
        if(n <= 0) {
            perror("receive failed");
            free(encrypted_msg);
            close(client_sock);
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        total_received += n;
    }
    printf("[B] Successfully received all encrypted data\n");

    // 3. Расшифровываем данные
    char* decrypted_msg = NULL;
    size_t decrypted_len = 0;

    printf("[B] Starting decryption...\n");
    int decode_result = rsa_decode((char*)encrypted_msg, encrypted_len, "keys/b_private", &decrypted_msg, &decrypted_len, 1);
    if(!decode_result) {
        fprintf(stderr, "[B] Decryption failed\n");
        free(encrypted_msg);
        close(client_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("[B] Decryption successful, got %zu bytes\n", decrypted_len);
    print_hex("Decrypted data", (uint8_t*)decrypted_msg, decrypted_len);

    // 4. Извлекаем сеансовый ключ
    uint8_t session_key[KEY_SIZE];
    memcpy(session_key, decrypted_msg, KEY_SIZE);
    print_hex("Extracted session key", session_key, KEY_SIZE);

    // 5. Извлекаем временную метку
    uint64_t timestamp;
    memcpy(&timestamp, decrypted_msg + KEY_SIZE, sizeof(uint64_t));
    printf("[B] Extracted timestamp: %lu\n", timestamp);

    // 6. Читаем подпись из файла sent.sig
    CAdESSignature loaded_sig;
    memset(&loaded_sig, 0, sizeof(CAdESSignature));

    if (!cades_load_signature(SIGNATURE_FILE, &loaded_sig)) {
        fprintf(stderr, "[B] Failed to load signature from %s\n", SIGNATURE_FILE);
        free(encrypted_msg);
        free(decrypted_msg);
        close(client_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 7. Проверяем подпись
    printf("[B] Verifying signature...\n");
    if (!RSA_verify(TEMP_DATA_FILE, "keys/a_public", &loaded_sig, 1)) {
        fprintf(stderr, "[B] Signature verification FAILED\n");
        cades_free_signature(&loaded_sig);
        free(encrypted_msg);
        free(decrypted_msg);
        close(client_sock);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[B] Signature verified successfully\n");

    // 8. Очистка
    cades_free_signature(&loaded_sig);
    free(encrypted_msg);
    free(decrypted_msg);
    close(client_sock);
    close(server_fd);

    printf("[B] Done\n");
    return 0;
}
