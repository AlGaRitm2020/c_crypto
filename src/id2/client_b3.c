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

    // Шаг 1: Получаем r_A (16 байт) + M1 (текст)
    uint8_t r_a[16];
    char m1[256];
    
    recv(new_socket, r_a, 16, 0);
    int m1_len = recv(new_socket, m1, 256, 0);
    m1[m1_len] = '\0';
    
    printf("[CLIENT_B] Получен шаг 1: r_A (16 байт) + M1=%s\n", m1);

    // Шаг 2: Готовим M3 и зашифрованную часть
    char m3[256], m2[256];
    printf("Введите сообщение M2: ");
    scanf(" %[^\n]%*c", m2);
    printf("Введите сообщение M3: ");
    scanf(" %[^\n]%*c", m3);

    // Генерируем r_B
    uint8_t r_b[16];
    RAND_bytes(r_b, 16);

    // Формируем данные для шифрования: r_A (16), r_B (16), B, M2
    uint8_t to_encrypt[1024];
    memcpy(to_encrypt, r_a, 16);
    memcpy(to_encrypt + 16, r_b, 16);
    strcpy((char*)to_encrypt + 32, ID_B);
    strcpy((char*)to_encrypt + 32 + strlen(ID_B) + 1, m2);
    int to_encrypt_len = 32 + strlen(ID_B) + 1 + strlen(m2) + 1;

    uint8_t encrypted[1024];
    int ciphertext_len;
    aes_encrypt(to_encrypt, to_encrypt_len, key, encrypted, &ciphertext_len);

    // Формируем полное сообщение шага 2: M3 | encrypted_data
    send(new_socket, m3, strlen(m3) + 1, 0);
    send(new_socket, "|", 1, 0);
    send(new_socket, encrypted, ciphertext_len, 0);
    printf("[CLIENT_B] Отправлен шаг 2: M3=%s | encrypted data\n", m3);

    // Шаг 3: Получаем M5 (текст) + зашифрованные данные (бинарные)
    char m5[256];
    uint8_t encrypted_part[1024];
    
    // Получаем M5 (до символа '|')
    int bytes_received = 0;
    char c;
    while (recv(new_socket, &c, 1, 0) > 0) {
        if (c == '|') break;
        m5[bytes_received++] = c;
    }
    m5[bytes_received] = '\0';

    // Получаем зашифрованные данные
    int enc_size = recv(new_socket, encrypted_part, 1024, 0);
    if (enc_size <= 0) handle_error("Failed to receive encrypted part");

    // Расшифровываем
    uint8_t decrypted[1024];
    int decrypted_len = aes_decrypt(encrypted_part, enc_size, key, decrypted);
    decrypted[decrypted_len] = '\0';

    // Парсим расшифрованные данные: r_B (16), r_A (16), A, M4
    uint8_t rb_received[16], ra_received[16];
    char ida[32], m4[256];
    
    memcpy(rb_received, decrypted, 16);
    memcpy(ra_received, decrypted + 16, 16);
    strcpy(ida, (char*)decrypted + 32);
    strcpy(m4, (char*)decrypted + 32 + strlen(ida) + 1);

    // Проверяем r_B и r_A
    if (memcmp(rb_received, r_b, 16) != 0 || memcmp(ra_received, r_a, 16) != 0 || strcmp(ida, "Client_A") != 0) {
        handle_error("Authentication failed at step 3");
    }

    printf("[CLIENT_B] Аутентификация успешна!\n");
    printf("Полученные данные: M5=%s, M4=%s\n", m5, m4);

    close(new_socket);
    close(server_fd);
    return 0;
}
