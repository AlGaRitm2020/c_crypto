#include <stdlib.h>
#include <argp.h>
#include <stdio.h>
#include <stdbool.h>
#include "base.h"

const char *argp_program_version = "1.0";
const char *argp_program_bug_address = "<your@email.com>";
static char doc[] = "Implemention base64/base32 encode and decode functions on C";
static char args_doc[] = "[FILENAME]...";  // Description of non-option arguments

/* Options definition */
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"output", 'o', "FILE", 0, "Output to FILE instead of stdout"},
    {"encode", 'e', 0, 0, "Encode input (default)"},
    {"decode", 'd', 0, 0, "Decode input"},
    {"base64", '6', 0, 0, "Base64 encoding (default)"},
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
      char* encoded = base64_encode(buffer, charsCount, arguments.verbose);
      if(verbose) printf("\nENCODED: ");
      printf("%s\n", encoded);
      free(encoded);
//    getline(kk);
    }
    else if (!arguments.encode && arguments.base64) { // base64 decode
      char* decoded = base64_decode(buffer, charsCount, arguments.verbose);
      if(verbose) printf("\nDECODED: ");
      printf("%s\n", decoded);
      free(decoded);

    }
    else if(arguments.encode && !arguments.base64){
      printf("base32 encode\n");
    }
    else if(!arguments.encode && !arguments.base64){
      printf("base32 decode\n");
    }   
    /* Clean up */
    free(arguments.input_files);
    
    return 0;
}
