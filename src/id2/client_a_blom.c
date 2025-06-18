#include "common.h"

void compute_shared_key(mpz_t k, mpz_t ai, mpz_t bi, mpz_t xj, mpz_t p) {
    // k = ai + bi * xj mod p
    mpz_mul(k, bi, xj);
    mpz_add(k, k, ai);
    mpz_mod(k, k, p);
}

void user(const char *user_name, int xi, int xj, int port) {
    int sock = connect_to("127.0.0.1", PORT_TC);
    printf("[%s] Connected to Trusted Center\n", user_name);

    // Получаем параметры от доверенного центра
    mpz_t p, ai, bi;
    mpz_init(p);
    mpz_init(ai);
    mpz_init(bi);
    
    recv_mpz(sock, p);
    recv_mpz(sock, ai);
    recv_mpz(sock, bi);
    
    gmp_printf("[%s] Received parameters: p=%Zd, ai=%Zd, bi=%Zd\n", user_name, p, ai, bi);
    close(sock);

    // Вычисляем общий ключ
    mpz_t k;
    mpz_init(k);
    
    mpz_t xj_mpz;
    mpz_init_set_ui(xj_mpz, xj);
    
    compute_shared_key(k, ai, bi, xj_mpz, p);
    gmp_printf("[%s] Computed shared key: %Zd\n", user_name, k);

    // Очистка
    mpz_clear(p);
    mpz_clear(ai);
    mpz_clear(bi);
    mpz_clear(k);
    mpz_clear(xj_mpz);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <A|B>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "A") == 0) {
        user("A", 1, 2, PORT_USER_A); // User A has xi=1, needs xj=2 (B's id)
    } else if (strcmp(argv[1], "B") == 0) {
        user("B", 2, 1, PORT_USER_B); // User B has xi=2, needs xj=1 (A's id)
    } else {
        printf("Invalid user. Use A or B.\n");
        return 1;
    }

    return 0;
}
