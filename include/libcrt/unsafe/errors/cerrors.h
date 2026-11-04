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

#ifndef CERRORS_CERRORS_H_
#define CERRORS_CERRORS_H_

typedef void (*CErrorHandler)(const char*, int);
typedef void (*CErrorRegister)(CErrorHandler);
typedef const char* CError;

typedef struct CErrorPackage {
  CError (*New)(CError);      /* New error */
  CError (*Throw)(CError);    /* Throw error */
  CError (*Warn)(CError);     /* New warning */
  CError (*LastError)(void);  /* New warning */
  CErrorRegister ThrowReg;    /* Register Throw function */
  CErrorRegister WarnReg;     /* Register Warning function */
} CErrorPackage;

extern const CErrorPackage cerror;

#define CERROR_ERROR 1
#define CERROR_WARN  2

#endif /* CERRORS_CERRORS_H_ */

#ifdef CERRORS_SOURCE

#include <stdio.h>
#include <stdlib.h>

static void CErrorStdThrowHandler(CError err, int type) {
  (void)type;
  
  fputs("error: ", stderr);
  fputs(err, stderr);
  fputc('\n', stderr);

  exit(EXIT_FAILURE);
}

static void CErrorStdWarnHandler(CError warn, int type) {
  (void)type;
  
  fputs("warning: ", stderr);
  fputs(warn, stderr);
  fputc('\n', stderr);
}

static CError CErrorRealLastError(CError err, int type, int mode) {
  static CError ret = NULL;
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

static CErrorHandler CErrorThrowHandler(CErrorHandler handler,
                                        int mode) {
  static CErrorHandler ret = CErrorStdThrowHandler;
  if (mode) {
    ret = handler;
  }

  return ret;
}

static CErrorHandler CErrorWarnHandler(CErrorHandler handler,
                                       int mode) {
  static CErrorHandler ret = CErrorStdWarnHandler;
  if (mode) {
    ret = handler;
  }

  return ret;
}

static CError CErrorNew(CError err) {
  return CErrorRealLastError(err, CERROR_ERROR, 1);
}

static CError CErrorWarn(CError err) {
  CErrorHandler handler = CErrorWarnHandler(NULL, 0);
  if (handler) {
    handler(err, CERROR_WARN);
  } else {
    CErrorStdWarnHandler(err, CERROR_WARN);
  }
  
  return CErrorRealLastError(err, CERROR_WARN, 1);
}

static CError CErrorThrow(CError err) {
  CErrorHandler handler = CErrorThrowHandler(NULL, 0);
  if (handler) {
    handler(err, CERROR_ERROR);
  } else {
    CErrorStdThrowHandler(err, CERROR_ERROR);
  }

  return CErrorRealLastError(err, CERROR_ERROR, 1);
}

static void CErrorThrowReg(CErrorHandler handler) {
  CErrorThrowHandler(handler, 1);
}

static void CErrorWarnReg(CErrorHandler handler) {
  CErrorWarnHandler(handler, 1);
}

static CError CErrorLastError(void) {
  return CErrorRealLastError(NULL, 0, 0);
}

const CErrorPackage cerror = {
  CErrorNew, CErrorThrow, CErrorWarn,
  CErrorLastError, CErrorThrowReg, CErrorWarnReg
};

#endif /* CERRORS_SOURCE */
