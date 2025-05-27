// client_a.c
#include "common.h"

int main() {
    printf("Client A starting...\n");

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    
    // User input
    printf("Use timestamp for freshness? (1 for timestamp, 0 for random): ");
    int use_timestamp;
    scanf("%d", &use_timestamp);
    getchar(); // Consume newline
    
    printf("Enter message M1: ");
    char M1[BUFFER_SIZE];
    fgets(M1, BUFFER_SIZE, stdin);
    M1[strcspn(M1, "\n")] = 0; // Remove newline
    
    printf("Enter message M2: ");
    char M2[BUFFER_SIZE];
    fgets(M2, BUFFER_SIZE, stdin);
    M2[strcspn(M2, "\n")] = 0; // Remove newline
    
    // Generate freshness
    freshness_t freshness;
    generate_freshness(&freshness, use_timestamp);
    
    // Create message
    protocol_message msg1;
    msg1.freshness = freshness;
    msg1.identity = 'A';
    strncpy(msg1.message, M1, BUFFER_SIZE);
    
    // Encrypt part of the message
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char ciphertext[sizeof(protocol_message)];
    encrypt_message(&msg1, iv, ciphertext);
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_B);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    // Connect to Client B
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    // Send M2 and encrypted part
    char send_buffer[BUFFER_SIZE * 2];
    snprintf(send_buffer, BUFFER_SIZE * 2, "M2:%s:IV:", M2);
    int offset = strlen(send_buffer);
    memcpy(send_buffer + offset, iv, AES_BLOCK_SIZE);
    offset += AES_BLOCK_SIZE;
    memcpy(send_buffer + offset, ":CIPHER:", 8);
    offset += 8;
    memcpy(send_buffer + offset, ciphertext, sizeof(protocol_message));
    offset += sizeof(protocol_message);
    
    send(sock, send_buffer, offset, 0);
    printf("Message sent to B\n");
    
    // Receive response from B
    read(sock, buffer, BUFFER_SIZE);
    printf("Received from B: %s\n", buffer);
    
    // Parse response (M4:IV:CIPHER)
    char *token = strtok(buffer, ":");
    if (strcmp(token, "M4") != 0) {
        printf("Invalid response format\n");
        return -1;
    }
    
    token = strtok(NULL, ":"); // M4 value
    char *M4 = token;
    printf("M4: %s\n", M4);
    
    token = strtok(NULL, ":"); // IV
    unsigned char recv_iv[AES_BLOCK_SIZE];
    memcpy(recv_iv, token, AES_BLOCK_SIZE);
    
    token = strtok(NULL, ":"); // CIPHER
    unsigned char recv_cipher[sizeof(protocol_message)];
    memcpy(recv_cipher, token, sizeof(protocol_message));
    
    // Decrypt the message
    protocol_message msg2;
    if (!decrypt_message(&msg2, recv_iv, recv_cipher)) {
        printf("Decryption failed\n");
        return -1;
    }
    
    // Verify the message
    printf("Decrypted message: identity=%c, message=%s\n", msg2.identity, msg2.message);
    
    if (msg2.identity != 'B') {
        printf("Authentication failed: invalid identity\n");
        return -1;
    }
    
    if (!verify_freshness(&msg2.freshness, 60)) {
        printf("Authentication failed: invalid freshness\n");
        return -1;
    }
    
    if (memcmp(&msg2.freshness, &freshness, sizeof(freshness_t)) != 0) {
        printf("Authentication failed: freshness mismatch\n");
        return -1;
    }
    
    printf("M3: %s\n", msg2.message);
    printf("IDENTIFICATION SUCCESSFUL: B authenticated\n");
    
    close(sock);
    return 0;
}
