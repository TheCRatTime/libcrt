#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/encodings/RLE.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  const char* to_encode = NULL;
  char buffer[64] = {0};
  char buffer2[64] = {0};
  size_t encode_len = 0;
  int equals = 0;
  int diff = 0;
  
  to_encode = (argc < 2 ? "AAAAAAAAAAAAAAAABBBBBBBBBBBBB" : argv[1]);

  encode_len = strlen(to_encode);

  RLEEncode(to_encode, buffer, 63);
  
  RLEDecode(buffer, buffer2, 63);

  equals = (strcmp(buffer2, to_encode) ? 0 : 1);
  diff = (int)(strlen(buffer) - encode_len);

  printf("=== RLE - Run Length Encoding ===\n"
         "String:  %s\n"
         "Encoded: %s\n"
         "Decoded: %s\n"
         "---------------------------------\n"
         "Equals:  %s\n"
         "Diff:    %d\n"
         "=================================\n",
         to_encode, buffer, buffer2, (equals ? "yes" : "no"), diff);

  return 0;
}
