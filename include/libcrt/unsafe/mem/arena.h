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

/* -Weverything flag */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic fatal "-Weverything"
#pragma clang diagnostic fatal "-Wpedantic"
#pragma clang diagnostic fatal "-Wall"
#pragma clang diagnostic fatal "-Wextra"

/* Error on 'buffer[i]' */
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
/* Error on "int var = (int)'6'" */
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

/* === BEGIN HEADER === */

#ifndef ARENA_ARENA_H_
#define ARENA_ARENA_H_

/* Debug arena mode: */
#if !defined(ARENA_DEBUG) && !defined(FAST_ARENA)
# error "Define ARENA_DEBUG or FAST_ARENA"
#endif

#include <stddef.h>

typedef void* (*AllocFunc)(size_t);
typedef void (*ResetFunc)(void);

/* struct Arena */
typedef struct Arena {
  char* data;        /* Arena data          */
  size_t capacity;   /* Total size of arena */
  size_t offset;     /* Offset of arena     */
#ifdef ARENA_DEBUG
  size_t peak_usage;  /* Peak usage        */
  size_t corruptions; /* Corruptions       */
#endif
  AllocFunc Alloc;    /* Internal allocator */
  ResetFunc Reset;    /* Internal reset     */
} Arena;

/* Initial function for arena */
Arena* InitArena(size_t);

/* Alloc memory from arena */
void* ArenaAlloc(Arena*, size_t);

/* Reset arena (free all objects) */
void ArenaReset(Arena*);

/* Free arena */
void FreeArena(Arena*);

#endif /* ARENA_ARENA_H_ */
/************************/

/* === BEGIN SOURCE === */

#ifdef ARENA_SOURCE

/* Debug arena mode: */
#if !defined(ARENA_DEBUG) && !defined(FAST_ARENA)
# error "Define ARENA_DEBUG or FAST_ARENA"
#endif

#include <stdlib.h>

static int BadArena(Arena* arena) {
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

Arena* InitArena(size_t init_cap) {
  char* slice = NULL;
  Arena* arena = NULL;
  size_t total_size = 0;
  size_t arena_size = 0;
#ifdef ARENA_DEBUG
  size_t cur_byte = 0;
#endif

  if (init_cap == 0) {
    return NULL;
  }

  arena_size = (sizeof(Arena) + 7) & (size_t)~7;
  total_size = arena_size + init_cap;

  slice = (char*)malloc(total_size);
  if (slice == NULL) {
    return NULL;
  }

  arena = (Arena*)(void*)slice;
  
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

void* ArenaAlloc(Arena* arena, size_t bytes) {
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

void ArenaReset(Arena* to_reset) {
#ifdef ARENA_DEBUG
  size_t cur_byte = 0;
#endif
  
  if (BadArena(to_reset)) {
    return;
  }

#ifdef ARENA_DEBUG
  if (to_reset->offset < to_reset->capacity) {
    for(cur_byte = to_reset->offset;
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

void FreeArena(Arena* ptr) {
  if (ptr) {
    free(ptr);
  }
}

#endif /* ARENA_SOURCE */
/************************/

/* === BEGIN BIND GENERATION === */
#ifdef NEEDED_BIND
#undef NEEDED_BIND

#define GEN_BIND(id) \
static Arena* InternalGetterID##id(int mode, Arena* ptr) { \
  static Arena* storage = NULL; \
  if (mode) { \
    return storage; \
  } \
  \
  storage = ptr; \
  return storage; \
} \
\
static void* InternalAllocID##id(size_t bytes) { \
  return ArenaAlloc(InternalGetterID##id(1, NULL), bytes); \
} \
\
static void InternalResetID##id(void) { \
  ArenaReset(InternalGetterID##id(1, NULL)); \
} \
\
static int BindID##id(Arena* arena) { \
  if (BadArena(arena)) { \
    return 1; \
  } \
  InternalGetterID##id(0, arena); \
  arena->Alloc = InternalAllocID##id; \
  arena->Reset = InternalResetID##id; \
  \
  return 0; \
}

#endif /* NEEDED_BIND */
/*********************************/

/* -Weverything flag */
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
