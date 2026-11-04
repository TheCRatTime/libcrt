#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/mem/GCmalloc.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>

typedef struct HardStruct {
  struct HardStruct* cycle_dep;
} HardStruct;

static void TestFunc(void) {
  HardStruct* ptr1 = NULL;
  HardStruct* ptr2 = NULL;
  HardStruct* garbage = NULL; /* unused */

  GC_INIT();

  ptr1 = (HardStruct*)GCMalloc(sizeof(HardStruct));
  ptr2 = (HardStruct*)GCMallocRaw(sizeof(HardStruct)); /* No registering to roots */
  garbage = (HardStruct*)GCMalloc(sizeof(HardStruct)); /* unused */

  GCAddDependency(ptr1, ptr2);
  GCAddDependency(ptr2, ptr1);

  ptr1->cycle_dep = ptr2;
  ptr2->cycle_dep = ptr1;
  garbage->cycle_dep = ptr1;

  RunGC();

  GCUnregisterRoot(garbage);

  RunGC();
  
  GCreturn;
}

int main(void) {
  printf("TestFunc()\n");
  
  TestFunc();
  
  printf("RunGC()\n");
  
  RunGC();

  return 0;
}
