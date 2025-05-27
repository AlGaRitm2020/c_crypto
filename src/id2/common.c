// common.c
#include "common.h"

void generate_freshness(freshness_t *freshness, int use_timestamp) {
    if (use_timestamp) {
        freshness->type = FRESHNESS_TIMESTAMP;
        freshness->value.timestamp = time(NULL);
    } else {
        freshness->type = FRESHNESS_RANDOM;
        RAND_bytes(freshness->value.random, FRESHNESS_SIZE);
    }
}

int verify_freshness(const freshness_t *freshness, int max_delay) {
    if (freshness->type == FRESHNESS_TIMESTAMP) {
        time_t current_time = time(NULL);
        return difftime(current_time, freshness->value.timestamp) <= max_delay;
    } else if (freshness->type == FRESHNESS_RANDOM) {
        // In a real implementation, we'd check against used random values
        return 1; // Accept all random values for this example
    }
    return 0;
}

void encrypt_message(const protocol_message *msg, unsigned char *iv, unsigned char *ciphertext) {
    AES_KEY aes_key_enc;
    unsigned char plaintext[sizeof(protocol_message)];
    memcpy(plaintext, msg, sizeof(protocol_message));

    // Generate random IV
    RAND_bytes(iv, AES_BLOCK_SIZE);

    // Encrypt the message
    AES_set_encrypt_key(aes_key, 128, &aes_key_enc);
    AES_cbc_encrypt(plaintext, ciphertext, sizeof(protocol_message), &aes_key_enc, iv, AES_ENCRYPT);
}

int decrypt_message(protocol_message *msg, const unsigned char *iv, const unsigned char *ciphertext) {
    AES_KEY aes_key_dec;
    unsigned char plaintext[sizeof(protocol_message)];

    // Decrypt the message
    AES_set_decrypt_key(aes_key, 128, &aes_key_dec);
    AES_cbc_encrypt(ciphertext, plaintext, sizeof(protocol_message), &aes_key_dec, (unsigned char *)iv, AES_DECRYPT);

    memcpy(msg, plaintext, sizeof(protocol_message));
    return 1;
}
