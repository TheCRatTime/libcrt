#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/tools/eval.h"
#include "../include/libcrt/crt.h"

int main(void) {
  /* Some hard expression */
  const char* expr = "(6 * 7 - 22) * 5 + (75 * 2) - 50 - 33";
  double res = eval(expr);

  /* expected 167 */
  printf("eval(\"%s\") = %g (or %.2f)\n", expr, res, res);

  return 0;
}
