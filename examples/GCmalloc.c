#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/mem/GCmalloc.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>

static void PtrHandler(void* ptr) {
  printf("GC really freed %p\n", ptr);
}

typedef struct HardStruct {
  struct HardStruct* cycle_dep;
} HardStruct;

static void TestFunc(void) {
  HardStruct* ptr1 = NULL;
  HardStruct* ptr2 = NULL;
  HardStruct* garbage = NULL; /* unused */

  GC_INIT();

  ptr1 = (HardStruct*)GCMalloc(sizeof(HardStruct));
  ptr2 = (HardStruct*)GCMallocChild(sizeof(HardStruct), ptr1); /* No registering to roots */
  garbage = (HardStruct*)GCMalloc(sizeof(HardStruct)); /* unused */

  /* If you want, you can set destructor, for example: */
  GCRegDestructor(ptr1, PtrHandler);
  GCRegDestructor(ptr2, PtrHandler);
  GCRegDestructor(garbage, PtrHandler);

  GCAddDependency(ptr2, ptr1);

  GC_SET_PTR(ptr1, cycle_dep, ptr2);
  GC_SET_PTR(ptr2, cycle_dep, ptr1);
  GC_SET_PTR(garbage, cycle_dep, ptr1);

  GC_FULL_RUN();
  
  GC_DEINIT();
}

int main(void) {
  printf("TestFunc()\n");
  
  TestFunc();
  
  printf("RunGC()\n");

  GC_FULL_RUN();

  return 0;
}
