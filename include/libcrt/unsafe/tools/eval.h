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

#ifndef EVAL_EVAL_H_
#define EVAL_EVAL_H_

/* Evaluate the EXPRession.
   Returns 0.0 at error, printing error to stderr. */
double eval(const char* expr);

#endif /* EVAL_EVAL_H_ */

#ifdef EVAL_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

#define EVAL_MAX_RECURSION_LEVEL 300

typedef struct eval_lexer {
  const char* ptr;
  int charno;
  int cur_rec_level;
  jmp_buf error_jmp;
} eval_lexer;

/* Clang's -Wmissing-noreturn disable */
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

static void eval_error(const char* err, eval_lexer* lxr) {
  fprintf(stderr, "eval: %d: %s\n", lxr->charno, err);
  longjmp(lxr->error_jmp, 1);
}

#ifdef __clang__
# pragma clang diagnostic pop
#endif

#define eval_cur(lxr) (*((lxr)->ptr))
#define eval_shift_by(lxr, diff) do { \
  (lxr)->charno += (diff); \
  (lxr)->ptr += (diff); \
} while (0)

#define eval_guard(lxr) do { \
  if ((lxr)->cur_rec_level >= EVAL_MAX_RECURSION_LEVEL) { \
    eval_error("too hard expression", (lxr)); \
  } \
  \
  (lxr)->cur_rec_level++; \
} while (0)

static void eval_skip_spaces(eval_lexer* lxr) {
  while (isspace(eval_cur(lxr))) { eval_shift_by(lxr, 1); } 
}

static double eval_level1(eval_lexer* lxr);

static double eval_level3(eval_lexer* lxr) {
  double ret = 0.0;

  eval_guard(lxr);

  if (eval_cur(lxr) == '-') {
    eval_shift_by(lxr, 1);
    return -eval_level3(lxr);
  }
  
  if (isdigit(eval_cur(lxr))) {
    char* end = NULL;
    ret = strtod(lxr->ptr, &end);

    eval_shift_by(lxr, end - lxr->ptr);
    eval_skip_spaces(lxr);

    return ret;
  }

  if (eval_cur(lxr) == '(') {
    eval_shift_by(lxr, 1);
    ret = eval_level1(lxr);
    if (eval_cur(lxr) != ')') {
      eval_error("unclosed '('", lxr);
    }

    eval_shift_by(lxr, 1);

    return ret;
  }
  
  eval_error("unknown character", lxr);
  return 0.0;
}

static double eval_level2(eval_lexer* lxr) {
  double ret = eval_level3(lxr);
  double tmp = 0.0;
  char op = 0;

  eval_guard(lxr);
  
  eval_skip_spaces(lxr);

  while (eval_cur(lxr) == '*' || eval_cur(lxr) == '/') {
    op = eval_cur(lxr);
    eval_shift_by(lxr, 1);
    eval_skip_spaces(lxr);

    tmp = eval_level3(lxr);

    switch (op) {
      case '*': ret *= tmp; break;
      case '/':
        if (tmp == 0.0) {
          eval_error("division by zero", lxr);
        }

        ret /= tmp;
        break;
      default:
        break;
    }
  }

  return ret;
}

static double eval_level1(eval_lexer* lxr) {
  double ret = eval_level2(lxr);
  double tmp = 0.0;
  char op = 0;

  eval_guard(lxr);

  eval_skip_spaces(lxr);

  while (eval_cur(lxr) == '+' || eval_cur(lxr) == '-') {
    op = eval_cur(lxr);

    eval_shift_by(lxr, 1);
    eval_skip_spaces(lxr);

    tmp = eval_level2(lxr);

    switch (op) {
      case '+': ret += tmp; break;
      case '-': ret -= tmp; break;
      default: break;
    }
  }

  return ret;
}

double eval(const char* expr) {
  eval_lexer lxr;
  double ret = 0;
  lxr.charno = 1;
  lxr.cur_rec_level = 0;
  lxr.ptr = expr;

  if (setjmp(lxr.error_jmp) == 0) {
    ret = eval_level1(&lxr);
    if (*(lxr.ptr) != '\0') {
      fprintf(stderr, "eval: %d: unknown character\n", lxr.charno);
      return 0.0;
    }

    return ret;
  } else {
    return 0.0;
  }
}

#endif /* EVAL_SOURCE */
