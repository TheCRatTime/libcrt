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
 
#ifdef CRT_NOT_USE_NAMESPACE
# error "Not using namespace. Include from unsafe/ instead."
#endif

#ifndef LIBCRT_LIBCRT_ERRORS_CERRORS_H_
#define LIBCRT_LIBCRT_ERRORS_CERRORS_H_

typedef void (*CrtCErrorHandler)(const char*, int);
typedef void (*CrtCErrorRegister)(CrtCErrorHandler);
typedef const char* CrtCError;

typedef struct CrtCErrorPackage {
  CrtCError (*New)(CrtCError);      /* New error */
  CrtCError (*Throw)(CrtCError);    /* Throw error */
  CrtCError (*Warn)(CrtCError);     /* New warning */
  CrtCError (*LastError)(void);  /* New warning */
  CrtCErrorRegister ThrowReg;    /* Register Throw function */
  CrtCErrorRegister WarnReg;     /* Register Warning function */
} CrtCErrorPackage;

extern const CrtCErrorPackage CrtCerror;

#define CERROR_ERROR 1
#define CERROR_WARN  2

#endif /* LIBCRT_LIBCRT_ERRORS_CERRORS_H_ */

#if (defined(CERRORS_SOURCE) || defined(CRT_SOURCE)) && !defined(WAS_CERRORS)
#define WAS_CERRORS

#include <stdio.h>
#include <stdlib.h>

static void CErrorStdThrowHandler(CrtCError err, int type) {
  (void)type;
  
  fputs("error: ", stderr);
  fputs(err, stderr);
  fputc('\n', stderr);

  exit(EXIT_FAILURE);
}

static void CErrorStdWarnHandler(CrtCError warn, int type) {
  (void)type;
  
  fputs("warning: ", stderr);
  fputs(warn, stderr);
  fputc('\n', stderr);
}

static CrtCError CErrorRealLastError(CrtCError err, int type, int mode) {
  static CrtCError ret = NULL;
  static char ret_type = '\0';
  
  if (mode == 1) {
    ret = err;
    if (type) {
      ret_type = (char)type + '0';
    }
  } else if (mode == 2) {
    return &ret_type;
  }

  return ret;
}

static CrtCErrorHandler CErrorThrowHandler(CrtCErrorHandler handler,
                                           int mode) {
  static CrtCErrorHandler ret = CErrorStdThrowHandler;
  if (mode) {
    ret = handler;
  }

  return ret;
}

static CrtCErrorHandler CErrorWarnHandler(CrtCErrorHandler handler,
                                          int mode) {
  static CrtCErrorHandler ret = CErrorStdWarnHandler;
  if (mode) {
    ret = handler;
  }

  return ret;
}

static CrtCError CErrorNew(CrtCError err) {
  return CErrorRealLastError(err, CERROR_ERROR, 1);
}

static CrtCError CErrorWarn(CrtCError err) {
  CrtCErrorHandler handler = CErrorWarnHandler(NULL, 0);
  if (handler) {
    handler(err, CERROR_WARN);
  } else {
    CErrorStdWarnHandler(err, CERROR_WARN);
  }
  
  return CErrorRealLastError(err, CERROR_WARN, 1);
}

static CrtCError CErrorThrow(CrtCError err) {
  CrtCErrorHandler handler = CErrorThrowHandler(NULL, 0);
  if (handler) {
    handler(err, CERROR_ERROR);
  } else {
    CErrorStdThrowHandler(err, CERROR_ERROR);
  }

  return CErrorRealLastError(err, CERROR_ERROR, 1);
}

static void CErrorThrowReg(CrtCErrorHandler handler) {
  CErrorThrowHandler(handler, 1);
}

static void CErrorWarnReg(CrtCErrorHandler handler) {
  CErrorWarnHandler(handler, 1);
}

static CrtCError CErrorLastError(void) {
  return CErrorRealLastError(NULL, 0, 0);
}

const CrtCErrorPackage CrtCerror = {
  CErrorNew, CErrorThrow, CErrorWarn,
  CErrorLastError, CErrorThrowReg, CErrorWarnReg
};

#endif /* CERRORS_SOURCE */
