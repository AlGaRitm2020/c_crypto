#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <inttypes.h>
#include "sha.h"


int calc_n_of_zeroes(int len, int hashsize) {
  if (hashsize == 256) {
    int k = (448/8) - len - 1;  
    while (k < 0)
      k += 512/8; 
    return k;
  }
  else {
    int k = (896/8) - len - 1;  
    while (k < 0)
      k += 1024/8; 
    return k;

  }
}
// write len to array (big endian)
void write_len(uint8_t* dest, uint64_t len_bits) {
  for (int i=0 ; i < 16; i++) {
    dest[i]= (len_bits >> (8 * (15 - i))) & 0xFF;
  }
}

void Pad(void** data, uint64_t len, int hashsize) {
    // Calculate padding bytes needed
    int k = calc_n_of_zeroes(len, hashsize);
    
    // Resize the buffer (old size: len, new size: len + 1 + k + 8)
    int len_size;
    if (hashsize == 256)
      len_size = 8; 
    else
      len_size = 16; 

    printf("T2trying to reallocate %d bytes", len + 1 + k + len_size);
    printf("trying to reallocate %d byteslen:%d, k:%d, len_size:%d", len + 1 + k + len_size, len, len, len);
    // uint8_t* new_data = realloc(*data, len + 1 + k + len_size);
    uint8_t* new_data = (uint8_t*)malloc(len + 1 + k + len_size);
    if (!new_data) {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }
    memcpy(new_data, *data, len); 
    
    *data = new_data; // Update caller's pointer
    
    // Add padding
    memset(new_data + len, 0x80, 1);        // 1-byte 0x80
    memset(new_data + len + 1, 0x00, k);    // k zero bytes
    
    // Append original message length (in bits, big-endian)
    uint64_t bit_len = len * 8;
    uint8_t len_bytes[16];
    write_len(len_bytes, bit_len);
    uint64_t be_bit_len = htobe64(bit_len); // Convert to big-endian
    memcpy(new_data + len + 1 + k, len_bytes, len_size);
    
    // Debug print
    for (int i = 0; i < len + 1 + k + len_size; i++) {
        printf("byte[%d]=%02x\t\t", i, new_data[i]);
    }
    
    printf("\nk:%d, l:%d, total: %d\n", k, len, len+1+k+len_size);
    printf("bitlen: %d, lensize: %d\n", bit_len, len_size);
}







#include <assert.h>


// #define Ch(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))

void sha256(void **data, uint64_t len, void** out_hash) {

  Pad(data, len, 256);
  // sha256((void**)&msg, (uint64_t)strlen(msg), &hash256);
 
  int step =  64; // step in data pointer (for block dividing)
  uint8_t* block = (uint8_t*)(*data);
  uint32_t* w = (uint32_t*)malloc(sizeof(uint32_t) * step);
  assert(ROTR32(0x80000000, 1) == 0x40000000);
  assert(ROTR32(0x00000001, 31) == 0x00000002);

  // uint32_t* H = (uint32_t*)malloc(sizeof(uint32_t) * 8);
  // H = H256;
  // memcpy(H, H256, 8);
  //////
  uint32_t H[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  for (int i = 0; i < (len/64)+1; i++ ) {
    printf("}\nblock %d: {\n", i);
    for (int t =0; t < step; t++) {
      uint32_t current_word; 
      if (t < 16)
      {
        w[t] = (block[0] << 24) + (block[1] << 16) + (block[2] << 8) + block[3];
        block+=4;
      }
      else 
        w[t] = s1_32(w[t-2]) + w[t-7] + s0_32(w[t-15]) + w[t-16];  
      // printf("W[%d]: %8.8x\t\t\t",t, w[t]);
      
    }
    uint32_t a,b,c,d,e,f,g,h;
    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];

    for (int t=0; t < 64; t++) {
      uint32_t T1 = h + S1_32(e) + Ch32(e,f,g) + k[t] + w[t];  
      uint32_t T2 = S0_32(a) + Maj(a,b,c);  
      if (t > 3333) {
    printf("STAGE %d\n", t);
    printf("S1 = %32.32b\n",S1_32(e)); 
    printf("S0 = %32.32b\n",S0_32(a)); 
    printf("ch = %32.32b\n",Ch32(e,f,g)); 
    printf("maj= %32.32b\n",Maj(a,b,c)); 
    printf("k  = %32.32b\n",k[t]); 
    printf("w  = %32.32b\n",w[t]); 
    printf("T1 = %32.32b\n",T1); 
    printf("T2 = %32.32b\n",T2);
    printf("a  = %32.32b\n",a); 
    printf("b  = %32.32b\n",b); 
    printf("c  = %32.32b\n",c); 
    printf("d  = %32.32b\n",d); 
    printf("e  = %32.32b\n",e); 
    printf("f  = %32.32b\n",f); 
    printf("g  = %32.32b\n",g); 
    printf("h  = %32.32b\n",h); 
      }
      h = g;
      g = f;
      f = e;
      e = d + T1;
      d = c;
      c = b; 
      b = a;
      a = T1 + T2;
    }
    H[0]+=a;
    H[1]+=b;
    H[2]+=c;
    H[3]+=d;
    H[4]+=e;
    H[5]+=f;
    H[6]+=g;
    H[7]+=h;
  
  }
  //result
  printf("\n\n Final sha256 hash is::::: ");
  if (*out_hash == NULL)
    *out_hash = malloc(32);
  
  uint32_t* hash_words = (uint32_t*)*out_hash;

  for(int i=0; i < 8; i++) {
    printf("%8.8x", H[i]);
    hash_words[i]= H[i];

  }
  printf("\n");
  
  free(w);
} 

