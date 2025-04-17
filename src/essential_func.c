#include "essential_func.h"
#include <stdio.h>
#include <stdbool.h>


void extendedGCD(mpz_t a, mpz_t b, mpz_t gcd, mpz_t x, mpz_t y) {
    if (mpz_cmp_ui(b, 0) == 0) {
        mpz_set(gcd, a);
        mpz_set_ui(x, 1);
        mpz_set_ui(y, 0);
    } else {
        mpz_t tempX, tempY, tempB, tempA;
        mpz_init(tempX);
        mpz_init(tempY);
        mpz_init(tempB);
        mpz_init(tempA);

        mpz_set(tempB, b);
        mpz_mod(tempA, a, b);

        extendedGCD(tempB, tempA, gcd, tempY, tempX);

        mpz_tdiv_q(y, a, b); // y = a / b
        mpz_mul(y, y, tempX); // y = (a / b) * tempX
        mpz_sub(y, tempY, y); // y = tempY - y

        mpz_set(x, tempX);

        mpz_clear(tempX);
        mpz_clear(tempY);
        mpz_clear(tempB);
        mpz_clear(tempA);
    }
}
void fast_power(mpz_t result, const mpz_t base, const mpz_t exponent) {
    mpz_t temp_base, temp_exp;
    mpz_init(temp_base);
    mpz_init(temp_exp);

    // Initialize result to 1
    mpz_set_ui(result, 1);

    // Copy base and exponent to temporary variables
    mpz_set(temp_base, base);
    mpz_set(temp_exp, exponent);

    while (mpz_sgn(temp_exp) > 0) {
        if (mpz_tstbit(temp_exp, 0)) {
            // If the least significant bit is 1, multiply result by temp_base
            mpz_mul(result, result, temp_base);
        }
        // Square the base
        mpz_mul(temp_base, temp_base, temp_base);
        // Right shift the exponent
        mpz_tdiv_q_2exp(temp_exp, temp_exp, 1);
    }

    mpz_clear(temp_base);
    mpz_clear(temp_exp);
}



void fast_power_mod(mpz_t result, const mpz_t base, const mpz_t exponent, const mpz_t modulus) {
    mpz_t temp_base, temp_exp;
    mpz_init(temp_base);
    mpz_init(temp_exp);

    // Initialize result to 1
    mpz_set_ui(result, 1);

    // Copy base and exponent to temporary variables
    mpz_set(temp_base, base);
    mpz_set(temp_exp, exponent);

    while (mpz_sgn(temp_exp) > 0) {
        if (mpz_tstbit(temp_exp, 0)) {
            // If the least significant bit is 1, multiply result by temp_base and take modulus
            mpz_mul(result, result, temp_base);
            mpz_mod(result, result, modulus);
        }
        // Square the base and take modulus
        mpz_mul(temp_base, temp_base, temp_base);
        mpz_mod(temp_base, temp_base, modulus);
        // Right shift the exponent
        mpz_tdiv_q_2exp(temp_exp, temp_exp, 1);
    }

    mpz_clear(temp_base);
    mpz_clear(temp_exp);
}

bool miller_rabin_test(const mpz_t n, int k, gmp_randstate_t state) {
    // Проверка случаев до 5
    if (mpz_cmp_ui(n, 2) < 0) return false; 
    if (mpz_cmp_ui(n, 2) == 0) return true; 
    if (mpz_even_p(n)) return false; // все четные - составные (кроме 2) 

    mpz_t q, s, a, x, n_minus_1;
    mpz_inits(q, s, a, x, n_minus_1, NULL);

    mpz_sub_ui(n_minus_1, n, 1); // n_minus_1 = n - 1
    mpz_set(q, n_minus_1);       // q = n - 1
    mpz_set_ui(s, 0);            // s = 0

    // n - 1 = 2^s * q. Находим s и q
    while (mpz_even_p(q)) {
        mpz_div_2exp(q, q, 1); // q = q / 2
        mpz_add_ui(s, s, 1);   // s = s + 1
    }

    // Повторяем тест k раз
    for (int i = 0; i < k; i++) {
        // Генерируем случайное a ∈ {1, ..., n-1}
        mpz_urandomm(a, state, n_minus_1); // a ∈ [0, n-2]
        mpz_add_ui(a, a, 1);               // a ∈ [1, n-1]

        // Вычисляем x = a^q mod n
        fast_power_mod(x, a, q, n);

        // Если x == 1 или x == n-1, число проходит тест
        if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, n_minus_1) == 0) {
            continue;
        }

        bool is_prime = false;
	mpz_t two;
	mpz_init(two);
	mpz_set_ui(two,2); 
        for (mpz_t r; mpz_cmp(r, s) < 0; mpz_add_ui(r, r, 1)) {
            fast_power_mod(x, x, two, n); // x = x^2 mod n
            //mpz_powm(x, x, two, n); // x = x^2 mod n
	    //gmp_printf("x is: %Zd\n", x);

            // x == n-1, число проходит тест

            if (mpz_cmp(x, n_minus_1) == 0) {
                is_prime = true;
                break;
            }

            // x == 1, число составное
            if (mpz_cmp_ui(x, 1) == 0) {
                mpz_clears(q, s, a, x, n_minus_1, NULL);
                return false;
            }
        }
	// если ни разу не был установлен флаг в true, то составное
        if (!is_prime) {
            mpz_clears(q, s, a, x, n_minus_1, NULL);
            return false;
        }
    }

    mpz_clears(q, s, a, x, n_minus_1, NULL);
    return true;
}

void generate_prime(mpz_t prime, int bits, gmp_randstate_t state) {
    mpz_t temp;
    mpz_init(temp);
    
    do {
        mpz_urandomb(temp, state, bits);
        mpz_setbit(temp, bits - 1);  // Ensure high bit is set for correct bit length
        mpz_setbit(temp, 0);         // Make odd
        
        // Check if the number is prime with high probability
    } while (!miller_rabin_test(temp, 40, state));  // 40 rounds for high confidence
    
    mpz_set(prime, temp);
    mpz_clear(temp);
}
