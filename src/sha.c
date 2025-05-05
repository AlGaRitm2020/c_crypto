#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "stribog_consts.h"
#include <inttypes.h>
#define BLOCK_SIZE 64
#define HASH_SIZE 64

uint32_t H256[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
uint64_t H512[] = {0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1, 0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};


static const uint32_t k[] = {
			    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
			    0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
			    0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
			    0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
			    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
			    0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
			    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
			    0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
			    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
			    0xc67178f2};


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
    printf("debug1\n");
    
    // Calculate padding bytes needed
    int k = calc_n_of_zeroes(len, hashsize);
    
    
    // Resize the buffer (old size: len, new size: len + 1 + k + 8)
    int len_size;
    if (hashsize == 256)
      len_size = 8; 
    else
      len_size = 16; 

    uint8_t* new_data = realloc(*data, len + 1 + k + len_size);
    if (!new_data) {
        perror("realloc failed");
        exit(EXIT_FAILURE);
    }
    
    *data = new_data; // Update caller's pointer
    
    // Add padding
    memset(new_data + len, 0x80, 1);        // 1-byte 0x80
    memset(new_data + len + 1, 0x00, k);    // k zero bytes
    
    // Append original message length (in bits, big-endian)
    uint64_t bit_len = len * 8;
    uint8_t len_bytes[16];
    write_len(len_bytes, bit_len);
    uint64_t be_bit_len = htobe64(bit_len); // Convert to big-endian
    memcpy(new_data + len + 1 + k, &len_bytes, len_size);
    
    // Debug print
    for (int i = 0; i < len + 1 + k + len_size; i++) {
        printf("byte[%d]=%02x\t\t", i, new_data[i]);
    }
    
    printf("\nk:%d, l:%d, total: %d\n", k, len, len+1+k+len_size);
    printf("bitlen: %d, lensize: %d\n", bit_len, len_size);
}

typedef uint8_t vec512[512];
typedef uint8_t vec1024[1024];

uint32_t ROTR32(uint32_t x, int n) {
  return (x >> n) | (x << (32-n)); 
}
uint64_t ROTR64(uint64_t x, int n) {
  return (x >> n) | (x << (64-n)); 
}

uint32_t s0_32(uint32_t w) {
  if (w == 0x6c6f636b) {
    printf("\norig: %32.32b\nRot7: %32.32b,\nrot18: %32.32b\nsh3: %32.32b\n",w,ROTR32(w, 7) , ROTR32(w, 18) , (w >> 3)); 
  
    printf("\norig: %8.8x\nRot7: %8.8x,\nrot18: %8.8x\nsh3: %8.8x\n",w,ROTR32(w, 7) , ROTR32(w, 18) , (w >> 3)); 
  
  }
  return ROTR32(w, 7) ^ ROTR32(w, 18) ^ (w >> 3);  
}
uint32_t s1_32(uint32_t w) {
  return ROTR32(w, 17) ^ ROTR32(w, 19) ^ (w >> 10);  
}
uint64_t s0_64(uint64_t w) {
  return ROTR64(w, 1) ^ ROTR64(w, 8) ^ (w >> 7);  
}
uint64_t s1_64(uint64_t w) {
  return ROTR64(w, 19) ^ ROTR64(w, 61) ^ (w >> 6);  
}

uint32_t S0_32(uint32_t w) {
  
  return ROTR32(w, 2) ^ ROTR32(w, 13) ^ ROTR32(w, 22) ;  
}
uint32_t S1_32(uint32_t w) {
  return ROTR32(w, 6) ^ ROTR32(w, 11) ^  ROTR32(w, 25);  
}
uint64_t S0_64(uint64_t w) {
  return ROTR64(w, 28) ^ ROTR64(w, 34) ^ ROTR64(w,39);  
}
uint64_t S1_64(uint64_t w) {
  return ROTR64(w, 14) ^ ROTR64(w, 18) ^ ROTR64(w, 41);  
}

// uint32_t Ch(uint32_t x,uint32_t y, uint32_t z) {
//   return (x & y) ^ (~x & z);
// }

uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (~x & z);
}

// #define Ch(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

void print_hashes(uint32_t* H, uint32_t T1, uint32_t T2, uint32_t ch, uint32_t maj, uint32_t s0, uint32_t s1, uint32_t k, uint32_t w, int stage) {
  char* alpha = "ABCDEFGH"; 
  printf("STAGE %d\n");
    printf("S1 = %32.32b\n",s1); 
    printf("S0 = %32.32b\n",s0); 
    printf("ch = %32.32b\n",ch); 
    printf("maj= %32.32b\n",maj); 
    printf("k  = %32.32b\n",k); 
    printf("w  = %32.32b\n",w); 
    printf("T1 = %32.32b\n",T1); 
    printf("T2 = %32.32b\n",T2); 
  for (int i=0 ; i < 8 ; i++) {
    printf("%c = %32.32b\n", alpha[i], H[i]); 
  }
}

void parse(void **data, uint64_t len, int hashsize ) {
  // if (hashsize == 256) 
  //   typedef uint8_t block[512];
  // else
  //   typedef uint8_t block[1024];
  //
  // block 
  int step = (hashsize==256) ? 64: 128; // step in data pointer (for block dividing)
  uint8_t* block = (uint8_t*)(*data);
  uint32_t* w = (uint32_t*)malloc(sizeof(uint32_t) * step);
  assert(ROTR32(0x80000000, 1) == 0x40000000);
  assert(ROTR32(0x00000001, 31) == 0x00000002);

  uint32_t* H = H256;
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
      printf("W[%d]: %8.8x\t\t\t",t, w[t]);
      
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
      uint32_t T1 = h + S1_32(e) + Ch(e,f,g) + k[t] + w[t];  
      uint32_t T2 = S0_32(a) + Maj(a,b,c);  
      if (t < 3333) {
    printf("STAGE %d\n", t);
    printf("S1 = %32.32b\n",S1_32(e)); 
    printf("S0 = %32.32b\n",S0_32(a)); 
    printf("ch = %32.32b\n",Ch(e,f,g)); 
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
  for(int i=0; i < 8; i++)
    printf("%8.8x", H[i]);
  printf("\n");
  
  free(w);
} 
int main() {
    // THashContext ctx;
    char *msg = strdup("RedBlockBlue");
    // char *msg = strdup("abc");
    // char *msg;  
    size_t size = 32;
    // getline(&msg, &size, stdin);

    // printf("%d\n", calc_n_of_zeroes(strlen(msg)));  
    Pad((void**)&msg, (uint64_t)strlen(msg), 256);
    parse((void**)&msg, (uint64_t)strlen(msg), 256);
    return 0;
}
