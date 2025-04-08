#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


char BASE64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghigklmnopqrstuvwxyz01234567890+/";


char* format24bits(int value, char* buffer) {
    char oct0 = (value >> 16) & 0xFF;
    char oct1 = (value >> 8) & 0xFF;
    char oct2 = (value >> 0) & 0xFF;

    sprintf(buffer, "%08b-%08b-%08b",oct0,oct1,oct2);
    return buffer; 
}

char* base64_encode(char *data, size_t inputSize, bool verbose) {
  size_t outputSize = ((inputSize / 3) + 1) * 4; // for every 3 8bit = 4 6bit
  char octets[3];
  char sixtets[4];

  char* encoded = (char*)malloc((int)outputSize * sizeof(char));
  for (int i=0,j=0; i < (int)inputSize; i++) {
    octets[i%3] = data[i];
    if (verbose) printf("octet[%d][%d]:\t%8.8b\n",  i/3, i%3,octets[i%3]);

    int chunk24Bit;

    if (i%3 ==2 || i == inputSize-1){ // chunk create condition
      // adding blank octets to fill 24 bit chunk
      if (i == inputSize-1){
        if (i % 3 == 0)
          octets[1] = 0;
        octets[2] = 0;
      }
      chunk24Bit = (octets[0] << 16) + (octets[1] << 8) + octets[2]; 
      //if (verbose) printf("chunk24[%d]: %024b\n", i/3,chunk24Bit);
      char *formattedChunk = format24bits(chunk24Bit, formattedChunk);
      if (verbose) printf("chunk24[%d]:\t%s\n", i/3,formattedChunk);

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

  // i = input idx; j = output idx; jj = inx inside the chunk;
  for (int i=0,j=0; i < inputSize; i++) {
    sixtets[i%4] = BASE64INVERSE[data[i]]; 
    if(verbose) printf("sixtet[%d][%d]:\t%06b\n", i/4, i%4, sixtets[i%4]);

    if (i%4==3 | i == inputSize-1){
      int chunk24Bit = (sixtets[0] << 18) + (sixtets[1] << 12) + (sixtets[2] << 6) + sixtets[3];

      char* formattedChunk = (char*)malloc(sizeof(char) * 30);
      formattedChunk = format24bits(chunk24Bit, formattedChunk);
      if (verbose) printf("chunk24[%d]:\t%s\n", i/4,formattedChunk);
 //     if (verbose) printf("chunk24[%d]:\t%024b\n", i/4,chunk24Bit);
      free(formattedChunk);

      for (int jj=0; jj<3;jj++) {
        octets[jj] = (chunk24Bit >> (2-jj)*8) & 0xFF;
        decoded[j++] = octets[jj]; 
      } 
    }
  }

  return decoded;
  
}
