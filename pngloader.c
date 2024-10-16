#include "pngloader.h"

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

  if (magicNumbers == NULL) {
    printf("Error reading the file");
    return NULL;
  }

  // Check if image is PNG
  if (!isPNG(magicNumbers)) {
      printf("File must be in PNG Format");
      return NULL;
  }

  fclose(fptr); // Close the file

  return NULL;
}