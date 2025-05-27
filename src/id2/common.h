// common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define PORT_B 8081
#define KEY_SIZE 16
#define MAX_BUF 1024

void handle_error(const char *msg);
void generate_key(uint8_t key[KEY_SIZE]);
void aes_encrypt(const uint8_t *plaintext, int plaintext_len,
                 const uint8_t *key, uint8_t *ciphertext, int *ciphertext_len);
int aes_decrypt(const uint8_t *ciphertext, int ciphertext_len,
                const uint8_t *key, uint8_t *plaintext);

void hex_dump(const char *prefix, const void *data, size_t len);

#endif
