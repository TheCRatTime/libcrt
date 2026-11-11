#define CRT_SOURCE
#define CRT_NOT_USE_NAMESPACE
#define CRT_IMPORT "unsafe/tools/testing.h"
#include "../include/libcrt/crt.h"

testing_defines; /* needed only in one file */

/* testing function */
static int sum(int a, int b) { return a+b; }
static int bad_sum(int a, int b) { return a+b+1; }

static void good(void) {
  check_cond(sum(2, 2) == 4, "sum(2, 2) check");
}

static void bad(void) {
  check_cond(bad_sum(2, 2) == 4, "sum(2, 2) check");
}

int main(void) {
  start_testing; /* for calculate different from start to end */
  run_test(good);
  run_test(bad);
  test_log(); /* printing log */
  return testing_failed /* <- return 1 if one test has failed. */;
}
