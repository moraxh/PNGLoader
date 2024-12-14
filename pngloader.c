#include "pngloader.h"

const uint32_t poly = 0xEDB88320L;

PNG loadPNGImage(const char* path) {
  FILE *fptr;
  PNG png;

  // Open the file
  fptr = fopen(path, "rb");

  if (fptr == NULL) {
      printf("Error opening file");
      exit(EXIT_FAILURE);
  }

  // Check file size
  png.fileSize = getFileSize(fptr);

  // Get Magic Numbers
  png.magicNumbers = getMagicNumbers(fptr);

  // Check if image is PNG
  if (!isPNG(png.magicNumbers)) {
      printf("File must be in PNG Format");
      exit(EXIT_FAILURE);
  }

  // Get chunks
  png.chunks = getChunksFromFile(fptr, &png.header);

  fclose(fptr); // Close the file

  return png;
}

void savePNGImage(const char* path, PNG png) {
  FILE *fptr;

  // Open the file
  fptr = fopen(path, "wb");

  if (fptr == NULL) {
      printf("Error opening file");
      exit(EXIT_FAILURE);
  }

  // Write magic numbers
  fwrite(png.magicNumbers, 1, 8, fptr);

  uint32_t length;
  uint32_t crc;

  // Write chunks
  int i = 0;
  while (1) {
    // Write chunk length
    length = __builtin_bswap32(png.chunks[i].length); // Endianess
    fwrite(&length, sizeof(uint32_t), 1, fptr);

    // Write chunk type
    fwrite(png.chunks[i].type, 1, 4, fptr);

    // Write data
    fwrite(png.chunks[i].data, png.chunks[i].length, 1, fptr);
    
    // Write crc
    crc = __builtin_bswap32(png.chunks[i].crc); // Endianess
    fwrite(&crc, sizeof(uint32_t), 1, fptr);

    if (memcmp(png.chunks[i].type, "IEND", 4) == 0) {
      break;
    }

    i++;
  }

  fclose(fptr); // Close the file
}

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
  long fileSize = ftell(fptr); // Get the cursor position (fileSize)

  if (fileSize <= 0) {
    perror("Empty file");
    exit(EXIT_FAILURE);
  }

  return fileSize;
}

unsigned char* getMagicNumbers(FILE *fptr) {
  rewind(fptr); // Go back to the begining

  unsigned char* magicNumbers = malloc(8);

  if (magicNumbers == NULL) {
    printf("Failed to allocate memory for magic numbers");
    exit(EXIT_FAILURE);
  }

  if (fread(magicNumbers, 1, 8, fptr) < 8) {
    printf("Failed to read from magic numbers");
    exit(EXIT_FAILURE);
  }

  return magicNumbers;
}

HEADER getHeaderFromChunks(unsigned char* data) {
  HEADER header;
  int location = 0;

  // Width
  memcpy(&header.width, data + location, sizeof(header.width));
  header.width = __builtin_bswap32(header.width); // Endianess
  location+=sizeof(header.width);

  // Height
  memcpy(&header.height, data + location, sizeof(header.height));
  header.height = __builtin_bswap32(header.height); // Endianess
  location+=sizeof(header.height);

  // Bit Depth
  memcpy(&header.bit_depth, data + location, sizeof(header.bit_depth));
  location+=sizeof(header.bit_depth);

  // Color Type
  memcpy(&header.color_type, data + location, sizeof(header.color_type));
  location+=sizeof(header.color_type);

  // Compression Method
  memcpy(&header.compression_method, data + location, sizeof(header.compression_method));
  location+=sizeof(header.compression_method);

  // Filter Method
  memcpy(&header.filter_method, data + location, sizeof(header.filter_method));
  location+=sizeof(header.filter_method);

  // Interlance Method
  memcpy(&header.interlance_method, data + location, sizeof(header.interlance_method));
  location+=sizeof(header.interlance_method);

  return header;
}

CHUNK* getChunksFromFile(FILE *fptr, HEADER* header) {
  CHUNK* chunks = malloc(sizeof(CHUNK));

  if (chunks == NULL) {
    perror("Failed to allocate memory for chunks array");
    exit(EXIT_FAILURE);
  }

  fseek(fptr, 8, SEEK_SET); // 8 bytes magic numbers

  int i = 0;
  
  while (1){
    // Chunk Length
    fread(&chunks[i].length, sizeof(uint32_t), 1, fptr);
    chunks[i].length = __builtin_bswap32(chunks[i].length); // Endianess

    // Chunk Type
    chunks[i].type = malloc(4);
    fread(chunks[i].type, 1, 4, fptr); // Type

    // Allocate memory for data
    chunks[i].data = malloc(chunks[i].length); // Allocate memory
    if (chunks[i].data == NULL) {
      perror("Failed to allocate memory for chunk data");
      exit(EXIT_FAILURE);
    }

    // Chunk Data
    fread(chunks[i].data, chunks[i].length, 1, fptr); // Data

    if (memcmp(chunks[i].type, "IHDR", 4) == 0) {
      *header = getHeaderFromChunks(chunks[i].data);
    }

    // Chunk CRC
    fread(&chunks[i].crc, sizeof(uint32_t), 1, fptr);
    chunks[i].crc = __builtin_bswap32(chunks[i].crc); // Endianess

    // Validate crc
    uint32_t calculatedCRC = calculateCRC(chunks[i].type, chunks[i].data, chunks[i].length);

    if (calculatedCRC !=  chunks[i].crc) {
      printf("Invalid CRC in chunk %s", chunks[i].type);
      exit(EXIT_FAILURE);
    }

    if (memcmp(chunks[i].type, "IEND", 4) == 0) {
      break;
    }

    i+=1;

    // Allocate more memory
    CHUNK* chunks_aux = malloc(sizeof(CHUNK) * (i + 1));
    if (chunks_aux == NULL) {
      perror("Failed to allocate memory for chunks aux array");
      exit(EXIT_FAILURE);
    }

    // Copy data
    for (int j = 0; j < i; j++) {
      chunks_aux[j] = chunks[j];
    }

    free(chunks); // Delete previous array

    chunks = malloc(sizeof(CHUNK) * (i + 1)); // Allocate memory
    if (chunks == NULL) {
      perror("Failed to allocate memory for chunks array");
      exit(EXIT_FAILURE);
    }

    // Copy data again
    for (int j = 0; j < i; j++) {
      chunks[j] = chunks_aux[j];
    }

    free(chunks_aux);
  }
  return chunks;
}

uint32_t calculateCRC(char* type, unsigned char* data, size_t dataLength) {
  // Its VERY important to set this variable in a unsigned char array
  // because if not the program can have negative values and the negative 
  // values will show them with two's complement
  unsigned char* string = malloc(dataLength + 4);

  // Copy type
  for (int i = 0; i < 4; i++) {
    string[i] = type[i];
  }

  // Copy data
  for (size_t i = 0; i < dataLength; i++) {
    string[i + 4] = data[i];
  }

	uint32_t crc = 0xFFFFFFFFu;

  for (int byteindex = 0; byteindex < dataLength + 4; byteindex++) {
    crc ^= string[byteindex];

    for (int i = 0; i < 8; i++) {
      // If the msb is 1, xor with poly
      if (crc & 1)
        crc = poly ^ (crc >> 1);
      else 
        crc >>= 1;
    }
  }

  free(string);
	
	return ~crc;
}