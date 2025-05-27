// common.h
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

#define PORT_A 65432
#define PORT_B 65433
#define BUFFER_SIZE 1024
#define AES_KEY_SIZE 16
#define FRESHNESS_SIZE 16

// Shared AES key (in a real application, this would be securely exchanged)
static const unsigned char aes_key[AES_KEY_SIZE] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

typedef enum {
    FRESHNESS_TIMESTAMP,
    FRESHNESS_RANDOM
} freshness_type;

typedef struct {
    freshness_type type;
    union {
        time_t timestamp;
        unsigned char random[FRESHNESS_SIZE];
    } value;
} freshness_t;

typedef struct {
    freshness_t freshness;
    char identity;
    char message[BUFFER_SIZE];
} protocol_message;

void generate_freshness(freshness_t *freshness, int use_timestamp);
int verify_freshness(const freshness_t *freshness, int max_delay);
void encrypt_message(const protocol_message *msg, unsigned char *iv, unsigned char *ciphertext);
int decrypt_message(protocol_message *msg, const unsigned char *iv, const unsigned char *ciphertext);

#endif // COMMON_H
