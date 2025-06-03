#include "common.h"
#include "../rsa.h"
#include "../sha.h"

#define ID_B "Client_B"
#define SHA256_DIGEST_LENGTH 32

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        handle_error("Socket failed");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT_B);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        handle_error("Bind failed");

    if (listen(server_fd, 3) < 0)
        handle_error("Listen");

    printf("[CLIENT_B] Сервер запущен, ожидание подключения...\n");
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    // Шаг 1: Получаем h(z) | A | E_k(z, A)
    uint8_t buffer[2048];
    ssize_t received = recv(new_socket, buffer, sizeof(buffer), 0);
    if (received <= 0) handle_error("read() failed");

    // Парсим h(z) (первые 32 байта)
    uint8_t hz_received[SHA256_DIGEST_LENGTH];
    memcpy(hz_received, buffer, SHA256_DIGEST_LENGTH);

    // Парсим IDA (до следующего '|')
    char* ptr = (char*)buffer + SHA256_DIGEST_LENGTH;
    if (*ptr != '|') handle_error("Invalid message format");
    ptr++;
    
    char ida[32];
    char* ida_end = strchr(ptr, '|');
    if (!ida_end) handle_error("Invalid message format");
    size_t ida_len = ida_end - ptr;
    memcpy(ida, ptr, ida_len);
    ida[ida_len] = '\0';
    printf("[CLIENT_B] Получен IDA: %s\n", ida);

    // Получаем зашифрованную часть
    char* enc_data = ida_end + 1;
    size_t enc_data_len = received - (enc_data - (char*)buffer);

    // Расшифровываем
    char* dec_data = (char*)malloc(1024);
    size_t dec_data_len = 0;
    rsa_decode(enc_data, enc_data_len, "priv_key_B.pem", &dec_data, &dec_data_len, 1);

    // Парсим расшифрованные данные: IDA|z (бинарные)
    printf("dec data: %s\n", dec_data);
    char* z_pos = (char*)memchr(dec_data, '|', dec_data_len);
    if (!z_pos) handle_error("Invalid decrypted data format");
    
    // Проверяем IDA
    size_t dec_ida_len = z_pos - dec_data;
    if (dec_ida_len != ida_len || memcmp(dec_data, ida, dec_ida_len) != 0) {
        handle_error("IDA mismatch");
    }

    // Получаем z (бинарные данные);;
    // uint8_t z[32];
    uint8_t* z = (uint8_t*)malloc(32);
    // if (dec_data_len - (z_pos - dec_data) - 1 != sizeof(z))
    //     handle_error("Invalid z length");
    memcpy(z, z_pos + 1, 32);
    print_hex_arr(z, 32, "получен z");

    // Проверяем h(z)
    uint8_t* hz_calculated = (uint8_t*)malloc(32);
    sha256((void**)&z, 32, (void**)&hz_calculated);

    if (memcmp(hz_received, hz_calculated, SHA256_DIGEST_LENGTH) != 0) {
        handle_error("h(z) verification failed");
    }

    // Шаг 2: Отправляем z обратно
    send(new_socket, z, 32, 0);
    printf("[CLIENT_B] Отправлен z клиенту\n");

    free(dec_data);
    free(hz_calculated);
    close(new_socket);
    close(server_fd);
    return 0;
}
