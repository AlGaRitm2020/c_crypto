#include "common.h"
#include "../rsa.h"
#include <stdint.h>
#include <openssl/rand.h>
#include "../sha.h"

#define ID_A "Client_A"
#define ID_B "Client_B"
#define SHA256_DIGEST_LENGTH 32

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Генерируем случайное число z
    // uint8_t z[32];
    uint8_t* z = (uint8_t*)malloc(32);
    RAND_bytes(z, 32);
    print_hex_arr(z, 32, "генерируем z");
    
    // Вычисляем h(z)
    uint8_t *hz = (uint8_t*)malloc(SHA256_DIGEST_LENGTH);
    sha256((void**)&z, 32, (void**)&hz);

    // Установка соединения
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("Socket creation error");

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_B);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        handle_error("Invalid address");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        handle_error("Connection Failed");

    printf("[CLIENT_A] Подключение к серверу установлено.\n");

    // Шаг 1: Шифруем z и A с помощью открытого ключа B
    char* enc_data = (char*)malloc(1024);
    size_t enc_data_len = 0;
    
    // Формируем данные для шифрования: IDA + z (бинарные)
    size_t msg_to_enc_len = strlen(ID_A) + 1 + 32;
    char* msg_to_enc = (char*)malloc(msg_to_enc_len);
    memcpy(msg_to_enc, ID_A, strlen(ID_A));
    msg_to_enc[strlen(ID_A)] = '|';
    memcpy(msg_to_enc + strlen(ID_A) + 1, z, 32);

    printf("msg to encrypt: %s\n", msg_to_enc);
    
    rsa_encode(msg_to_enc, msg_to_enc_len, "pub_key_B.pem", &enc_data, &enc_data_len, 1);
    free(msg_to_enc);

    // Формируем сообщение: h(z) | A | E_k(z, A)
    size_t msg1_len = SHA256_DIGEST_LENGTH + 1 + strlen(ID_A) + 1 + enc_data_len;
    char* msg1 = (char*)malloc(msg1_len);
    size_t offset = 0;
    
    memcpy(msg1, hz, SHA256_DIGEST_LENGTH);
    offset += SHA256_DIGEST_LENGTH;
    msg1[offset++] = '|';
    memcpy(msg1 + offset, ID_A, strlen(ID_A));
    offset += strlen(ID_A);
    msg1[offset++] = '|';
    memcpy(msg1 + offset, enc_data, enc_data_len);
    offset += enc_data_len;

    // Отправляем сообщение
    send(sock, msg1, msg1_len, 0);
    printf("[CLIENT_A] Отправлен шаг 1\n");
    free(enc_data);
    free(msg1);
    free(hz);

    // Шаг 2: Получаем z от сервера
    uint8_t received_z[32];
    ssize_t received = recv(sock, received_z, sizeof(received_z), 0);
    if (received != sizeof(received_z)) handle_error("Invalid z received");

    // Проверяем полученное z
    if (memcmp(z, received_z, 32) != 0) {
        handle_error("z verification failed");
    }
    printf("[CLIENT_A] Аутентификация успешна!\n");

    close(sock);
    return 0;
}
