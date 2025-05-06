#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include <errno.h>
#include "sha.h"


void print_64bytes(uint8_t* data, char* message) {
  printf("%s", message);
  for (int i=0 ; i < 64 ; i++) {
    printf("0.2%x",data[i]);
  }
  printf("\n");
}

void HMAC(uint8_t* key, int keylen, uint8_t* m, int m_len, void** digest) {
  // K' creation (K is K', key is K in standard)
  const int B= 64;
  uint8_t* K = (uint8_t*)malloc(64);
  uint8_t* tempK = NULL; 
  if (keylen > 256) {
    // uint8_t* tempK = NULL; 
    
    uint8_t* key_working = (uint8_t*)malloc(keylen);
    memcpy(key_working, key, keylen);
    sha256((void**)&key_working, keylen, (void**)&tempK); // first 32 bytes is hash
    
    memcpy(K, tempK, 32); 
    memset(K+32, 0x00, 32);                   // last 32 bytes is zeroes
    
    free(tempK);
    free(key_working);
  }
  else {
    memcpy(K, key, keylen); 
    memset(K+keylen, 0x00, B - keylen);
  }


  // pad creation
  uint8_t K_x_ipad[B];
  uint8_t K_x_opad[B];
  memset(K_x_ipad, 0x36, B);
  memset(K_x_opad, 0x5c, B);
  print_64bytes(K_x_ipad, "ipad here: ");

  for (int i=0; i < B; i++){
    K_x_ipad[i] ^= K[i];
    K_x_opad[i] ^= K[i];
  }
  print_64bytes(K_x_ipad, "K XORed ipad here: ");

  printf("Attempting to allocate %zu bytes (m_len=%d, B=%d)\n", 
       (size_t)(m_len + B), m_len, B);

  uint8_t* ihash = NULL;
  uint8_t* imessage = (uint8_t *)malloc(m_len + B); 
  if (imessage == NULL)
    perror("malloc failed");

  memcpy(imessage, K_x_ipad, B);
  memcpy(imessage+64, m, m_len);

  sha256((void**)&imessage, m_len+B, (void **)&ihash);
  print_sha256((void**)&ihash, "this is inner hash: ");

  uint8_t* ohash = NULL;
  uint8_t* omessage = (uint8_t *)malloc(64 + 32); 
  if (imessage == NULL)
    perror("malloc failed");
  
  print_sha256((void**)&ihash, "IHASH");
  memcpy(omessage, K_x_opad, B);
  // memcpy(omessage+64, ihash, 32);
  
  //   WORD 0          WORD 1
  // 1f 47 36 78    4f 37 26 18  ................
  //         -/
  //  ------/
  // /
  // 78 36 47 1f    18 26 37 4f
  // 4-0-1 = 3 78
  // 4-1-1= 2  36
  //
  // i = 4, w = i / 4 + 1 = 2; 2 * 4 - 
  
  for (int i=0; i < 32; i++ ) {
    int word = i / 4 + 1;
    omessage[B+i] = ihash[word * 4  - (i%4) - 1];
    // omessage[B+i] = ihash[i];
  }
  sha256((void**)&omessage, 32+64, (void **)&ohash);
  print_sha256((void**)&ihash, "this is inner hash twice: ");
  print_sha256((void**)&ohash, "this is outer hash: ");
  
  uint8_t* digest_data = (uint8_t*)*digest;
  memcpy(digest_data, ohash, 32);

  // free(imessage);
  // print_sha256((void**)&K, "'hash me' string' hash inside hmac func: ");
  // free(K);
}

#ifndef HMAC_LIB
int main() {
    char* msg = "RedBlock";
    char* key = "hashme";
    void *hmac_sha256 = malloc(32);

     HMAC((uint8_t*)key, strlen(key), (uint8_t*)msg, strlen(msg), &hmac_sha256); 

     print_sha256(&hmac_sha256, "FFFfinal hash in HMAC main ");
    
    free(hmac_sha256);
    return 0;
}
#endif /* ifndef HMAC_LIB */
