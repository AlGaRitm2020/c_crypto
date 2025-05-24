#ifndef EL_GAMAL_H
#define EL_GAMAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void el_gamal_generate_keys(const char* pub_key_file, const char* priv_key_file, int pbits, int verbose);

void el_gamal_encode(char* message, size_t size, char* priv_key_file, 
                    char** enc_message, size_t* enc_message_len, int verbose);

void el_gamal_decode(char* enc_message, size_t enc_size, char* pub_key_file,
                    char** dec_message, size_t* dec_message_len, int verbose);

#endif // EL_GAMAL_H
