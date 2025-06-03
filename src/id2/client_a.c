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

    // Выбор временной метки или случ. числа 
    char choice;
    printf("Использовать временную метку (t) или случайное число (r)? ");
    scanf(" %c", &choice);

    // Ввод сообщений
    char m1[256], m2[256];
    printf("Введите сообщение M1: ");
    scanf(" %[^\n]%*c", m1);
    printf("Введите сообщение M2: ");
    scanf(" %[^\n]%*c", m2);

    // Подготовка аутентификатора (t_A или r_A)
    uint8_t auth_data[16] = {0}; 
    int auth_len = 0;

    if (choice == 't') {
        time_t t_a = time(NULL);
        memcpy(auth_data, &t_a, sizeof(time_t));
        auth_len = sizeof(time_t);
        printf("[CLIENT_A] Используется временная метка: %ld\n", t_a);
    } else {
        RAND_bytes(auth_data, 16);
        auth_len = 16;
        printf("[CLIENT_A] Используется случайное число (16 байт)\n");
    }

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

    // Шаг 1: Формируем зашифрованную часть: auth_data, A, M1
    uint8_t to_encrypt[1024];
    memcpy(to_encrypt, auth_data, auth_len);
    strcpy((char*)(to_encrypt + auth_len), ID_A);
    strcpy((char*)(to_encrypt + auth_len + strlen(ID_A) + 1), m1);
    int to_encrypt_len = auth_len + strlen(ID_A) + 1 + strlen(m1) + 1;

    aes_encrypt(to_encrypt, to_encrypt_len, key, encrypted, &ciphertext_len);

    // Отправка: M2 (открыто) | зашифрованные данные
    send(sock, m2, strlen(m2) + 1, 0);
    send(sock, "|", 1, 0);
    send(sock, encrypted, ciphertext_len, 0);
    printf("[CLIENT_A] Отправлен шаг 1: M2=%s | encrypted data\n", m2);

    // Шаг 2: Получаем ответ
    char m4[256];
    uint8_t encrypted_part[1024];
    
    // Чтение M4 до разделителя
    int bytes_received = 0;
    char c;
    while (recv(sock, &c, 1, 0) > 0) {
        if (c == '|') break;
        m4[bytes_received++] = c;
    }
    m4[bytes_received] = '\0';

    // Чтение зашифрованной части
    int enc_size = recv(sock, encrypted_part, 1024, 0);
    if (enc_size <= 0) handle_error("Failed to receive encrypted part");

    // Расшифровка
    int decrypted_len = aes_decrypt(encrypted_part, enc_size, key, decrypted);
    decrypted[decrypted_len] = '\0';

    // Парсинг: auth_data, B, M3
    uint8_t auth_received[24];
    char idb[32], m3[256];
    
    memcpy(auth_received, decrypted, auth_len);
    strcpy(idb, (char*)(decrypted + auth_len));
    strcpy(m3, (char*)(decrypted + auth_len + strlen(idb) + 1));

    // Проверка аутентификатора
    if (memcmp(auth_received, auth_data, auth_len) != 0) {
        handle_error("Authentication data mismatch");
    }

    printf("[CLIENT_A] Аутентификация успешна! Получено M4=%s, M3=%s\n", m4, m3);
    close(sock);
    return 0;
}
