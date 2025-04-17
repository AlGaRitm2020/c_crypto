#ifndef ESSENTIAL_FUNC_H
#define ESSENTIAL_FUNC_H

#include <gmp.h>
#include <stdbool.h>


void extendedGCD(mpz_t a, mpz_t b, mpz_t gcd, mpz_t x, mpz_t y);

void fast_power(mpz_t result, const mpz_t base, const mpz_t exponent);

void fast_power_mod(mpz_t result, const mpz_t base, const mpz_t exponent, const mpz_t modulus); 

bool miller_rabin_test(const mpz_t n, int k, gmp_randstate_t state); 

void generate_prime(mpz_t prime, int bits, gmp_randstate_t state);

#endif
