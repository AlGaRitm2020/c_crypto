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
    {"base64", '4', 0, 0, "Base64 encoding (default)"},
    {"base32", '2', 0, 0, "Base32 encoding"},
    {0}
};

/* Structure to hold parsed arguments */
struct arguments {
    char *output_file;
    int verbose;
    int encode;  // 1 for encode, 0 for decode
    int base64;  // 1 for base64, 0 for base32 
    char **input_files;
    int input_files_count;
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
        case 'e':
            arguments->encode = 1;
            break;
        case 'd':
            arguments->encode = 0;
            break;
        case '4':
            arguments->base64= 1;
            break;
        case '2':
            arguments->base64= 0;
            break;
            
        case ARGP_KEY_ARG:
            // Handle non-option arguments (filenames)
            arguments->input_files = realloc(arguments->input_files, (arguments->input_files_count + 1) * sizeof(char *));
            arguments->input_files[arguments->input_files_count++] = arg;
            break;
            
        case ARGP_KEY_END:
            // Handle end of arguments
            if (arguments->input_files_count == 0) {
                // No input files specified, read from stdin
            }
            break;
            
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
    arguments.output_file = "-";  // Default to stdout
    arguments.verbose = 0;
    arguments.encode = 1;  // Default to encode mode
    arguments.base64= 1;  // Default to encode mode
    arguments.input_files = NULL;
    arguments.input_files_count = 0;
    
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
    if(verbose) printf("Running in %s mode (encoding: %s)\n", arguments.encode ? "encode" : "decode", arguments.base64 ? "base64" : "base32");
    
    if (arguments.encode && arguments.base64) { // base64 encod3
    //
         mpz_t n ,two; 
        mpz_init(n);
        mpz_init(two);
        //
         printf("old n= %Zd \n",n); 
        mpz_set_ui(n, 10); 
        mpz_set_ui(two, 2); 

         gmp_printf("old n= %Zd \n",n); 
         rsa_encode(buffer, charsCount, n, two);
        
        gmp_printf("new 2^10 mod 10 = %Zd \n",n); 

        mpz_clear(n);
        mpz_clear(two);
    
//    getline(kk);
    }
    else if (!arguments.encode && arguments.base64) { // base64 decode

    }
    else if(arguments.encode && !arguments.base64){
    }
    else if(!arguments.encode && !arguments.base64){
    }   
    /* Clean up */
    free(arguments.input_files);
    
    return 0;
}
