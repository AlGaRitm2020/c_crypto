#include "essential_func.h"
#include "rsa.h"
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

void rsa_save_key(mpz_t n, mpz_t exp, char* filename, int verbose) {
  if (verbose) printf("SAVING TO %s...\n", filename);
  FILE* stream = fopen(filename, "w");
  char string[2048];
  gmp_sprintf(string, "%Zd\n%Zd\n",n, exp); 
  fprintf(stream,"%s", string);
  fclose(stream);
  if (verbose) printf("Success!\n", filename);
}

void rsa_load_key(mpz_t n, mpz_t exp, char* filename, int verbose) {
  if (verbose) printf("LOADING %s...\n", filename);
  FILE* stream = fopen(filename, "r");
  size_t size = 100;
  char* n_str=(char*)malloc(sizeof(char)*size);
  char* exp_str=(char*)malloc(sizeof(char)*size);
  size_t charCount = getline(&n_str, &size, stream);
  getline(&exp_str, &size, stream);

  mpz_set_str(n, n_str, 10);
  mpz_set_str(exp, exp_str, 10);
  // printf("%s   \n%s\n",n_str, exp_str); 
  if(verbose) gmp_printf("n = %Zd\n exponent = %Zd\n", n, exp);
  // fprintf(stream,"%s", string);
  fclose(stream);
  free(n_str);
  free(exp_str);
  if (verbose) printf("Success!\n", filename);
}
void rsa_gen_key(int bits, char* pubKeyFile, char* priKeyFile, int verbose) {
    gmp_randstate_t state;
    gmp_randinit_default(state);
    unsigned long seed = (unsigned long)time(NULL);
    gmp_randseed_ui(state, seed);
    
    mpz_t p, q, n, e, phi, p_minus_one, q_minus_one;
    mpz_inits(p,q,n, phi, e, p_minus_one, q_minus_one, NULL);
  
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
    if (verbose) gmp_printf("GENKEY FUNC:\nn:%Zd\nphi:%Zd\ne:%Zd\nd:%Zd\n", n, phi,e, x);
    
    // rsa_save_key(n, e, "gg.pub", 1);
    // rsa_save_key(n, x, "gg.pri", 1);
    // rsa_load_key(n, e, "gg.pub", 1);
  
    rsa_save_key(n, e, pubKeyFile, verbose);
    rsa_save_key(n, x, priKeyFile, verbose);
    rsa_load_key(n, e, pubKeyFile, verbose);
    
    gmp_randclear(state);
    // mpz_clears(x,y,gcd, p,q,n,phi,e, p_minus_one, q_minus_one);
    mpz_clears( x,y,gcd,p,q,n,phi,e, p_minus_one, q_minus_one, NULL);
}

void rsa_encode(char* message, size_t size, char* pubKeyFile, int verbose) {
      // fast_power_mod(n, e, ten, ten);
  mpz_t n, e;
  mpz_inits(n,e,NULL);
  
  rsa_load_key(n, e, pubKeyFile, verbose);


  printf("ok"); 
  mpz_clears(n,e,NULL);
}

