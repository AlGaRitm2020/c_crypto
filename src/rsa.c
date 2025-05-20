#include "essential_func.h"
#include "rsa.h"
// #include <cstdint>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


#define CHUNKSIZE 32 
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

void pkcs7_pad(uint8_t *data, size_t data_len, uint8_t* padded_data, size_t* total_len, size_t BLOCKSIZE) {
  uint8_t pad_len = BLOCKSIZE - (data_len % BLOCKSIZE);
  *total_len = data_len + pad_len;
  memcpy(padded_data, data, data_len);  
  memset(padded_data+data_len, pad_len, pad_len);  
 
}

int pkcs7_unpad(uint8_t* padded_data, size_t total_len, uint8_t* data, size_t* data_len) {
  if (total_len == 0) return 1;

  uint8_t pad_len = padded_data[total_len-1]; 
  *data_len = total_len - pad_len; 
  for (int i=*data_len ; i < total_len; i++) {
    if (padded_data[i] != pad_len) return 1; 
  } 

  memcpy(data, padded_data, *data_len);
  return 0;

}
void rsa_encode(char* message, size_t size, char* pubKeyFile, char** enc_message, size_t* enc_message_len, int verbose) {
      // fast_power_mod(n, e, ten, ten);
  mpz_t n, e, c, m;
  mpz_inits(n,e, c, m,NULL);
  
  rsa_load_key(n, e, pubKeyFile, verbose);
  // size_t* total_len = NULL;
  size_t total_len;
  uint8_t* padded_message = (uint8_t*)malloc( (size/CHUNKSIZE+1)*CHUNKSIZE); 

  pkcs7_pad((uint8_t*)message, size, padded_message, &total_len, (size_t) CHUNKSIZE);
  
    printf("\npadded_message: "); 
  for (int i = 0; i < total_len;i++) {
    printf("%02x",padded_message[i]); 
  }
    printf("\n"); 

  mpz_set_ui(m, 0);
  mpz_t chunk, local_c; 
  mpz_inits(chunk, local_c, NULL);
  char* ci = (char*)malloc(2000*sizeof(char)); 
  ci[0] = '\0';
  // int CHUNKSIZE = 8; // in bytes. so 16B = 128b 
  for (int j=0; j < (total_len/CHUNKSIZE) ;j++)
  {
    // uint64_t chunk = 0;
    mpz_set_ui(chunk, 0);
    int remainder = 0xfa;
    for (int i=0; (i < CHUNKSIZE); i++) {
      // chunk = (chunk << 8) | (unsigned char)message[i];
      mpz_mul_2exp(chunk, chunk, 8); // equal to chunk = chunk << 
      if (i < total_len) {
        mpz_add_ui(chunk, chunk, (unsigned char)padded_message[i+(j*CHUNKSIZE)]);
        remainder--;
     }
      
      // else
        // mpz_add_ui(chunk, chunk, remainder);

    // if (verbose) gmp_printf("chunk%d:\t%16.16Zx\n",i, chunk);
    }
    if (verbose) gmp_printf("chunk%d:\t%16.16Zx\n",j, chunk);
    

    fast_power_mod(local_c, chunk, e, n);
    gmp_printf("local_c: %Zx\n",local_c);
    char temp_ci[2000];
    gmp_sprintf(temp_ci, "%Zxg", local_c);
    gmp_printf("temp ci: %s\n",temp_ci);
    char *tmp = realloc(ci, sizeof(char) * (strlen(ci) + 200));
    ci = tmp;
    strcat(ci, temp_ci);
                 
    mpz_add(c, c, local_c);
    mpz_mul_2exp(c,c,64);

  }
  // gmp_printf("ciphertext: %Zx\n", c); 
  gmp_printf("ci: %s\n", ci); 
  *enc_message = (char*)realloc(*enc_message, strlen(ci)+1);
  strcpy(*enc_message , ci);
  *enc_message_len = strlen(ci);

  mpz_clears(n,e, chunk, m,c, local_c,NULL);
  free(ci);
  free(padded_message);
}

void rsa_decode(char* ciphertext, size_t size, char* priKeyFile,char** dec_message, size_t* dec_message_len, int verbose){ 

      // fast_power_mod(n, e, ten, ten);
  mpz_t n, d, c, m;
  mpz_inits(n,d, c, m,NULL);
  
  rsa_load_key(n, d, priKeyFile, verbose);
  
  const char delim[] = "g"; 
  int cnt=0;
  for(int i=0; i< size; i++)
    if (ciphertext[i] == delim[0]) cnt++; 

  // getdelim(&str_chunk, &lineSize, int delimiter, FILE *__restrict stream)
  // mpz_set_str(m, ciphertext, 10);
  // gmp_printf("ciphertext: %Zd\n", m);
  printf("number of chunks: %d\n", cnt);

  //tokens = (char **)malloc(cnt * sizeof(char *));
  int i = 0;
  char* token;
  token = strtok(ciphertext, delim);

  char message[1000];
  size_t paded_size=0;
  while (token != NULL) {
        if(verbose) printf("token%d: %s\n",i, token);

        mpz_t chunk_c, chunk_m;
        mpz_inits(chunk_c, chunk_m, NULL);
        mpz_set_str(chunk_c, token, 16);
        mpz_set_ui(chunk_m, 0);

        gmp_printf("chunk[%d] encrypted: %Zx\n", i, chunk_c);
        if(verbose)fast_power_mod(chunk_m, chunk_c, d, n);
        gmp_printf("chunk[%d] decrypted: %Zx\n", i, chunk_m);

    
        char symbol; 
        mpz_t single_char_mpz;
        mpz_init(single_char_mpz);
        gmp_printf("cx\n");

        token = strtok(NULL, delim);
        if (token == NULL && mpz_cmp_ui(chunk_m, 0) == 0)
          continue;
     
        for (int j=0; j <  CHUNKSIZE ;j++ ){
          mpz_div_2exp(single_char_mpz, chunk_m, ((CHUNKSIZE-1)- j)*8);
          mpz_mod_ui(single_char_mpz, single_char_mpz, 256);
          if(verbose)gmp_printf("single char[%d]: 0x%02Zx\n", j, single_char_mpz);
          if(verbose)gmp_printf("single char[%d]: %Zc\n", j, single_char_mpz);
          paded_size++;
          message[j+ (i*CHUNKSIZE)] = (unsigned char)mpz_get_ui(single_char_mpz);
        // gmp_printf("cy\n");
        }
        if(verbose) printf("message: %s\n", message);
       mpz_clear(single_char_mpz);
        
       i++;
       mpz_clears(chunk_m, chunk_c, NULL);
        
    }
        uint8_t* unpaded = (uint8_t*)malloc(paded_size); 
        size_t data_len;
        printf("sizeof_padded = %d or %d?\n", sizeof(message), paded_size);
        pkcs7_unpad((uint8_t*)message,paded_size, unpaded, &data_len); 
        printf("UNPADDED : %s\n", (char*)unpaded);

  *dec_message = (char*)realloc(*dec_message, sizeof(unpaded));     
  strcpy(*dec_message, (char*)unpaded);
  *dec_message_len = strlen((char*)unpaded);

        

  mpz_clears(n,d,m,c, NULL);
}

