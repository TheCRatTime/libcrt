#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define NEEDED_BIND
#define CRT_IMPORT "unsafe/mem/arena.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>

/* Generate BIND with ID 'ex' */
GEN_BIND(ex) static const int dummy = 1;
/* For quiet semi-warn ~~~~~~~~~~^      */

int main(void) {
  Arena* example = InitArena(4096);
  int* data1 = NULL;
  int* data2 = NULL;

  /* Using our dummy */
  (void)dummy;
  
  if (example == NULL) {
    printf("Out of memory\n");
    return 1;
  }

  /* Using BIND */
  BindIDex(example);

  /* This is C++ style. */
  /* Can allocate so: */
  data1 = (int*)example->Alloc(sizeof(int));
  data2 = (int*)example->Alloc(sizeof(int));

  if (data1 == NULL || data2 == NULL) {
    /* Need only free arena. */
    FreeArena(example);
    printf("Arena allocate failed\n");
    return 1;
  }

  *data1 = 5;
  *data2 = 50;
  
  printf("%d + %d = %d\n", *data1, *data2, (*data1 + *data2));

  /* You only free arena */
  FreeArena(example);
  return 0;
}
