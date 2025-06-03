#include "common.h"
#include <time.h>

#define ID_A "Client_A"
#define ID_B "Client_B"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    uint8_t key[KEY_SIZE] = "0123456789abcdef";
    uint8_t encrypted[1024], decrypted[1024];
    int ciphertext_len;

    // Шаг 1: Генерация r_A и M1
    uint8_t r_a[16];
    RAND_bytes(r_a, 16);

    char m1[256];
    printf("Введите сообщение M1: ");
    scanf(" %[^\n]%*c", m1);

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

    // Шаг 1: A → B: r_A (16 байт) + M1 (текст)
    send(sock, r_a, 16, 0); // Отправляем r_A как бинарные данные
    send(sock, m1, strlen(m1) + 1, 0); // Отправляем M1 как строку
    printf("[CLIENT_A] Отправлен шаг 1: r_A (16 байт) + M1=%s\n", m1);

    // Шаг 2: Получаем M3 (текст) + зашифрованные данные (бинарные)
    char m3[256];
    uint8_t encrypted_part[1024];
    
    // Сначала получаем M3 (до символа '|')
    int bytes_received = 0;
    char c;
    while (recv(sock, &c, 1, 0) > 0) {
        if (c == '|') break;
        m3[bytes_received++] = c;
    }
    m3[bytes_received] = '\0';

    // Затем получаем зашифрованные данные
    int enc_size = recv(sock, encrypted_part, 1024, 0);
    if (enc_size <= 0) handle_error("Failed to receive encrypted part");

    // Расшифровываем
    int decrypted_len = aes_decrypt(encrypted_part, enc_size, key, decrypted);
    decrypted[decrypted_len] = '\0';

    // Парсим расшифрованные данные: r_A, r_B, B, M2
    uint8_t ra_received[16];
    uint8_t r_b[16];
    char idb[32], m2[256];
    
    memcpy(ra_received, decrypted, 16);
    memcpy(r_b, decrypted + 16, 16);
    strcpy(idb, (char*)decrypted + 32);
    strcpy(m2, (char*)decrypted + 32 + strlen(idb) + 1);

    // Проверяем r_A и IDB
    if (memcmp(ra_received, r_a, 16) != 0 || strcmp(idb, ID_B) != 0) {
        handle_error("Authentication failed at step 2");
    }

    // Шаг 3: Готовим M5 и зашифрованную часть
    char m5[256], m4[256];
    printf("Введите сообщение M4: ");
    scanf(" %[^\n]%*c", m4);
    printf("Введите сообщение M5: ");
    scanf(" %[^\n]%*c", m5);

    // Формируем зашифрованную часть: r_B (16), r_A (16), A, M4
    uint8_t to_encrypt[1024];
    memcpy(to_encrypt, r_b, 16);
    memcpy(to_encrypt + 16, r_a, 16);
    strcpy((char*)to_encrypt + 32, ID_A);
    strcpy((char*)to_encrypt + 32 + strlen(ID_A) + 1, m4);
    int to_encrypt_len = 32 + strlen(ID_A) + 1 + strlen(m4) + 1;

    aes_encrypt(to_encrypt, to_encrypt_len, key, encrypted, &ciphertext_len);

    // Формируем полное сообщение шага 3: M5 | encrypted_data
    send(sock, m5, strlen(m5) + 1, 0);
    send(sock, "|", 1, 0);
    send(sock, encrypted, ciphertext_len, 0);
    printf("[CLIENT_A] Отправлен шаг 3: M5=%s | encrypted data\n", m5);

    close(sock);
    return 0;
}
