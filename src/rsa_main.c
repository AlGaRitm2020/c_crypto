#include <stdlib.h>
#include <argp.h>
#include <stdio.h>
#include <stdbool.h>
#include <gmp.h>
#include "base.h"
#include "rsa.h"

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<your@email.com>";
static char doc[] = "Implemention base64/base32 encode and decode functions on C";
static char args_doc[] = "[FILENAME]...";  // Description of non-option arguments

/* Options definition */
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"output", 'o', "FILE", 0, "Output to FILE instead of stdout"},
    {"generate", 'g', 0, 0, "Generate keys PUBLIC and PRIVATE"},
  {"encode", 'e', 0, 0, "Encrypt input "},
    {"decode", 'd', 0, 0, "Decode input"},
    {0}
};

/* Structure to hold parsed arguments */
struct arguments {
    char *output_file;
    int verbose;
    int mode;  // 0 for gen, 1 for encode, 2 for decode 
};

/* Parse a single option */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    
    switch (key) {
        case 'v':
            arguments->verbose = 1;
            break;
        case 'o':
            arguments->output_file = arg;
            break;
        case 'g':
            arguments->mode = 0;
            break;
        case 'e':
            arguments->mode= 1;
            break;
        case 'd':
            arguments->mode= 2;
            break;
        
            
        case ARGP_KEY_ARG:
            // Handle non-option arguments (filenames)
            // arguments->input_files = realloc(arguments->input_files, (arguments->input_files_count + 1) * sizeof(char *));
            // arguments->input_files[arguments->input_files_count++] = arg;
            // break;
            //
        // case ARGP_KEY_END:
            // Handle end of arguments
            // if (arguments->input_files_count == 0) {
            //     // No input files specified, read from stdin
            // }
            // break;
            
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


/* Argp parser */
static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
    struct arguments arguments;
    
    /* Default values */
    arguments.verbose = 0;
    arguments.mode= 1;  // Default to encode mode
    
    /* Parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    bool verbose = arguments.verbose;
    if(verbose) printf("Verbose output ENABLED\n");

    char *buffer;
    size_t buffSize = 32;
    size_t charsCount;
    
    buffer = (char *)malloc(sizeof(char) * buffSize);
    if (buffer == NULL) {
      perror("unable to allocate buffer");
      exit(1);
    }

    if(verbose) printf("Type: ");
    charsCount = getline(&buffer, &buffSize, stdin);

    if(verbose) printf("%zu chars were read\n",charsCount);
    if(verbose) printf("you typed: %s \n",buffer);

    
    /* Program logic goes here */
    // if(verbose) printf("Running in %s mode\n", arguments.encode ? "encode" : "decode");
    
    if (arguments.mode == 0) { // generate 
    //
        char *public_key_file; 
        char *private_key_file= buffer; 
        // sprintf(public_key_file, "%s.pub", buffer);
        // rsa_gen_key(512, public_key_file, private_key_file, verbose);
        rsa_gen_key(512, "keys/ts.pub", "keys/ts", verbose);
        
    
//    getline(kk);
    }
    else if (arguments.mode == 1) { // encode 
      char* encrypted = (char*)malloc(100);
      if (encrypted == NULL) return 1;
      size_t enc_size = 0;
      rsa_encode(buffer, charsCount, "hello.pub", &encrypted, &enc_size, verbose);
      printf("ENCR in main. CIPTHERTEXT: %s, SIZE: %d", encrypted, (int)enc_size); 
    }
    else if( arguments.mode == 2) {  //decode
      char* decrypted = (char*)malloc(10);
      if (decrypted == NULL) return 1;
      size_t dec_size = 0;   

      rsa_decode(buffer, charsCount, "hello", &decrypted, &dec_size, verbose);
    
      printf("DECR in main. CIPTHERTEXT: %s, SIZE: %d", decrypted, (int)dec_size); 
  }
    else if(!arguments.mode ){
    }   
    /* Clean up */
    // free(arguments.input_files);
    
    return 0;
}
