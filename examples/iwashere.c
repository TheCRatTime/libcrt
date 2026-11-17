#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/tools/iwashere.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>
#include <stdlib.h>

static void error(void) { fputs("Exited.\n", stdout); exit(EXIT_FAILURE); }

static void some_func(void) {
  iwashere();
  printf("some error\n");
  error();
}

int main(void) {
  iwashere();
  printf("calling some_func()...\n");
  some_func();
  return 0;
}
