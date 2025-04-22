#include "essential_func.h"
#include "rsa.h"
#include <cstdint>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>
#include "stribog_consts.h"

// Constants for Stribog
#define BLOCK_SIZE 64 // 512 bits = 64 bytes
#define HASH_SIZE 64  // 512 bits = 64 bytes
//

typedef uint8_t block[BLOCK_SIZE];

void X(uint8_t *c, const uint8_t *a, const uint8_t *b) { // XOR двух 
  for( int i =0; i < 64; i++)
    c[i] = a[i] ^ b[i];
}

void Add512(uint8_t *c, const uint8_t *a, const uint8_t *b) {
  for (int i=0,sum=0; i < 64; i++) {
    sum = a[i] + b[i] + ( sum >> 8); //(sum>>8) это переполнение предыдущих сложений
    c[i] = sum & 0xff; // отбрасываем переполнение в текущей итерации
  }  
}

void S(uint8_t *state) { // substitution - подстановка
  block result;
  for (int i = 63; i >=0; i--) 
    result[i] = Pi[state[i]];
  memcpy(state, result, BLOCK_SIZE);
}

void P(uint8_t *state) { // permutation - перестановка
  block result;
  for (int i = 63; i >=0; i--) 
    result[i] = Tau[state[i]]; // транспонирование
  memcpy(state, result, BLOCK_SIZE);
}

void L(uint8_t *state) {
  uint64_t* input = (uint64_t*)state; // вектор 64 элементов по байту -> 8 элементов по 8 байт 
  uint64_t result[8]; 
  memset(result, 0x00, BLOCK_SIZE); // ?? вместо BLOCK_SIZE должно быть 8 
  for (int i =7; i >= 0; i--){
    for (j = 63; j >=0; j--) {
      if ((input[i] >>j) & 1) // если j-ый бит в блоке равен 1, то ксорим с A[63-j] 
        result[i] ^= A[63-j];
    }


  }

  memcpy(state, result, 64); // копируем 64 байта в state

}




