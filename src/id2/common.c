// common.c
#include "common.h"

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void generate_key(uint8_t key[KEY_SIZE]) {
    if (!RAND_bytes(key, KEY_SIZE))
        handle_error("Failed to generate key");
    hex_dump("[KEY] Generated shared key:", key, KEY_SIZE);
}

void aes_encrypt(const uint8_t *plaintext, int plaintext_len,
                 const uint8_t *key, uint8_t *ciphertext, int *ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_error("EVP_CIPHER_CTX_new failed");

    hex_dump("[AES_ENC] Plaintext input", plaintext, plaintext_len);

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL))
        handle_error("EncryptInit failed");

    EVP_CIPHER_CTX_set_padding(ctx, 1); // PKCS7 padding

    int len;
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handle_error("EncryptUpdate failed");

    *ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handle_error("EncryptFinal failed");

    *ciphertext_len += len;

    hex_dump("[AES_ENC] Ciphertext output", ciphertext, *ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
}

int aes_decrypt(const uint8_t *ciphertext, int ciphertext_len,
                const uint8_t *key, uint8_t *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_error("EVP_CIPHER_CTX_new failed");

    hex_dump("[AES_DEC] Ciphertext input", ciphertext, ciphertext_len);

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL))
        handle_error("DecryptInit failed");

    EVP_CIPHER_CTX_set_padding(ctx, 1); // PKCS7 padding

    int len;
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handle_error("DecryptUpdate failed");

    int plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handle_error("DecryptFinal failed");

    plaintext_len += len;

    hex_dump("[AES_DEC] Plaintext output", plaintext, plaintext_len);

    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

void hex_dump(const char *prefix, const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;
    printf("%s\n", prefix);
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}
