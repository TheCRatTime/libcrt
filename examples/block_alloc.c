#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/mem/block_alloc.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>

int main(void) {
  /* Creating 'plate' */
  PlateStorage plate_storage;
  Plate plate;
  
  int* data1 = NULL;
  int* data2 = NULL;
  int* data3 = NULL;

  InitPlate(&plate, plate_storage.array);

  data1 = (int*)PlateAlloc(&plate); /* O(1) allocate */
  data2 = (int*)PlateAlloc(&plate); /* O(1) allocate */

  *data1 = 123;
  *data2 = 456;

  printf("data1: 0x%p:%d, data2: 0x%p:%d\n", (void*)data1, *data1,
         (void*)data2, *data2);

  /* O(1) free: */
  PlateFree(&plate, data1);

  data3 = (int*)PlateAlloc(&plate);

  *data3 = 789;

  printf("data3: 0x%p:%d\n", (void*)data3, *data3);

  /* ... (free) ... */
  return 0;
}
