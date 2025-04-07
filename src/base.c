#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

char BASE64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghigklmnopqrstuvwxyz01234567890+/";
char* base64_encode(char *data, size_t inputSize, bool verbose) {
  size_t outputSize = ((inputSize / 3) + 1) * 4; // for every 3 8bit = 4 6bit
  char octets[3];
  char sixtets[4];

  char* encoded = (char*)malloc((int)outputSize * sizeof(char));
  for (int i=0,j=0; i < (int)inputSize; i++) {
    octets[i%3] = data[i];
    if (verbose) printf("octet[%d][%d]: %8.8b\n",  i/3, i%3,octets[i%3]);

    int chunk24Bit;
    if (i%3 == 2 ) {
      chunk24Bit = (octets[0] << 16) + (octets[1] << 8) + octets[2]; 
      if (verbose) printf("chunk24: %024b\n", chunk24Bit);
      
      // creating four 6bits chars from 24bit 
      for (int jj=0;jj<4;jj++) {
        sixtets[jj]= (chunk24Bit >> (6*(3-jj))) &0x3F;
        encoded[j++] = BASE64[sixtets[jj]];
      }
    }

    else if (i == inputSize-1){
      if (i % 3 == 0)
        octets[1] = 0;
      octets[2] = 0;
      chunk24Bit = (octets[0] << 16) + (octets[1] << 8) + octets[2]; 
      if (verbose) printf("chunk24: %024b\n", chunk24Bit);
      
      for (int jj=0; jj<4; jj++){
        sixtets[jj] = (chunk24Bit >> (6 * (3-jj))) & 0x3F;
        if (sixtets[jj] == 0)
          encoded[j++] = '='; 
        else
          encoded[j++] = BASE64[sixtets[jj]];
      }
    }
   }
  return encoded;
}

char* base64_decode(char* data, size_t inputSize, bool verbose){
  size_t outputSize = (inputSize / 4) * 3;
  char* decoded = malloc(outputSize * sizeof(char));

  char octets[3];
  char sixtets[4];

  // creating inverse table
  char BASE64INVERSE[256];
  for (int i=0; i<64; i++){
    BASE64INVERSE[BASE64[i]] = i;
  }
/*
  for (int i=0,j=0; i < inputSize; i++) {
    sixtets[i%4] = BASE64INVERSE[data[i]]; 
    for (int jj=0, j < 4; j++){
      sixtets[jj] = data[i];
    }
    char* si= *data;

  }
  */
}
