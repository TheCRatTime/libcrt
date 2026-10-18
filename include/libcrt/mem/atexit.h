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

/* === BEGIN HEADER === */

#ifndef LIBCRT_LIBCRT_MEM_ATEXIT_H_
#define LIBCRT_LIBCRT_MEM_ATEXIT_H_

#include <stdio.h>
#include <stddef.h>

/* Initial AtExitHeader struct */
#define DEFAULT_ATEXIT_SIZE 50

/* Pass default value to InitAtExit */
#define ATEXIT_DEFVAL        0

typedef void (*Void_VoidPtr_F)(void*);
typedef Void_VoidPtr_F V_VP_F;

/* AtExit node */
typedef struct CrtAtExitNode {
  void* arg;       /* Argument, that will be used */
  V_VP_F function; /* Pointer to function         */
} CrtAtExitNode;

/* Header of AtExit nodes */
typedef struct CrtAtExitHeader {
  int capacity;      /* Size of nodes */
  int used;          /* Used nodes    */
  CrtAtExitNode* array; /* Nodes         */
} CrtAtExitHeader;

/* Init AtExitHeader.
   Return value is pointer to AtExitHeader*
   or NULL if OOM */
CrtAtExitHeader* CrtInitAtExit(int);

/* Set function, that will be called later.
   Return value is 0 if all good and 1 if
   haven't left nodes */
int CrtSetAtExit(CrtAtExitHeader*, V_VP_F, void*);

/* Run and reset all AtExit functions */
void CrtDoAtExit(CrtAtExitHeader*);

/* Malloc data and set AtExit free function
   Return pointer, that gave by malloc */
void* CrtAtExitMalloc(CrtAtExitHeader*, size_t);

/* Open file and set AtExit fclose function
   Return pointer to FILE*, that gave by fopen */
FILE* CrtAtExitFopen(CrtAtExitHeader*, const char*, const char*);

/* Clean all AtExit list.
   Warning: if you have set free, don't call this
   function, you can get memory leak. */
void CrtAtExitClean(CrtAtExitHeader*);

/* Free HEAD. */
void CrtAtExitFree(CrtAtExitHeader*);

/* Stolen from Go */
#define defer SetAtExit

#ifdef ATEXIT_DEBUG
/* Print all malloc' blocks, that not freed. */
void DropAllocated(void);

/* Get number of not freed blocks */
int GetAllocated(void);
#endif

#endif /* LIBCRT_LIBCRT_MEM_ATEXIT_H_ */
/************************/

/* === BEGIN SOURCE === */
#if (defined(ATEXIT_SOURCE) || defined(CRT_SOURCE)) && !defined(WAS_ATEXIT)
#define WAS_ATEXIT

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* debug.c */

#ifdef ATEXIT_DEBUG
/* Google style: bad: created global non const variable */
static int allocated_blocks = 0;
#endif

/* Malloc wrapper: */
static void* MemAlloc(size_t bytes) {
  void* ret = malloc(bytes);
  
#ifdef ATEXIT_DEBUG
  if (ret != NULL) {
    allocated_blocks++;
  }
#endif

  return ret;
}

static void MemFree(void* ptr) {
  free(ptr);
  
#ifdef ATEXIT_DEBUG
  allocated_blocks--;
#endif

  return;
}

#ifdef ATEXIT_DEBUG

void DropAllocated(void) {
  printf("Allocated blocks: %d\n", allocated_blocks);
  return;
}

int GetAllocated(void) {
  return allocated_blocks;
}

#endif

/* atexit.c */

/* fclose() isn't equals void (*func)(void*),
   needed wrapper. */
static void CrtAtExitFClose(void* file) {
  if (file != NULL) {
    fclose((FILE*)file);
  }
}

/* Same for free */
static void CrtAtExitFreeWrapper(void* ptr) {
  if (ptr) {
    free(ptr);
  }
}

