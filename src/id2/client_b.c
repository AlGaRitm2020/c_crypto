// client_b.c
#include "common.h"

int main() {
    printf("Client B starting...\n");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // User input
    printf("Enter message M3: ");
    char M3[BUFFER_SIZE];
    fgets(M3, BUFFER_SIZE, stdin);
    M3[strcspn(M3, "\n")] = 0; // Remove newline
    
    printf("Enter message M4: ");
    char M4[BUFFER_SIZE];
    fgets(M4, BUFFER_SIZE, stdin);
    M4[strcspn(M4, "\n")] = 0; // Remove newline
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT_B);
    
    // Bind to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Waiting for connection from A...\n");
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    // Read message from A
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Received from A: %s\n", buffer);
    
    // Parse message (M2:IV:CIPHER)
    char *token = strtok(buffer, ":");
    if (strcmp(token, "M2") != 0) {
        printf("Invalid message format\n");
        return -1;
    }
    
    token = strtok(NULL, ":"); // M2 value
    char *M2 = token;
    printf("M2: %s\n", M2);
    
    token = strtok(NULL, ":"); // IV
    unsigned char recv_iv[AES_BLOCK_SIZE];
    memcpy(recv_iv, token, AES_BLOCK_SIZE);
    
    token = strtok(NULL, ":"); // CIPHER
    unsigned char recv_cipher[sizeof(protocol_message)];
    memcpy(recv_cipher, token, sizeof(protocol_message));
    
    // Decrypt the message
    protocol_message msg1;
    if (!decrypt_message(&msg1, recv_iv, recv_cipher)) {
        printf("Decryption failed\n");
        return -1;
    }
    
    // Verify the message
    printf("Decrypted message: identity=%c, message=%s\n", msg1.identity, msg1.message);
    
    if (msg1.identity != 'A') {
        printf("Authentication failed: invalid identity\n");
        return -1;
    }
    
    if (!verify_freshness(&msg1.freshness, 60)) {
        printf("Authentication failed: invalid freshness\n");
        return -1;
    }
    
    printf("M1: %s\n", msg1.message);
    
    // Prepare response
    protocol_message msg2;
    msg2.freshness = msg1.freshness; // Echo back the same freshness
    msg2.identity = 'B';
    strncpy(msg2.message, M3, BUFFER_SIZE);
    
    // Encrypt the response
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char ciphertext[sizeof(protocol_message)];
    encrypt_message(&msg2, iv, ciphertext);
    
    // Send response (M4:IV:CIPHER)
    char send_buffer[BUFFER_SIZE * 2];
    snprintf(send_buffer, BUFFER_SIZE * 2, "M4:%s:IV:", M4);
    int offset = strlen(send_buffer);
    memcpy(send_buffer + offset, iv, AES_BLOCK_SIZE);
    offset += AES_BLOCK_SIZE;
    memcpy(send_buffer + offset, ":CIPHER:", 8);
    offset += 8;
    memcpy(send_buffer + offset, ciphertext, sizeof(protocol_message));
    offset += sizeof(protocol_message);
    
    send(new_socket, send_buffer, offset, 0);
    printf("Response sent to A\n");
    
    printf("IDENTIFICATION SUCCESSFUL: A authenticated\n");
    
    close(new_socket);
    close(server_fd);
    return 0;
}
