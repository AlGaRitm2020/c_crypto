#include "common.h"

void generate_blom_secrets(BlomSecrets *secrets, mpz_t p) {
    // Инициализация GMP генератора случайных чисел
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Генерация случайных коэффициентов a, b, c
    mpz_init(secrets->a);
    mpz_init(secrets->b);
    mpz_init(secrets->c);
    
    mpz_urandomm(secrets->a, state, p);
    mpz_urandomm(secrets->b, state, p);
    mpz_urandomm(secrets->c, state, p);

    gmp_printf("[TC] Generated secrets: a=%Zd, b=%Zd, c=%Zd\n", secrets->a, secrets->b, secrets->c);
    
    gmp_randclear(state);
}

void trusted_center() {
    int server_fd, client_a, client_b;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    mpz_t p;
    mpz_init(p);
    mpz_set_str(p, "170141183460469231731687303715884105727", 10); // большое простое число 2^127-1

    BlomSecrets secrets;
    generate_blom_secrets(&secrets, p);

    // Создаем сервер
    server_fd = create_server_socket(PORT_TC);
    printf("[TC] Trusted Center started on port %d\n", PORT_TC);

    // Принимаем соединение от пользователя A
    if ((client_a = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        handle_error("accept failed");
    }
    printf("[TC] User A connected\n");

    // Принимаем соединение от пользователя B
    if ((client_b = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        handle_error("accept failed");
    }
    printf("[TC] User B connected\n");

    // Отправляем параметры пользователям
    send_mpz(client_a, p);
    send_mpz(client_b, p);

    // Генерируем и отправляем ключевые материалы для пользователя A (xi = 1)
    mpz_t xi_a;
    mpz_init_set_ui(xi_a, 1);
    
    UserKey key_a;
    mpz_init(key_a.ai);
    mpz_init(key_a.bi);
    compute_user_key(&key_a, xi_a, &secrets, p);
    
    send_mpz(client_a, key_a.ai);
    send_mpz(client_a, key_a.bi);

    // Генерируем и отправляем ключевые материалы для пользователя B (xi = 2)
    mpz_t xi_b;
    mpz_init_set_ui(xi_b, 2);
    
    UserKey key_b;
    mpz_init(key_b.ai);
    mpz_init(key_b.bi);
    compute_user_key(&key_b, xi_b, &secrets, p);
    
    send_mpz(client_b, key_b.ai);
    send_mpz(client_b, key_b.bi);

    printf("[TC] Key materials sent to both users\n");

    // Очистка
    close(client_a);
    close(client_b);
    close(server_fd);
    mpz_clear(p);
    mpz_clear(secrets.a);
    mpz_clear(secrets.b);
    mpz_clear(secrets.c);
}

int main() {
    trusted_center();
    return 0;
}
