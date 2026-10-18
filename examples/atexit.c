#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define MAIN_VOID
#define CRT_IMPORT "unsafe/mem/atexit.h"
#define CRT_IMPORT2 "unsafe/mem/main_wrapper.h"
#include "../include/libcrt/crt.h"


int Main(AtExitHeader* main_atexit) {
  char* buffer = (char*)AtExitMalloc(main_atexit, 4096);
  if (!buffer) {
    return 1;
  }

  /* do something... */

  /* No need write free() */
  return 0;
}
