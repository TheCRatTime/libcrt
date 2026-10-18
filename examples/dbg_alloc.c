#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/mem/dbg_alloc.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>

int main(void) {
  int* data = NULL;
  
  InitAllocator(4096);

  data = (int*)DbgAlloc(sizeof(int));
  if (data == NULL) {
    return 1;
  }

  *data = 5;

  printf("data = %p, *data = %d\n", (void*)data, *data);

  /* Alignment: 8
     sizeof(int) = 4 -> 8
     data = 0, data + 2*4 = 8 - end of allocated data. */
  *(data+2) = 6;

  AllocatorLog();

  KillAllocator();

  return 0;
}
