#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "rsa.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


int is_timestamp_valid(char* timestamp) {
    return 1;
    // time_t now = time(NULL);
    // double diff_seconds = difftime(now, timestamp);
    // return (diff_seconds >= 0 && diff_seconds <= 120);  // Не старше 2 минут
}

//
// int is_timestamp_valid(const char* timestamp) {
//     struct tm tm_time = {0};
//     time_t timestamp_time, current_time;
//     double diff_seconds;
//
//     // Парсим строку timestamp в struct tm
//     if (strptime(timestamp, "%Y-%m-%d %H:%M:%S", &tm_time) == NULL) {
//         return 0; // Ошибка парсинга
//     }
//
//     // Преобразуем в time_t
//     timestamp_time = mktime(&tm_time);
//     current_time = time(NULL); // Текущее время
//
//     // Разница в секундах
//     diff_seconds = difftime(current_time, timestamp_time);
//
//     // Проверяем что разница не более 2 минут (120 секунд) и не отрицательная
//     return (diff_seconds >= 0 && diff_seconds <= 120);
// }

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Разрешаем повторное использование порта
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Ожидание соединений
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("TSA сервер запущен на порту 8080...\n");

    while (1) {
        // Принятие соединения
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        printf("Новое соединение принято\n");



        // Чтение длины хеша
        size_t hash_len;
        if (read(new_socket, &hash_len, sizeof(size_t)) != sizeof(size_t)) {
            perror("read hash_len");
            close(new_socket);
            continue;
        }

        // Чтение хеша
        uint8_t *hash = malloc(hash_len);
        if (!hash) {
            perror("malloc hash");
            close(new_socket);
            continue;
        }

        if (read(new_socket, hash, hash_len) != hash_len) {
            perror("read hash");
            free(hash);
            close(new_socket);
            continue;
        }

        // Чтение длины timestamp
        size_t timestamp_len;
        if (read(new_socket, &timestamp_len, sizeof(size_t)) != sizeof(size_t)) {
            perror("read timestamp_len");
            free(hash);
            close(new_socket);
            continue;
        }

        // Чтение timestamp
        char *timestamp = malloc(timestamp_len);
        if (!timestamp) {
            perror("malloc timestamp");
            free(hash);
            close(new_socket);
            continue;
        }



        if (read(new_socket, timestamp, timestamp_len) != timestamp_len) {
            perror("read timestamp");
            free(hash);
            free(timestamp);
            close(new_socket);
            continue;
        }


        printf("Получены данные: hash_len=%zu, timestamp='%s'\n", hash_len, timestamp);

        // Создание подписи
        if (!is_timestamp_valid(timestamp)) {
            free(hash);
            free(timestamp);
            close(new_socket);
            continue;
            perror("old timestamp (>=120sec)");
         }
        char *tbs = (char*)malloc(hash_len + timestamp_len);
        if (!tbs) {
            perror("malloc tbs");
            free(hash);
            free(timestamp);
            close(new_socket);
            continue;
        }

        memcpy(tbs, hash, hash_len);
        memcpy(tbs + hash_len, timestamp, timestamp_len);

        char *ts_sig = NULL;
        size_t ts_sig_len = 0;
        rsa_encode(tbs, hash_len + timestamp_len, "keys/ts", &ts_sig, &ts_sig_len, 1); 

        // Отправка длины подписи
        if (write(new_socket, &ts_sig_len, sizeof(size_t)) != sizeof(size_t)) {
            perror("write sig_len");
            free(hash);
            free(timestamp);
            free(tbs);
            free(ts_sig);
            close(new_socket);
            continue;
        }

        // Отправка подписи
        if (write(new_socket, ts_sig, ts_sig_len) != ts_sig_len) {
            perror("write signature");
            free(hash);
            free(timestamp);
            free(tbs);
            free(ts_sig);
            close(new_socket);
            continue;
        }

        printf("Подпись отправлена (%zu байт)\n", ts_sig_len);

        // Очистка
        free(hash);
        free(timestamp);
        free(tbs);
        free(ts_sig);
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
