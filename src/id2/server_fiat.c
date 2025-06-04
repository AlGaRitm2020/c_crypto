#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
#include "common.h"
#include "../essential_func.h"

void generate_rsa_modulus(mpz_t n, unsigned int bits, gmp_randstate_t state) {
    mpz_t p, q;
    mpz_inits(p, q, NULL);
    
    generate_prime(p, bits/2, state);
    generate_prime(q, bits/2, state);
    
    mpz_mul(n, p, q);
    mpz_clears(p, q, NULL);
}

int main() {
    mpz_t n;
    mpz_init(n);
    
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));
    
    generate_rsa_modulus(n, 2048, state);
    
    // Создание серверного сокета
    int server_fd = create_server_socket(PORT_TRUSTED);
    printf("Trusted Center: waiting for connections on port %d...\n", PORT_TRUSTED);
    
    while (1) {
        int client_socket;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            handle_error("accept");
        }
        
        printf("Trusted Center: client connected\n");
        
        // Отправка модуля n клиенту
        send_mpz(client_socket, n);
        printf("Trusted Center: sent modulus n to client\n");
        
        close(client_socket);
    }
    
    mpz_clear(n);
    gmp_randclear(state);
    close(server_fd);
    return 0;
}
