#include "essential_func.h"
#include "rsa.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>


mpz_t rsa_encode(char* message, size_t size, mpz_t n, mpz_t e) {
  // extracting file
  // padding 128bit
   //
   //  mpz_t exp, mod;
   //  mpz_init(mod);
   //  mpz_init_set_ui(exp, 10);
   //
      int
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

