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
// const uint64_t H512[] = {0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1, 0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179};
uint64_t H512[] ={ 0x6A09E667F3BCC908, 0xBB67AE8584CAA73B,0x3C6EF372FE94F82B, 0xA54FF53A5F1D36F1, 0x510E527FADE682D1, 0x9B05688C2B3E6C1F,0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179};


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

const uint64_t K[80] =
{
    0x428A2F98D728AE22, 0x7137449123EF65CD, 0xB5C0FBCFEC4D3B2F, 0xE9B5DBA58189DBBC,
    0x3956C25BF348B538, 0x59F111F1B605D019, 0x923F82A4AF194F9B, 0xAB1C5ED5DA6D8118,
    0xD807AA98A3030242, 0x12835B0145706FBE, 0x243185BE4EE4B28C, 0x550C7DC3D5FFB4E2,
    0x72BE5D74F27B896F, 0x80DEB1FE3B1696B1, 0x9BDC06A725C71235, 0xC19BF174CF692694,
    0xE49B69C19EF14AD2, 0xEFBE4786384F25E3, 0x0FC19DC68B8CD5B5, 0x240CA1CC77AC9C65,
    0x2DE92C6F592B0275, 0x4A7484AA6EA6E483, 0x5CB0A9DCBD41FBD4, 0x76F988DA831153B5,
    0x983E5152EE66DFAB, 0xA831C66D2DB43210, 0xB00327C898FB213F, 0xBF597FC7BEEF0EE4,
    0xC6E00BF33DA88FC2, 0xD5A79147930AA725, 0x06CA6351E003826F, 0x142929670A0E6E70,
    0x27B70A8546D22FFC, 0x2E1B21385C26C926, 0x4D2C6DFC5AC42AED, 0x53380D139D95B3DF,
    0x650A73548BAF63DE, 0x766A0ABB3C77B2A8, 0x81C2C92E47EDAEE6, 0x92722C851482353B,
    0xA2BFE8A14CF10364, 0xA81A664BBC423001, 0xC24B8B70D0F89791, 0xC76C51A30654BE30,
    0xD192E819D6EF5218, 0xD69906245565A910, 0xF40E35855771202A, 0x106AA07032BBD1B8,
    0x19A4C116B8D2D0C8, 0x1E376C085141AB53, 0x2748774CDF8EEB99, 0x34B0BCB5E19B48A8,
    0x391C0CB3C5C95A63, 0x4ED8AA4AE3418ACB, 0x5B9CCA4F7763E373, 0x682E6FF3D6B2B8A3,
    0x748F82EE5DEFB2FC, 0x78A5636F43172F60, 0x84C87814A1F0AB72, 0x8CC702081A6439EC,
    0x90BEFFFA23631E28, 0xA4506CEBDE82BDE9, 0xBEF9A3F7B2C67915, 0xC67178F2E372532B,
    0xCA273ECEEA26619C, 0xD186B8C721C0C207, 0xEADA7DD6CDE0EB1E, 0xF57D4F7FEE6ED178,
    0x06F067AA72176FBA, 0x0A637DC5A2C898A6, 0x113F9804BEF90DAE, 0x1B710B35131C471B,
    0x28DB77F523047D84, 0x32CAAB7B40C72493, 0x3C9EBE0A15C9BEBC, 0x431D67C49C100D4C,
    0x4CC5D4BECB3E42B6, 0x597F299CFC657E2A, 0x5FCB6FAB3AD6FAEC, 0x6C44198C4A475817
 }; 

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
    memcpy(new_data + len + 1 + k, len_bytes, len_size);
    
    // Debug print
    for (int i = 0; i < len + 1 + k + len_size; i++) {
        printf("byte[%d]=%02x\t\t", i, new_data[i]);
    }
    
    printf("\nk:%d, l:%d, total: %d\n", k, len, len+1+k+len_size);
    printf("bitlen: %d, lensize: %d\n", bit_len, len_size);
}


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

uint32_t Ch32(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (~x & z);
}
uint64_t Ch64(uint64_t x, uint64_t y, uint64_t z) {
  return (x & y) ^ (~x & z);
}

#include <assert.h>

void test_rotations() {
    // Тест для ROTR64
    assert(ROTR64(0x8000000000000000, 1) == 0x4000000000000000);
    assert(ROTR64(0x0000000000000001, 63) == 0x0000000000000002);
    
    // Тест для s0_64
    assert(s0_64(0x123456789ABCDEF0) == 0x2468ACF13579BDE1); // Примерное значение
    
    // Тест для s1_64
    assert(s1_64(0xFEDCBA9876543210) == 0x7F6E5D4C3B2A1908); // Примерное значение
    
    // Тест для S0_64
    assert(S0_64(0x123456789ABCDEF0) == 0x48D159E26AF37BC3); // Примерное значение
    
    // Тест для S1_64
    assert(S1_64(0xFEDCBA9876543210) == 0x7B5D4C3A291806F4); // Примерное значение
    
    printf("All rotation tests passed!\n");
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

uint8_t* sha256(void **data, uint64_t len, void** out_hash) {

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
    Pad((void**)&msg, (uint64_t)strlen(msg), 256);
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
