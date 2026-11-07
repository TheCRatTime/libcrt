/* gc_test.c - test GC garbage collection.
   for checks using dbg_alloc:
       https://github.com/TheCRatTime/dbg_alloc/
*/

#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/mem/GCMalloc.h"
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
  ptr2 = (HardStruct*)GCMalloc(sizeof(HardStruct)); /* No registering to roots */
  garbage = (HardStruct*)GCMalloc(sizeof(HardStruct)); /* unused */

  if (!ptr1 || !ptr2 || !garbage) {
    /* GC automatically free objects that was allocated. */
    GCreturn;
  }

  /* If you want, you can set destructor, for example: */
  GCRegDestructor(ptr1, PtrHandler);
  GCRegDestructor(ptr2, PtrHandler);
  GCRegDestructor(garbage, PtrHandler);

  RunGC();
  
  GC_DEINIT();
}

int main(void) {
  printf("TestFunc()\n");
  
  TestFunc();
  
  printf("RunGC()\n");

  RunGC();

  return 0;
}
