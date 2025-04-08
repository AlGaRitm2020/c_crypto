#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>


char BASE64[64]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghigklmnopqrstuvwxyz0123456789+/";

char BASE32[32]="ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";


char* format24bits(int value, char* buffer) {
    char oct0 = (value >> 16) & 0xFF;
    char oct1 = (value >> 8) & 0xFF;
    char oct2 = (value >> 0) & 0xFF;

    sprintf(buffer, "%08b-%08b-%08b",oct0,oct1,oct2);
    return buffer; 
}
char* format40bits(int64_t value, char* buffer) {
    char oct0 = (value >> 32) & 0xFF;
    char oct1 = (value >> 24) & 0xFF;
    char oct2 = (value >> 16) & 0xFF;
    char oct3 = (value >> 8) & 0xFF;
    char oct4 = (value >> 0) & 0xFF;

    sprintf(buffer, "%08b-%08b-%08b-%08b-%08b",oct0,oct1,oct2, oct3, oct4);
    return buffer; 
}

char* base64_encode(char *data, size_t inputSize, bool verbose) {
  size_t outputSize = ((inputSize / 3) + 1) * 4; // for every 3 8bit = 4 6bit
  char octets[3];
  char sextets[4];

  char* encoded = (char*)malloc((int)outputSize * sizeof(char));
  int j=0;
  for (int i=0; i < (int)inputSize; i++) {
    octets[i%3] = data[i];
    if (verbose) printf("octet[%d][%d]:\t%8.8b\n",  i/3, i%3,octets[i%3]);

    int chunk24Bit;
    int lastOctet= (int)(inputSize) %3;
    int lastSextet= ((lastOctet * 8)/6)+1;
    if (lastOctet ==0 ) lastSextet=5;

    if (i%3 ==2 || i == inputSize-1){ // chunk create condition
      // adding blank octets to fill 24 bit chunk
      if (i == inputSize-1){
        for (int ii=(i%3)+1; ii<3;ii++)
          octets[ii]=0;
        
        
      }
      chunk24Bit = (octets[0] << 16) + (octets[1] << 8) + octets[2]; 
      //if (verbose) printf("chunk24[%d]: %024b\n", i/3,chunk24Bit);
      char *formattedChunk = format24bits(chunk24Bit, formattedChunk);
      if (verbose) printf("chunk24[%d]:\t%s\n", i/3,formattedChunk);

      for (int jj=0; jj<4; jj++){
        sextets[jj] = (chunk24Bit >> (6 * (3-jj))) & 0x3F;
        if ((i==inputSize-1) && (jj >= lastSextet))
          encoded[j++] = '='; 
        else
          encoded[j++] = BASE64[sextets[jj]];
     }
    }
   }
   encoded[j] = '\0';
  return encoded;
}

char* base64_decode(char* data, size_t inputSize, bool verbose){
  size_t outputSize = (inputSize / 4) * 3;
  char* decoded = malloc(outputSize * sizeof(char));

  char octets[3];
  char sextets[4];

  // creating inverse table
  char BASE64INVERSE[256];
  for (int i=0; i<64; i++){
    BASE64INVERSE[BASE64[i]] = i;
  }

  // i = input idx; j = output idx; jj = inx inside the chunk;
  for (int i=0,j=0; i < inputSize; i++) {
    sextets[i%4] = BASE64INVERSE[data[i]]; 
    if(verbose) printf("sixtet[%d][%d]:\t%06b\n", i/4, i%4, sextets[i%4]);

    if (i%4==3 | i == inputSize-1){
      int chunk24Bit = (sextets[0] << 18) + (sextets[1] << 12) + (sextets[2] << 6) + sextets[3];

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


char* base32_encode(char *data, size_t inputSize, bool verbose) {
  size_t outputSize = ((inputSize / 5) + 1) * 8; // for every 3 8bit = 4 6bit
  int64_t octets[5];
  int64_t quintets[8];

    int lastOctet= (int)(inputSize) %5;
    int lastQuintet= ((lastOctet * 8)/5)+1;
    if (lastOctet==0) lastQuintet=9;

  char* encoded = (char*)malloc((int)outputSize * sizeof(char));
  int j=0;
  for (int i=0; i < (int)inputSize; i++) {
    octets[i%5] = data[i];
    if (verbose) printf("octet[%d][%d]:\t%8.8b\n",  i/5, i%5,octets[i%5]);

    int64_t chunk40Bit;

    if (i%5 ==4 || i == inputSize-1){ // chunk create condition
      
      // adding blank octets to fill chunk
      if (i == inputSize-1){
        for (int ii=(i%5)+1; ii<5; ii++)
          octets[ii] = 0;
      }
      chunk40Bit = (octets[0] << 32) + (octets[1] << 24) + (octets[2] << 16) + (octets[3] << 8) + octets[4]; 
      //if (verbose) printf("chunk24[%d]: %024b\n", i/3,chunk24Bit);
      char *formattedChunk = (char*)malloc(sizeof(char) * 45);
      format40bits(chunk40Bit, formattedChunk);
      if (verbose) printf("chunk24[%d]:\t%s\n", i/5,formattedChunk);

      for (int jj=0; jj<8; jj++){
        quintets[jj] = (chunk40Bit >> (5 * (7-jj))) & 0x1F;
        if (verbose) printf("quintets[%d][%d]:\t%5.5b  ===  %c\n",  i/8, jj%8,quintets[jj],BASE32[quintets[jj]]);
        if ((i==inputSize-1) && (jj>=lastQuintet)) {
          encoded[j++] = '='; 
//          encoded[j++] = '';
        }
        else
          encoded[j++] = BASE32[quintets[jj]];
     }
      
    }
   }
   encoded[j] = '\0';
  return encoded;
}


char* base32_decode(char* data, size_t inputSize, bool verbose){
  size_t outputSize = (inputSize / 8) * 5;
  char* decoded = malloc(outputSize * sizeof(char));

  int64_t octets[5];
  int64_t quintets[8];

  // creating inverse table
  char BASE32INVERSE[256];
  for (int i=0; i<32; i++){
    BASE32INVERSE[BASE32[i]] = i;
  }

  // i = input idx; j = output idx; jj = inx inside the chunk;
  for (int i=0,j=0; i < inputSize; i++) {
    quintets[i%8] = BASE32INVERSE[data[i]]; 
    if(verbose) printf("quintets[%d][%d]:\t%06b\n", i/8, i%8, quintets[i%8]);

    if (i%8==7 | i == inputSize-1){
      int64_t chunk40Bit = (quintets[0] << 35) + (quintets[1] << 30) + (quintets[2] << 25) +(quintets[3] << 20) + (quintets[4] << 15) + (quintets[5] << 10) + (quintets[6] << 5) + quintets[7];

      char* formattedChunk = (char*)malloc(sizeof(char) * 45);
      formattedChunk = format40bits(chunk40Bit, formattedChunk);
      if (verbose) printf("chunk40[%d]:\t%s\n", i/8,formattedChunk);
 //     if (verbose) printf("chunk24[%d]:\t%024b\n", i/4,chunk24Bit);
      free(formattedChunk);

      for (int jj=0; jj<5;jj++) {
        octets[jj] = (chunk40Bit >> (4-jj)*8) & 0xFF;
        decoded[j++] = octets[jj]; 
      } 
    }
  }

  return decoded;
  
}
