#include "common.h"

int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);

    mpz_t p, g;
    mpz_inits(p, g, NULL);

    // Генерируем параметры
    printf("Server: Generating prime p...\n");
    generate_prime(p, 512, state);

    printf("Server: Generating primitive root g...\n");
    generate_primitive_root(g, p, state);

    // Сохраняем в файл diff.key
    FILE *fp = fopen("diff.key", "w");
    if (!fp) {
        handle_error("Failed to open diff.key");
    }

    char *p_str = mpz_get_str(NULL, 16, p);
    char *g_str = mpz_get_str(NULL, 16, g);

    fprintf(fp, "p=%s\ng=%s\n", p_str, g_str);
    fclose(fp);

    free(p_str);
    free(g_str);
    mpz_clears(p, g, NULL);

    printf("Server: Parameters saved to diff.key\n");
    return 0;
}
