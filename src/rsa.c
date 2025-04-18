#include "essential_func.h"
#include "rsa.h"
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

void rsa_save_key(mpz_t n, mpz_t exp, char* filename, int verbose) {
  if (verbose) printf("SAVING TO %s...\n", filename);
  char* mode = "w";
  FILE* stream = fopen(filename, mode);
  if (stream== NULL) {
        perror("Error opening file");
        fprintf(stderr, "Failed to open file: %s in mode: %s\n", filename, mode);
        exit(EXIT_FAILURE);
  }
  char string[2048];
  gmp_sprintf(string, "%Zd\n%Zd\n",n, exp); 
  fprintf(stream,"%s", string);
  fclose(stream);
  if (verbose) printf("Success!\n", filename);
}

void rsa_load_key(mpz_t n, mpz_t exp, char* filename, int verbose) {
  if (verbose) printf("LOADING %s...\n", filename);
  char* mode = "r";
  FILE* stream = fopen(filename, mode);
  if (stream== NULL) {
        // Print detailed error message
        perror("Error opening file");
        fprintf(stderr, "Failed to open file: %s in mode: %s\n", filename, mode);
        exit(EXIT_FAILURE);
    }
  size_t size = 300;
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
  mpz_t n, e, c, m;
  mpz_inits(n,e, c, m,NULL);
  
  rsa_load_key(n, e, pubKeyFile, verbose);
  
  mpz_set_ui(m, 0);
  mpz_t chunk, local_c; 
  mpz_inits(chunk, local_c, NULL);
  char* ci = (char*)malloc(1000*sizeof(char)); 
  ci[0] = '\0';
  int chunkSize = 32; // in bytes. so 16B = 128b 
  for (int j=0; (j < (size/(chunkSize))+1) ;j++)
  {
    // uint64_t chunk = 0;
    mpz_set_ui(chunk, 0);
    for (int i=0; (i < chunkSize); i++) {
      // chunk = (chunk << 8) | (unsigned char)message[i];
      mpz_mul_2exp(chunk, chunk, 8); // equal to chunk = chunk << 8
      if (i < size)
        mpz_add_ui(chunk, chunk, (unsigned char)message[i+(j*8)]);

    if (verbose) gmp_printf("chunk%d:\t%16.16Zx\n",i, chunk);
    }
    

    fast_power_mod(local_c, chunk, e, n);
    gmp_printf("local_c: %Zx\n",local_c);
    char temp_ci[200];
    gmp_sprintf(temp_ci, "%Zx|", local_c);
    char *tmp = realloc(ci, sizeof(char) * (strlen(ci) + 200));
    strcat(ci, temp_ci);
    ci = tmp;
                 
    mpz_add(c, c, local_c);
    mpz_mul_2exp(c,c,64);

  }
  gmp_printf("ciphertext: %Zx\n", c); 
  gmp_printf("ci: %s\n", ci); 
  mpz_clears(n,e, chunk, m,c, local_c,NULL);
  free(ci);
}
void rsa_decode(char* ciphertext, size_t size, char* priKeyFile, int verbose) {


      // fast_power_mod(n, e, ten, ten);
  mpz_t n, d, c, m;
  mpz_inits(n,d, c, m,NULL);
  
  rsa_load_key(n, d, priKeyFile, verbose);
  // char* str_chunk = (char*)malloc();
  // getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict stream)
  mpz_set_str(m, ciphertext, 10);
  gmp_printf("ciphertext: %Zd\n", m);
  //
  // mpz_set_ui(c, 0);
  // gmp_printf("octal c: %Zo\n", c);
  // mpz_t chunk, local_m; 
  // mpz_inits(chunk, local_m, NULL);
  // for (int j=0; (j < (size/8)+1) ;j++)
  // {
  //   // uint64_t chunk = 0;
  //   mpz_set_ui(chunk, 0);
  //   for (int i=0; (i < 8); i++) {
  //     // chunk = (chunk << 8) | (unsigned char)message[i];
  //     mpz_mul_2exp(chunk, chunk, 8); // equal to chunk = chunk << 8
  //     if (i < size)
  //       mpz_add_ui(chunk, chunk, (unsigned char)message[i]);
  //
  //   gmp_printf("chunk%d:\t%16.16Zx\n",i, chunk);
  //   }
  //
  //
  //   fast_power_mod(local_c, chunk, e, n);
  //   mpz_add(c, c, local_c);
  //   // mpz_mul_ui(c,c, 18446744073709551616);
  //   mpz_mul_2exp(c,c,64);
  //   gmp_printf("oct m:\t%Zd\n", c);
  //
  // }
  // gmp_printf("ciphertext: %8.8Zd\n", c); 
  // mpz_clears(n,d, chunk, m,c, local_m,NULL);
  mpz_clears(n,d,m,c, NULL);
}

