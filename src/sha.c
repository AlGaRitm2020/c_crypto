#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stribog_consts.h"
#include <inttypes.h>
#define BLOCK_SIZE 64
#define HASH_SIZE 64


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
        printf("byte[%d]=%02x\n", i, new_data[i]);
    }
    
    printf("k:%d, l:%d, total: %d\n", k, len, len+1+k+len_size);
    printf("bitlen: %d, lensize: %d\n", bit_len, len_size);
}

int main() {
    // THashContext ctx;
    char *msg = strdup("hellochar *msg = strdup(char *msg = strdup(char *msg = strdup(hellochar *msg = strdup(char *msg = strdup(char *msg = st");
    // char *msg;  
    size_t size = 32;
    // getline(&msg, &size, stdin);

    // printf("%d\n", calc_n_of_zeroes(strlen(msg)));  
    Pad((void**)&msg, (uint64_t)strlen(msg), 256);
    return 0;
}
