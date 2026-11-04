#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/errors/cerrors.h"
#include "../include/libcrt/crt.h"

#include <stdio.h>

static void TestHandler(CError error, int type) {
  if (type == CERROR_ERROR) {
    printf("handler: error: %s\n", error);
  } else {
    printf("handler: warning: %s\n", error);
  }
}

int main(void) {
  cerror.WarnReg(TestHandler);
  cerror.ThrowReg(TestHandler);

  cerror.Throw("throwing some text");
  printf("Last error: %s\n", cerror.LastError());

  cerror.Warn("this is a warning");
  printf("Last error: %s\n", cerror.LastError());

  cerror.New("just returned, no handlers");
  printf("Last error: %s\n", cerror.LastError());

  return 0;
}
