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

    // Шаг 1: Получаем M2 (открыто) | зашифрованные данные
    char m2[256];
    uint8_t encrypted_part[1024];
    
    // Чтение M2 до разделителя
    int bytes_received = 0;
    char c;
    while (recv(new_socket, &c, 1, 0) > 0) {
        if (c == '|') break;
        m2[bytes_received++] = c;
    }
    m2[bytes_received] = '\0';

    // Чтение зашифрованной части
    int enc_size = recv(new_socket, encrypted_part, 1024, 0);
    if (enc_size <= 0) handle_error("Failed to receive encrypted part");

    // Расшифровка
    uint8_t decrypted[1024];
    int decrypted_len = aes_decrypt(encrypted_part, enc_size, key, decrypted);
    decrypted[decrypted_len] = '\0';

    // Определяем тип аутентификатора (8 байт - time_t, 16 - random)
    int auth_len = (decrypted[0] & 0x80) ? sizeof(time_t) : 16; // Эвристическое определение
    
    // Парсинг: auth_data, A, M1
    uint8_t auth_data[24];
    char ida[32], m1[256];
    
    memcpy(auth_data, decrypted, auth_len);
    strcpy(ida, (char*)(decrypted + auth_len));
    strcpy(m1, (char*)(decrypted + auth_len + strlen(ida) + 1));

    printf("[CLIENT_B] Получено M2=%s, IDA=%s, M1=%s\n", m2, ida, m1);

    // Шаг 2: Подготовка ответа
    char m3[256], m4[256];
    printf("Введите сообщение M3: ");
    scanf(" %[^\n]%*c", m3);
    printf("Введите сообщение M4: ");
    scanf(" %[^\n]%*c", m4);

    // Формируем зашифрованную часть: auth_data (то же что получили), B, M3
    uint8_t to_encrypt[1024];
    memcpy(to_encrypt, auth_data, auth_len);
    strcpy((char*)(to_encrypt + auth_len), ID_B);
    strcpy((char*)(to_encrypt + auth_len + strlen(ID_B) + 1), m3);
    int to_encrypt_len = auth_len + strlen(ID_B) + 1 + strlen(m3) + 1;

    uint8_t encrypted[1024];
    int ciphertext_len;
    aes_encrypt(to_encrypt, to_encrypt_len, key, encrypted, &ciphertext_len);

    // Отправка: M4 (открыто) | зашифрованные данные
    send(new_socket, m4, strlen(m4) + 1, 0);
    send(new_socket, "|", 1, 0);
    send(new_socket, encrypted, ciphertext_len, 0);
    printf("[CLIENT_B] Отправлен шаг 2: M4=%s | encrypted data\n", m4);

    close(new_socket);
    close(server_fd);
    return 0;
}
