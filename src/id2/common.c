// common.c
#include "common.h"

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_hex_arr(uint8_t* arr, size_t size, char* msg) {
  printf("%s: ", msg);
  for (size_t i = 0; i < size; i++) {
    printf("%x ", arr[i]);
    if ((i%17 == 0) && (i!=0) )

    printf("\n");
  }
}

void generate_key(uint8_t key[KEY_SIZE]) {
    if (!RAND_bytes(key, KEY_SIZE))
        handle_error("Failed to generate key");
    hex_dump("[KEY] Generated shared key:", key, KEY_SIZE);
}


#include "common.h"

// Реализации для лаб 11-14




// Реализации для лабы 15 (Фиат-Шамир)
int create_server_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        handle_error("socket failed");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        handle_error("setsockopt");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        handle_error("bind failed");
    }

    if (listen(server_fd, 3) < 0) {
        handle_error("listen");
    }

    return server_fd;
}

int connect_to_server(const char *ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        handle_error("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        handle_error("Invalid address/ Address not supported");
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        handle_error("Connection Failed");
    }

    return sock;
}

void send_mpz(int socket, const mpz_t num) {
    char buf[MAX_BUF];
    mpz_get_str(buf, 16, num);
    size_t len = strlen(buf) + 1;
    send(socket, &len, sizeof(len), 0);
    send(socket, buf, len, 0);
}

void recv_mpz(int socket, mpz_t num) {
    size_t len;
    char buf[MAX_BUF];
    
    recv(socket, &len, sizeof(len), MSG_WAITALL);
    recv(socket, buf, len, MSG_WAITALL);
    
    mpz_set_str(num, buf, 16);
}   char buf[MAX_BUF];

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
