#include "essential_func.h"
#include "rsa.h"
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

void rsa_gen_key(int bits, char* pubKeyFile, char* priKeyFile) {
    // p, q - random
    gmp_randstate_t state;
    // Initialize random state with default algorithm
    gmp_randinit_default(state);
    
    // Seed the random state (using current time as seed)
    unsigned long seed = (unsigned long)time(NULL);
    gmp_randseed_ui(state, seed);
    
     mpz_t p, q, n, e, phi, p_minus_one, q_minus_one;
     mpz_inits(p,q,n, phi, e, p_minus_one, q_minus_one, NULL);
    printf("ho\n");
    ///*
    generate_prime(p, (bits / 2), state); 
    generate_prime(q, (bits / 2), state); 

    mpz_sub_ui(p_minus_one, p, 1);
    mpz_sub_ui(q_minus_one, q, 1);

    mpz_mul(n, p, q);
    mpz_mul(phi, p_minus_one, q_minus_one);

    mpz_init_set_ui(e, 65537);

    mpz_t x,y,gcd;
    mpz_inits(x,y,gcd, NULL);
    extendedGCD(e, phi, gcd, x, y); // 1 = e * x + phi * y
                                    // 1 = e * x (mod phi)
                                    // d = x = e^-1 (mod phi)
    mpz_add(x,x,phi);
    mpz_mod(x,x,phi);
    
    gmp_printf("n:%Zd\nphi:%Zd\ne:%Zd\nd:%Zd\n", n, phi,e, x);
    
    // */
    gmp_randclear(state);
    // mpz_clears(x,y,gcd, p,q,n,phi,e, p_minus_one, q_minus_one);
    mpz_clears( p,q,n,phi,e, p_minus_one, q_minus_one, NULL);
  
    
}

void rsa_encode(char* message, size_t size, mpz_t n, mpz_t e) {
  // extracting file
  // padding 128bit
   //
   //  mpz_t exp, mod;
   //  mpz_init(mod);
   //  mpz_init_set_ui(exp, 10);
   //
      mpz_t ten;
      mpz_init_set_ui(ten, 10);
      fast_power_mod(n, e, ten, ten);
   //  mpz_powm(n, e, n, ten); 
   //  mpz_clear(ten);
    //mpz_set_ui(n, 1000);
    gmp_printf("new n: indise %Zd \n", n);
   //
  //
   // mpz_clears(exp, mod);
   //
   //
  
  // char* cyphermessage = (char*)malloc(size * )

  printf("ok"); 
}

