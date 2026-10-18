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

#ifndef LIBCRT_LIBCRT_MEM_ARENA_H_
#define LIBCRT_LIBCRT_MEM_ARENA_H_

/* Debug arena mode: */
#if !defined(ARENA_DEBUG) && !defined(FAST_ARENA)
# error "Define ARENA_DEBUG or FAST_ARENA"
#endif

#include <stddef.h>

typedef void* (*CrtAllocFunc)(size_t);
typedef void (*CrtResetFunc)(void);

/* struct Arena */
typedef struct CrtArena {
  char* data;        /* Arena data          */
  size_t capacity;   /* Total size of arena */
  size_t offset;     /* Offset of arena     */
#ifdef ARENA_DEBUG
  size_t peak_usage;  /* Peak usage        */
  size_t corruptions; /* Corruptions       */
#endif
  CrtAllocFunc Alloc;    /* Internal allocator */
  CrtResetFunc Reset;    /* Internal reset     */
} CrtArena;

/* Initial function for arena */
CrtArena* CrtInitArena(size_t);

/* Alloc memory from arena */
void* CrtArenaAlloc(CrtArena*, size_t);

/* Reset arena (free all objects) */
void CrtArenaReset(CrtArena*);

/* Free arena */
void CrtFreeArena(CrtArena*);

#endif /* LIBCRT_LIBCRT_MEM_ARENA_H_ */
/********************************/

/* === BEGIN SOURCE === */

#if (defined(ARENA_SOURCE) || defined(CRT_SOURCE)) && !defined(WAS_ARENA)
#define WAS_ARENA

/* Debug arena mode: */
#if !defined(ARENA_DEBUG) && !defined(FAST_ARENA)
# error "Define ARENA_DEBUG or FAST_ARENA"
#endif

#include <stdlib.h>

static int BadArena(CrtArena* arena) {
  if (arena == NULL) {
    return 1;
  }

  if (arena->capacity == 0            ||
      arena->data == NULL             ||
      arena->offset > arena->capacity) {
    return 1;
  }

  return 0;
}

CrtArena* CrtInitArena(size_t init_cap) {
  char* slice = NULL;
  CrtArena* arena = NULL;
  size_t total_size = 0;
  size_t arena_size = 0;
#ifdef ARENA_DEBUG
  size_t cur_byte = 0;
#endif

  if (init_cap == 0) {
    return NULL;
  }

  arena_size = (sizeof(CrtArena) + 7) & (size_t)~7;
  total_size = arena_size + init_cap;

  slice = (char*)malloc(total_size);
  if (slice == NULL) {
    return NULL;
  }

  arena = (CrtArena*)(void*)slice;
  
  arena->data = slice + arena_size;

#ifdef ARENA_DEBUG
  for(cur_byte = 0; cur_byte < init_cap; cur_byte++) {
    arena->data[cur_byte] = 0;
  }
#endif
  
  arena->capacity     = init_cap;
#ifdef ARENA_DEBUG
  arena->peak_usage  = 0;
  arena->corruptions = 0;
#endif
  arena->offset      = 0;

  return arena;
}

void* CrtArenaAlloc(CrtArena* arena, size_t bytes) {
  char* ret = NULL;
  size_t total_size = 0;
#ifdef ARENA_DEBUG
  size_t cur_byte = 0;
#endif
  
  if (BadArena(arena)) { 
    return NULL;
  }

  if (bytes == 0 || arena->offset == arena->capacity) {
    return NULL;
  }
  
  /* 8-byte alignment */
  total_size = ((bytes + 7) & (size_t)~7);
  if (arena->offset+total_size > arena->capacity) {
    return NULL;
  }

#ifdef ARENA_DEBUG
  /* Corruption check */
  for(cur_byte = 0; cur_byte < total_size; cur_byte++) {
    if (arena->data[cur_byte+arena->offset] != 0) {
      arena->corruptions++;
      break;
    }
  }
#endif

  ret = &arena->data[arena->offset];
  arena->offset += total_size;

#ifdef ARENA_DEBUG
  if (arena->offset > arena->peak_usage) {
    arena->peak_usage = arena->offset;
  }
#endif
  
  return ret;
}

void CrtArenaReset(CrtArena* to_reset) {
#ifdef ARENA_DEBUG
  size_t cur_byte = 0;
#endif
  
  if (BadArena(to_reset)) {
    return;
  }

#ifdef ARENA_DEBUG
  if (to_reset->offset < to_reset->capacity) {
    for (cur_byte = to_reset->offset;
        cur_byte < to_reset->capacity; cur_byte++) {
      if (to_reset->data[cur_byte] != 0) {
        to_reset->corruptions++;
        break;
      }
    }
  }

  for(cur_byte = 0; cur_byte < to_reset->capacity; cur_byte++) {
    to_reset->data[cur_byte] = 0;
  }
#endif

  to_reset->offset = 0;
}

void CrtFreeArena(CrtArena* ptr) {
  if (ptr) {
    free(ptr);
  }
}

#undef ARENA_SOURCE

#endif /* (ARENA_SOURCE || CRT_SOURCE) && !WAS_ARENA */
/***********************************************************************/

/* === BEGIN BIND GENERATION === */

#ifdef NEEDED_BIND
#undef NEEDED_BIND

#define GEN_BIND(id) \
static CrtArena* CrtInternalGetterID##id(int mode, CrtArena* ptr) { \
  static CrtArena* storage = NULL; \
  if (mode) { \
    return storage; \
  } \
  \
  storage = ptr; \
  return storage; \
} \
\
static void* CrtInternalAllocID##id(size_t bytes) { \
  return CrtArenaAlloc(CrtInternalGetterID##id(1, NULL), bytes); \
} \
\
static void CrtInternalResetID##id(void) { \
  CrtArenaReset(CrtInternalGetterID##id(1, NULL)); \
} \
\
static int CrtBindID##id(CrtArena* arena) { \
  if (BadArena(arena)) { \
    return 1; \
  } \
  CrtInternalGetterID##id(0, arena); \
  arena->Alloc = CrtInternalAllocID##id; \
  arena->Reset = CrtInternalResetID##id; \
  \
  return 0; \
}

#endif /* NEEDED_BIND */
/*********************************/
