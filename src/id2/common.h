#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <gmp.h> // Добавлено для лабы 15
#include "../essential_func.h"

#define PORT_B 8081
#define KEY_SIZE 16
#define MAX_BUF 1024

// Для лаб 11-14
void print_hex_arr(uint8_t* arr, size_t size, char* msg); 
void handle_error(const char *msg);
void generate_key(uint8_t key[KEY_SIZE]);
void aes_encrypt(const uint8_t *plaintext, int plaintext_len,
                 const uint8_t *key, uint8_t *ciphertext, int *ciphertext_len);
int aes_decrypt(const uint8_t *ciphertext, int ciphertext_len,
                const uint8_t *key, uint8_t *plaintext);
void hex_dump(const char *prefix, const void *data, size_t len);
// Для лабы 15 (Фиат-Шамир)
#define PORT_TRUSTED 8080
#define PORT_PROVER 8081
#define PORT_VERIFIER 8082

typedef struct {
    int socket;
    struct sockaddr_in address;
} connection_t;

int create_server_socket(int port);
int connect_to(const char *ip, int port);
void send_mpz(int socket, const mpz_t num);
void recv_mpz(int socket, mpz_t num);
// Размер поля GF(p)
//
// Добавить в common.h
#define PORT_TC 8080   // Порт доверенного центра
#define PORT_USER_A 8081
#define PORT_USER_B 8082

typedef struct {
    mpz_t a;
    mpz_t b;
    mpz_t c;
} BlomSecrets;

typedef struct {
    mpz_t ai;
    mpz_t bi;
} UserKey;

void generate_blom_secrets(BlomSecrets *secrets, mpz_t p);
void compute_user_key(UserKey *user_key, mpz_t xi, BlomSecrets *secrets, mpz_t p);
#endif
