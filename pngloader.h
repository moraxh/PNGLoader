#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t color_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlance_method;
} HEADER;

typedef struct {
  uint32_t length;
  char type[5];
  char* data;
  uint32_t crc;
} CHUNK;


typedef struct {
  uint32_t fileSize;
  unsigned char* magicNumbers;
  HEADER header;
  CHUNK* chunks;
} PNG;

int isPNG(const unsigned char* magicNumber);
long getFileSize(FILE *fptr);
unsigned char* getMagicNumbers(FILE *fptr);
HEADER getHeaderFromChunks(char* data);
CHUNK* getChunksFromFile(FILE *fptr, HEADER* header);
char* loadPNGImage(const char* path);