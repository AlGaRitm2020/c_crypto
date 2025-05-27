#include "common.h"
#include <time.h>

#define ID_A "Client_A"
#define ID_B "Client_B"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    uint8_t key[KEY_SIZE] = "0123456789abcdef"; // Предварительно согласованный ключ
    uint8_t encrypted[1024], decrypted[1024];
    int ciphertext_len;

    char choice;
    printf("Использовать временную метку (t) или случайное число (r)? ");
    scanf("%c", &choice);

    // Сообщения M1, M2, M3
    char m1[256], m2_input[256], m3[256];

    printf("Введите сообщение M1: ");
    scanf(" %[^\n]%*c", m1);
    printf("Введите сообщение M2: ");
    scanf(" %[^\n]%*c", m2_input);
    printf("Введите сообщение M3: ");
    scanf(" %[^\n]%*c", m3);

    // R_A
    char r_a[32] = {0};
    if (choice == 't') {
        time_t t = time(NULL);
        snprintf(r_a, sizeof(r_a), "%ld", t);
    } else {
        RAND_bytes((uint8_t*)r_a, 16);
        memcpy(r_a + 16, "", 1); // безопасное завершение строки
    }

    // Формируем сообщение: IDA || R_A || M1
    char msg[1024];
    snprintf(msg, sizeof(msg), "%s%s%s", ID_A, r_a, m1);
    int msg_len = strlen(msg);

    // Шифруем
    aes_encrypt((uint8_t*)msg, msg_len, key, encrypted, &ciphertext_len);

    // Устанавливаем соединение
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

    // Отправляем зашифрованное сообщение
    send(sock, encrypted, ciphertext_len, 0);
    printf("[CLIENT_A] Отправлено зашифрованное сообщение (%d байт)\n", ciphertext_len);

    // Получаем ответ
    uint8_t response[1024];
    ssize_t received = read(sock, response, 1024);
    if (received <= 0) handle_error("read() failed");

    int decrypted_len = aes_decrypt(response, received, key, decrypted);
    decrypted[decrypted_len] = '\0';
    printf("[CLIENT_A] Расшифрованное сообщение: %s\n", decrypted);

    close(sock);
    return 0;
}
