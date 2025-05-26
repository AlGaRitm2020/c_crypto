#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "essential_func.h"

typedef struct {
    mpz_t p;
    mpz_t alpha;
    mpz_t beta;
} PublicKey;

typedef struct {
    mpz_t a;
} PrivateKey;

typedef struct {
    mpz_t p;
    mpz_t g;
    PublicKey public_key;
    PrivateKey private_key;
} ElGamal;

void elgamal_init(ElGamal *eg) {
    mpz_inits(eg->p, eg->g, NULL);
    mpz_inits(eg->public_key.p, eg->public_key.alpha, eg->public_key.beta, NULL);
    mpz_init(eg->private_key.a);
}

void elgamal_clear(ElGamal *eg) {
    mpz_clears(eg->p, eg->g, NULL);
    mpz_clears(eg->public_key.p, eg->public_key.alpha, eg->public_key.beta, NULL);
    mpz_clear(eg->private_key.a);
}

void public_key_init(PublicKey *pk) {
    mpz_inits(pk->p, pk->alpha, pk->beta, NULL);
}

void public_key_clear(PublicKey *pk) {
    mpz_clears(pk->p, pk->alpha, pk->beta, NULL);
}

void generate_system(ElGamal *eg, gmp_randstate_t state) {
    // Generate prime p
    generate_prime(eg->p, 512, state);
    
    // Generate generator g
    mpz_t alpha, p_minus_1, temp;
    mpz_inits(alpha, p_minus_1, temp, NULL);
    mpz_sub_ui(p_minus_1, eg->p, 1);
    
    do {
        mpz_urandomm(alpha, state, p_minus_1);
        mpz_add_ui(alpha, alpha, 1); // alpha in [1, p-1]
        
        mpz_powm_ui(temp, alpha, 2, eg->p);
    } while (mpz_cmp_ui(temp, 1) == 0 || 
             mpz_divisible_p(p_minus_1, temp) ||
             mpz_cmp(temp, p_minus_1) == 0);
    
    mpz_set(eg->g, alpha);
    mpz_set(eg->public_key.alpha, alpha);
    
    // Generate private key a
    mpz_t a;
    mpz_init(a);
    mpz_sub_ui(temp, eg->p, 2); // p-2
    mpz_urandomm(a, state, temp);
    mpz_add_ui(a, a, 1); // a in [1, p-2]
    mpz_set(eg->private_key.a, a);
    
    // Compute public key beta = alpha^a mod p
    mpz_powm(eg->public_key.beta, alpha, a, eg->p);
    
    mpz_set(eg->public_key.p, eg->p);
    
    mpz_clears(alpha, p_minus_1, temp, a, NULL);
}

void sign_message(ElGamal *eg, const char *message, char **y_str, char **delta_str, gmp_randstate_t state) {
    // Convert message to number
    mpz_t message_num;
    mpz_init(message_num);
    mpz_import(message_num, strlen(message), 1, 1, 0, 0, message);
    
    mpz_t r, gcd, y, u, delta, p_minus_1, temp;
    mpz_inits(r, gcd, y, u, delta, p_minus_1, temp, NULL);
    mpz_sub_ui(p_minus_1, eg->p, 1);
    
    // Find suitable r
    do {
        mpz_sub_ui(temp, eg->p, 2); // p-2
        mpz_urandomm(r, state, temp);
        mpz_add_ui(r, r, 1); // r in [1, p-2]
        
        mpz_t x, y;
        mpz_inits(x, y, NULL);
        extendedGCD(r, p_minus_1, gcd, x, y);
        mpz_clears(x, y, NULL);
    } while (mpz_cmp_ui(gcd, 1) != 0);
    
    // Compute y = g^r mod p
    mpz_powm(y, eg->g, r, eg->p);
    
    // Compute u = r^-1 mod (p-1)
    mpz_t x, y_ext;
    mpz_inits(x, y_ext, NULL);
    extendedGCD(r, p_minus_1, gcd, x, y_ext);
    
    if (mpz_cmp_ui(x, 0) < 0) {
        mpz_add(x, x, p_minus_1);
    }
    mpz_set(u, x);
    
    // Compute delta = (message_num - a*y)*u mod (p-1)
    mpz_mul(temp, eg->private_key.a, y);
    mpz_sub(temp, message_num, temp);
    mpz_mul(temp, temp, u);
    mpz_mod(delta, temp, p_minus_1);
    
    if (mpz_cmp_ui(delta, 0) < 0) {
        mpz_add(delta, delta, p_minus_1);
    }
    
    // Convert y and delta to hex strings
    *y_str = mpz_get_str(NULL, 16, y);
    *delta_str = mpz_get_str(NULL, 16, delta);
    
    mpz_clears(message_num, r, gcd, y, u, delta, p_minus_1, temp, x, y_ext, NULL);
}

bool check_signature(ElGamal *eg, const char *y_str, const char *delta_str, const char *message) {
    mpz_t y, delta, message_num;
    mpz_inits(y, delta, message_num, NULL);
    
    mpz_set_str(y, y_str, 16);
    mpz_set_str(delta, delta_str, 16);
    mpz_import(message_num, strlen(message), 1, 1, 0, 0, message);
    
    mpz_t left, right, temp1, temp2;
    mpz_inits(left, right, temp1, temp2, NULL);
    
    // left = (beta^y * y^delta) mod p
    mpz_powm(temp1, eg->public_key.beta, y, eg->p);
    mpz_powm(temp2, y, delta, eg->p);
    mpz_mul(left, temp1, temp2);
    mpz_mod(left, left, eg->p);
    
    // right = alpha^message_num mod p
    mpz_powm(right, eg->public_key.alpha, message_num, eg->p);
    
    bool result = (mpz_cmp(left, right) == 0);
    
    mpz_clears(y, delta, message_num, left, right, temp1, temp2, NULL);
    
    return result;
}

#ifndef LIB

int main() {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    
    ElGamal eg;
    elgamal_init(&eg);
    generate_system(&eg, state);
    
    const char *message = "H";
    
    char *y_str = NULL;
    char *delta_str = NULL;
    sign_message(&eg, message, &y_str, &delta_str, state);
    
    bool valid = check_signature(&eg, y_str, delta_str, message);
    printf("Signature is %s\n", valid ? "valid" : "invalid");
    
    free(y_str);
    free(delta_str);
    elgamal_clear(&eg);
    gmp_randclear(state);
    
    return 0;
}

#endif