/* Check validate of HEADER */
static int BadAtExitHeader(CrtAtExitHeader* header) {
  if (header == NULL) {
    return 1;
  }

  if (header->array == NULL ||
      header->used < 0      ||
      header->capacity <= 0 ||
      header->used > header->capacity) {
    return 1;
  }

  return 0;
}

CrtAtExitHeader* CrtInitAtExit(int size) {
  CrtAtExitHeader* ret = NULL;
  char* slice = NULL;
  int cur_node = 0;
  size_t total_size = 0;
  size_t atexit_size = 0;

  /* No size / Big size error */
  if (size < 0 || size > 10000) {
    return ret;
  }

  /* Default value */
  if (size == 0) {
    size = DEFAULT_ATEXIT_SIZE;
  }

  atexit_size = (sizeof(CrtAtExitHeader) + 7) & (size_t)~7;
  total_size = atexit_size + (sizeof(CrtAtExitNode) * (size_t)size);
  
  /* Getting big slice of memory. */
  slice = (char*)MemAlloc(total_size);
  if (slice == NULL) {
    return NULL;
  }

  ret = (CrtAtExitHeader*)(void*)slice;
  ret->capacity = size;
  ret->used = 0;

  /* slice + header_offset = nodes */
  ret->array = (CrtAtExitNode*)(void*)(slice + atexit_size);
  
  for(; cur_node < size; cur_node++) {
    ret->array[cur_node].function = NULL;
    ret->array[cur_node].arg      = NULL;
  }
  
  return ret;
}

int CrtSetAtExit(CrtAtExitHeader* head, V_VP_F func, void* arg) {
  if (BadAtExitHeader(head) || func == NULL || arg == NULL) {
    return 1;
  }

  if (head->used >= head->capacity) {
    return 1;
  }

  head->array[head->used].function = func;
  head->array[head->used].arg      =  arg;
  head->used++;
  
  return 0;
}

void CrtDoAtExit(CrtAtExitHeader* head) {
  int cur_func = 0;
  
  if (BadAtExitHeader(head)) {
    return;
  }

  /* Using LIFO -> Last In First Out. */
  for(cur_func = head->used - 1; cur_func >= 0; cur_func--) {
    if (head->array[cur_func].function == NULL) {
      continue;
    }

    head->array[cur_func].function(
                            head->array[cur_func].arg);

    head->array[cur_func].function = NULL;
    head->array[cur_func].arg      = NULL;
  }

  head->used = 0;
}

void* CrtAtExitMalloc(CrtAtExitHeader* head, size_t bytes) {
  void* ret = NULL;

  if (BadAtExitHeader(head) || bytes == 0) {
    return NULL;
  }

  ret = malloc(bytes);
  if (ret == NULL) {
    return NULL;
  }

  if (CrtSetAtExit(head, CrtAtExitFreeWrapper, ret) == 1) {
    free(ret);
    return NULL;
  }
  
  return ret;
}

FILE* CrtAtExitFopen(CrtAtExitHeader* head, const char* filename,
                     const char* modes) {
  FILE* ret = NULL;
  if (BadAtExitHeader(head) || filename == NULL || modes == NULL) {
    return NULL;
  }

  ret = fopen(filename, modes);
  if (ret == NULL) {
    return NULL;
  }

  if(CrtSetAtExit(head, CrtAtExitFClose, ret) == 1) {
    fclose(ret);
    return NULL;
  }

  return ret;
}

void CrtAtExitClean(CrtAtExitHeader *head) {
  int cur_node = 0;
  
  if (BadAtExitHeader(head)) {
    return;
  }

  for(; cur_node < head->capacity; cur_node++) {
    head->array[cur_node].function = NULL;
    head->array[cur_node].arg      = NULL;
  }

  head->used = 0;
  
  return;
}

void CrtAtExitFree(CrtAtExitHeader *head) {
  if (BadAtExitHeader(head)) {
    return;
  }

  CrtDoAtExit(head);

  MemFree(head);
}

#undef ATEXIT_SOURCE

#endif /* (ATEXIT_SOURCE || CRT_SOURCE) && !WAS_ATEXIT */
/*************************************************************************/