void sha512(void **data, uint64_t len) {
  int step =  128; // step in data pointer (for block dividing)
  uint8_t* block = (uint8_t*)(*data);
  uint64_t* w = (uint64_t*)malloc(sizeof(uint64_t) * 80);

  // uint64_t H[8];
  // memcpy(H, H512, sizeof(H512));
uint64_t H[] ={ 0x6A09E667F3BCC908, 0xBB67AE8584CAA73B,0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1, 0x510E527FADE682D1, 0x9B05688C2B3E6C1F,0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179};
  for (int i = 0; i < (len/128)+1; i++ ) {
    printf("}\nblock %d: {\n", i);
    for (int t =0; t < 80; t++) {
      if (t < 16)
      {
        w[t] = ((uint64_t)block[0] << 56) | ((uint64_t)block[1] << 48) | ((uint64_t)block[2] << 40) | ((uint64_t)block[3] << 32) | ((uint64_t)block[4] << 24) | ((uint64_t)block[5] << 16) | ((uint64_t)block[6] << 8) | (uint64_t)block[7];
        // w[t] = block[0];
        block+=8;
      }
      else 
        w[t] = s1_64(w[t-2]) + w[t-7] + s0_64(w[t-15]) + w[t-16];  
      printf("W[%d]: %016llx\t\t\t",t, w[t]);
      
    }
    uint64_t a,b,c,d,e,f,g,h;
    a = H[0];
    b = H[1];
    c = H[2];
    d = H[3];
    e = H[4];
    f = H[5];
    g = H[6];
    h = H[7];

    for (int t=0; t < 80; t++) {
      uint64_t T1 = h + S1_64(e) + Ch64(e,f,g) + K[t] + w[t];  
      uint64_t T2 = S0_64(a) + Maj(a,b,c);  
      if (t > 3333) {
    printf("STAGE %d\n", t);
    printf("S1 = %016llx\n",S1_64(e)); 
    printf("S0 = %016llx\n",S0_64(a)); 
    printf("ch = %016llx\n",Ch64(e,f,g)); 
    printf("maj= %016llx\n",Maj(a,b,c)); 
    printf("k  = %016llx\n",K[t]); 
    printf("w  = %016llx\n",w[t]); 
    printf("T1 = %016llx\n",T1); 
    printf("T2 = %016llx\n",T2);
    printf("a  = %016llx\n",a); 
    printf("b  = %016llx\n",b); 
    printf("c  = %016llx\n",c); 
    printf("d  = %016llx\n",d); 
    printf("e  = %016llx\n",e); 
    printf("f  = %016llx\n",f); 
    printf("g  = %016llx\n",g); 
    printf("h  = %016llx\n",h); 
      }
      h = g;
      g = f;
      f = e;
      e = d + T1;
      d = c;
      c = b; 
      b = a;
      a = T1 + T2;
    }
    H[0]+=a;
    H[1]+=b;
    H[2]+=c;
    H[3]+=d;
    H[4]+=e;
    H[5]+=f;
    H[6]+=g;
    H[7]+=h;
  
  }
  //result
  printf("\n\n Final sha512 hash is::::: ");
  for(int i=0; i < 8; i++)
    printf("%016llx", H[i]);
  printf("\n");
  
  free(w);
} 
//
// void HMAC(uint8_t* key, int keylen, uint8_t* M) {
//   uint8_t K[32];
//   if (keylen > 256) {
//     Pad(key, keylen, 256);
//     sha256(key, keylen, (uint32_t)K);
//   }
//   return;
// }
//

void print_sha256(void** hash, char* message) {
  uint32_t* hasharr = (uint32_t*)*hash;
  printf("%s", message);
  for (int i=0; i < 8; i++) {
    printf("%x", hasharr[i]); 
  } 
  printf("\n");
}

#ifndef SHA_LIB
__attribute__((visibility("default"))) 
int main() {
    // THashContext ctx;
    char *msg = strdup("RedBlockBlue");
    char *msg2 = strdup("RedBlockBlue");
    // char *msg = strdup("abc");
    // char *msg;  
    size_t size = 32;
    // uint32_t hash256[8];
    void *hash256 = NULL;
    // getline(&msg, &size, stdin);

    // printf("%d\n", calc_n_of_zeroes(strlen(msg)));  
    // Pad((void**)&msg, (uint64_t)strlen(msg), 256);
    sha256((void**)&msg, (uint64_t)strlen(msg), &hash256);
    
    printf("\nmain func hash\n");
    print_sha256(&hash256, "this is hash in main: "); 
    
    // // test_rotations();
    // Pad((void**)&msg2, (uint64_t)strlen(msg2), 512);
    // sha512((void**)&msg2, (uint64_t)strlen(msg2));
    //
    free(msg);
    free(msg2);
    return 0;
}
#endif

