#include "common.h"
#include <time.h>

#define ID_B "Client_B"

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    uint8_t key[KEY_SIZE] = "0123456789abcdef";

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

    // Читаем зашифрованное сообщение
    uint8_t encrypted[1024];
    ssize_t received = read(new_socket, encrypted, 1024);
    if (received <= 0) handle_error("read() failed");

    // Расшифровываем
    uint8_t decrypted[1024];
    int decrypted_len = aes_decrypt(encrypted, received, key, decrypted);
    decrypted[decrypted_len] = '\0';

    printf("[CLIENT_B] Получено сообщение: %s\n", decrypted);

    // Парсим: IDA || R_A || M1
    char ida[32], ra[32], m1[256];
    // sscanf(decrypted, "%31[^ ]%31[^ ]%255s", ida, ra, m1);
    // sscanf(decrypted, "%s|%s|%s", ida, ra, m1);
    sscanf(decrypted, "%255[^|]|%255[^|]|%255[^|]", ida, ra, m1);
    printf("[CLIENT_B] IDA: %s | R_A: %s | M1: %s\n", ida, ra, m1);

    // Генерируем ответ: IDB || M2(R_A) || M3
    char m2[256], m3[256];
    strcat(m2, ra); // M2 зависит от R_A
    strcpy(m3, "Auth_OK"); // M3 — фиксированное значение

    char response_msg[1024];
    snprintf(response_msg, sizeof(response_msg), "%s|%s|%s", ID_B, m2, m3);
    int msg_len = strlen(response_msg);

    // Шифруем и отправляем
    uint8_t response_enc[1024];
    int response_enc_len;
    aes_encrypt((uint8_t*)response_msg, msg_len, key, response_enc, &response_enc_len);
    send(new_socket, response_enc, response_enc_len, 0);

    close(new_socket);
    close(server_fd);
    return 0;
}
