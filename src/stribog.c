#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "stribog_consts.h"
#define BLOCK_SIZE 64
#define HASH_SIZE 64

typedef uint8_t block[BLOCK_SIZE];
typedef uint8_t vect[BLOCK_SIZE];

// Константы из stribog_consts.h

void X(uint8_t *c, const uint8_t *a, const uint8_t *b) { // XOR
    for(int i = 0; i < 64; i++)
        c[i] = a[i] ^ b[i];
}

void Add512(uint8_t *c, const uint8_t *a, const uint8_t *b) { // 
    int sum = 0;
    for(int i = 0; i < 64; i++) {
        sum = a[i] + b[i] + (sum >> 8);
        c[i] = sum & 0xff;
    }
}

void S(uint8_t *state) { // substitution - подстановка
    block result;
    for(int i = 0; i < 64; i++)
        result[i] = Pi[state[i]];
    memcpy(state, result, BLOCK_SIZE);
}

void P(uint8_t *state) { // permutatin -  перестановка 
    block result;
    for(int i = 0; i < 64; i++)
        result[i] = state[Tau[i]];
    memcpy(state, result, BLOCK_SIZE);
}

void L(uint8_t *state) { // linear  - линейное преобразование
    uint64_t* input = (uint64_t*)state;
    uint64_t result[8] = {0};
    
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 64; j++) {
            if((input[i] >> j) & 1)
                result[i] ^= A[63 - j]; // единичные биты ксорим с константой
        }
    }
    memcpy(state, result, BLOCK_SIZE);
}

typedef struct HashContext {
    block buffer; // текущий
    block hash; // теукщий
    block h; // состояние
    block N; // длина обработанных данных
    block Sigma; // контрольная сумма (XOR)  
    block v_0;  // нулевой блок
    block v_512; // нулевой блок с единицей в конце
    size_t buf_size; //размер буфера
    int hash_size; // 256 или 512
} THashContext;

void GetRoundKey(uint8_t *K, int i) {
    X(K, K, C[i]); // against known plaintext attack
    S(K); // against linear cryptoanalys
    P(K); // against local diffusion attack
    L(K); // 
}

void E(uint8_t *K, const uint8_t *m, uint8_t *state) {
    X(state, m, K);
    for(int i = 0; i < 12; i++) {
        S(state);
        P(state);
        L(state);
        GetRoundKey(K, i);
        X(state, state, K);
    }
}

void G(uint8_t *h, uint8_t *N, const uint8_t *m) {
    block K, result;
    X(K, N, h);

    S(K);
    P(K);
    L(K);

    E(K, m, result);

    X(result, result, h);
    X(h, result, m);
}

// void Padding(THashContext *CTX) {
//     if(CTX->buf_size < BLOCK_SIZE) {
//         memset(CTX->buffer + CTX->buf_size, 0, BLOCK_SIZE - CTX->buf_size - 1);
//         CTX->buffer[CTX->buf_size] = 0x01;
//     }
// }

void Padding(THashContext *CTX)
{
    block internal;

    if (CTX->buf_size < BLOCK_SIZE)
    {
        memset(internal, 0x00, BLOCK_SIZE);
        memcpy(internal, CTX->buffer, CTX->buf_size);
        // memcpy(internal - CTX->buf_size, CTX->buffer, CTX->buf_size);
        internal[CTX->buf_size] = 0x01;
        memcpy(CTX->buffer, internal, BLOCK_SIZE);
    }
}

void Init(THashContext *CTX, uint16_t hash_size) {
    memset(CTX, 0, sizeof(THashContext));
    memset(CTX->h, (hash_size == 256) ? 0x01 : 0x00, BLOCK_SIZE);
    CTX->v_512[1] = 0x02; // 0x0200 = 512
    CTX->hash_size = hash_size;
}

void Stage2(THashContext *CTX, const uint8_t *data) {
    G(CTX->h, CTX->N, data);
    Add512(CTX->N, CTX->N, CTX->v_512);
    Add512(CTX->Sigma, CTX->Sigma, data);
}
void Padding(THashContext *CTX); 
void Stage3(THashContext *CTX) {
    block remainder = {0};
    remainder[0] = (CTX->buf_size * 8) & 0xff;
    remainder[1] = ((CTX->buf_size * 8) >> 8) & 0xff;

    Padding(CTX);
    G(CTX->h, CTX->N, CTX->buffer);

    Add512(CTX->N, CTX->N, remainder);
    Add512(CTX->Sigma, CTX->Sigma, CTX->buffer);

    G(CTX->h, CTX->v_0, CTX->N);
    G(CTX->h, CTX->v_0, CTX->Sigma);

    memcpy(CTX->hash, CTX->h, BLOCK_SIZE);
}


// void Padding(THashContext *CTX) {
//     if (CTX->buf_size < BLOCK_SIZE) { // неполный буфер
//         memset(CTX->buffer + CTX->buf_size, 0, BLOCK_SIZE - CTX->buf_size - 1);
//         CTX->buffer[CTX->buf_size] = 0x01;
//     }
//     else if (CTX->buf_size == BLOCK_SIZE) { // полный буфер - создаем пустой блок
//         Stage2(CTX, CTX->buffer);
//         CTX->buf_size = 0;
//
//         memset(CTX->buffer, 0, BLOCK_SIZE);
//         CTX->buffer[0] = 0x01;
//     }
// }


 
void Update(THashContext *CTX, const uint8_t *data, size_t len)
{
    size_t chk_size;

    // while((len > 63) && (CTX->buf_size) == 0)
    // {
    //     Stage2(CTX, data);
    //     data += 64;
    //     len -= 64;
    // }
    while (len)
    {
        chk_size = 64 - CTX->buf_size;
        if (chk_size > len)
            chk_size = len;
        memcpy(&CTX->buffer[CTX->buf_size], data, chk_size);
        CTX->buf_size += chk_size;
        len -= chk_size;
        // data += chk_size;
        if (CTX->buf_size == 64)
        {
            Stage2(CTX, CTX->buffer);
            CTX->buf_size = 0;
        }
    }
}

void Final(THashContext *CTX) {
    Stage3(CTX);
    CTX->buf_size = 0;
}

void HashPrint(THashContext *CTX) {
    printf("%d bit hash: ", CTX->hash_size);
    int start = (CTX->hash_size == 256) ? 32 : 0;
    for(int i = start; i < 64; i++)
        printf("%02x", CTX->hash[i]);
    printf("\n");
}

int main() {
    THashContext ctx;
    // const char *msg = "hello";
    char *msg;  
    size_t size = 32;
    getline(&msg, &size, stdin);

    Init(&ctx, 256);
    Update(&ctx, (uint8_t*)msg, strlen(msg));
    Final(&ctx);
    HashPrint(&ctx);
    
    Init(&ctx, 512);
    Update(&ctx, (uint8_t*)msg, strlen(msg));
    Final(&ctx);
    HashPrint(&ctx);
    
    return 0;
}
