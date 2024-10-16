#include "pngloader.h"

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t color_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlance_method;
} PNGHeader;


int isPNG(const unsigned char* magicNumber) {
    return 
        magicNumber[0] == 0X89 &&
        magicNumber[1] == 0X50 &&
        magicNumber[2] == 0X4E &&
        magicNumber[3] == 0X47 &&
        magicNumber[4] == 0X0D &&
        magicNumber[5] == 0X0A &&
        magicNumber[6] == 0X1A &&
        magicNumber[7] == 0X0A;
}

long getFileSize(FILE *fptr) {
  fseek(fptr, 0, SEEK_END); // Go to the end of the file
  return ftell(fptr); // Get the cursor position (fileSize)
}

unsigned char* getMagicNumbers(FILE *fptr) {
  rewind(fptr); // Go back to the begining

  unsigned char* magicNumbers = malloc(8);

  if (fread(magicNumbers, 1, 8, fptr) < 8) return NULL;

  return magicNumbers;
}

PNGHeader* getPNGHeader(FILE *fptr) {
  fseek(fptr, 8, SEEK_SET); // 8 bytes offset (8 bytes File Header)

  // Validate chunk size (13 bytes in IHDR) 
  uint32_t chunk_size;
  char chunk_type[5];

  fread(&chunk_size, sizeof(chunk_size), 1, fptr);
  fread(chunk_type, sizeof(char), 4, fptr);

  chunk_size = __builtin_bswap32(chunk_size);

  printf("%u\n", chunk_size);
  printf("%s\n", chunk_type);

  if (chunk_size != 13 || strcmp(chunk_type, "IHDR") != 0) return NULL;

  PNGHeader *header = malloc(sizeof(PNGHeader));

  fread(&header->width, sizeof(header->width), 1, fptr); // Width
  fread(&header->height, sizeof(header->height), 1, fptr); // Height
  fread(&header->bit_depth, 1, 1, fptr); // Bit Depth
  fread(&header->color_type, 1, 1, fptr); // Color Type
  fread(&header->compression_method, 1, 1, fptr); // Compression Method
  fread(&header->filter_method, 1, 1, fptr); // Filter Method
  fread(&header->interlance_method, 1, 1, fptr); // Interlace Method

  // Endianess
  header->width = __builtin_bswap32(header->width);
  header->height = __builtin_bswap32(header->height);

  return header;
}

char* loadPNGImage(const char* path) {
  FILE *fptr;

  // Open the file
  fptr = fopen(path, "rb");

  if (fptr == NULL) {
      printf("Error opening file");
      return NULL;
  }

  // Get the size of the file
  long fileSize = getFileSize(fptr);

  if (fileSize == 0) {
      printf("File is empty");
      return NULL;
  }

  unsigned char* magicNumbers = getMagicNumbers(fptr);

  // Extracting the magic numbers
  if (magicNumbers == NULL) {
    printf("Error reading the file");
    return NULL;
  }

  // Check if image is PNG
  if (!isPNG(magicNumbers)) {
      printf("File must be in PNG Format");
      return NULL;
  }
  
  // Get header
  PNGHeader *header = getPNGHeader(fptr);

  if (header == NULL) {
    printf("Error while reading the image's header");
  }

  printf("%d\n", header->width);
  printf("%d\n", header->height);

  fclose(fptr); // Close the file

  return NULL;
}