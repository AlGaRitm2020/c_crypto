#include <stdio.h>
#include <gmp.h>

int main() {
    mpz_t n;
    mpz_init(n);
    mpz_set_ui(n, 123456);  // Явное значение

    // Способ 1: Стандартный printf (может не работать)
    printf("Via printf: %Zd\n", n);

    // Способ 2: gmp_printf (должен работать всегда)
    gmp_printf("Via gmp_printf: %Zd\n", n);

    mpz_clear(n);
    return 0;
}
