#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/encodings/hex.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
  char* data = NULL;
  char buffer[65] = {0};
  char buffer2[33] = {0};

  if (argc >= 2) {
    size_t argv1_len = strlen(argv[1]);
    if ((argv1_len << 1) > 32) {
      printf("%s: too long string (%s), grab 32 bytes...\n", argv[0], argv[1]);
      argv[1][32] = '\0';
    }

    data = argv[1];
  } else {
    data = "testdata";
  }
  
  if (HexEncodeMaxLen(data, buffer, 33) == -1) {
    printf("%s: encoding to hex error\n", argv[0]);
    return 1;
  }
  
  if (HexDecodeMaxLen(buffer, buffer2, 33) == -1) {
    printf("%s: decoding from hex error\n", argv[0]);
    return 1;
  }
  
  printf(
         "=== hex - hexadecimal format ===\n"
         "String: %s\n"
         "Encoded: %s\n"
         "Decoded: %s\n"
         "================================\n",
         data, buffer, buffer2);

  return 0;
}
