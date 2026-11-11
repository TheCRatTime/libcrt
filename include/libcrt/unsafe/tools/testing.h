/*
 * Copyright (C) 2026 TheCRatTime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TESTING_TESTING_H_
#define TESTING_TESTING_H_

#include <stdio.h>
#include <setjmp.h>
#include <time.h>

extern jmp_buf t__jmp;
extern int t__total_tests;
extern int t__failed_tests;
extern clock_t t__testing_start;

#define testing_defines \
jmp_buf t__jmp; \
int t__total_tests = 0; \
int t__failed_tests = 0; \
clock_t t__testing_start = 0

#define t__assert(cond, desc, file, line) do { \
  if (!(cond)) { \
    printf("[TESTING]: '%s' failed, description: \"%s\", at %s:%d\n", \
           #cond, desc, file, line); \
    longjmp(t__jmp, 1); \
  } \
} while (0)

#define check_cond(cond, desc) t__assert(cond, desc, __FILE__, __LINE__)
#define testing_failed (t__failed_tests ? 1 : 0)

#define run_test(func) do { \
  volatile clock_t t__start = 0; \
  volatile clock_t t__end = 0; \
  double t__diff = 0.0; \
  \
  t__total_tests++; \
  t__start = clock(); \
  \
  if (setjmp(t__jmp) == 0) { \
    func(); \
    t__end = clock(); \
    t__diff = (double)(((double)t__end - (double)t__start) \
              / CLOCKS_PER_SEC) * 1000.0; \
    printf("[OK]: test '%s' passed (%.2f ms)\n", #func, t__diff); \
  } else { \
    t__end = clock(); \
    t__diff = (double)(((double)t__end - (double)t__start) \
              / CLOCKS_PER_SEC) * 1000.0; \
    printf("[FAIL]: test '%s' failed (%.2f ms)\n", #func, t__diff); \
    t__failed_tests++; \
  } \
} while (0)

#define test_log() do { \
  double t__diff = (double)(((double)(clock()) - (double)t__testing_start) / \
                   CLOCKS_PER_SEC) * 1000.0; \
  printf("=== TESTING LOG ===\n" \
         "Total:  %d\n" \
         "Failed: %d\n" \
         "Spent:  %.2f ms\n", \
         t__total_tests, t__failed_tests, t__diff); \
} while (0)

#define start_testing t__testing_start = clock()

#endif /* TESTING_TESTING_H_ */
