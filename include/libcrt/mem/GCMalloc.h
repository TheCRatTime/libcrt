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

#ifndef LIBCRT_LIBCRT_MEM_GC_MALLOC_H_
#define LIBCRT_LIBCRT_MEM_GC_MALLOC_H_

#include <stddef.h>

#define GC_NOT_MARKED 0
#define GC_MARKED     1
#define GC_MARKED_BAD 2

#define GC_MAGIC_TOTAL 4
#define GC_MAGIC1 0xCC
#define GC_MAGIC2 0xFF
#define GC_MAGIC3 0xDD
#define GC_MAGIC4 0xAA

/* GC unsigned char alias */
typedef unsigned char GCu8;

/* GC destructor function */
typedef void (*GCDestructor)(void*);

/* GCObject struct */
typedef struct GCObject {
  struct GCObject* next;
  struct GCObject* next_root;
  GCDestructor destructor;
  char magic[GC_MAGIC_TOTAL];
  GCu8 marked;
} GCObject;

/* GC state struct */
typedef struct GCState {
  GCObject* roots;
  GCObject* all_ptrs;
} GCState;

/* Global GC state */
extern GCState gGCState;

/* Allocate memory.
   Return value is malloc() function like. */
void* GCMalloc(size_t size);

/* Collect garbage in program */
void RunGC(void);

/* Register destructor that will be called
   when PTR is free */
void GCRegDestructor(void* ptr, GCDestructor dsr);

#define GC_START 0
#define GC_DEPS  1
#define GC_SWEEP 2

#define GC_INIT_FRAME(name) \
  GCObject* name = gGCState.roots

#define GC_DEINIT_FRAME(name) \
  gGCState.roots = name

#define GC_INIT() \
  GCObject* gc__end_roots = gGCState.roots

#define GC_DEINIT() \
  gGCState.roots = gc__end_roots

#define GCreturn \
  GC_DEINIT(); return

#endif /* LIBCRT_LIBCRT_MEM_GC_MALLOC_H_ */

#if (defined(GC_MALLOC_SOURCE) || defined(CRT_SOURCE)) && !defined(WAS_GC_MALLOC)
#define WAS_GC_MALLOC

#include <stdlib.h>

#ifdef GC_THOUGHTS
# define DEBUG
#endif

#ifdef DEBUG
# include <stdio.h>
# define GC_MSG(msg)          printf(msg)
# define GC_MSG_PTR(msg, ptr) printf(msg, ptr)
#else
# define GC_MSG(msg)          (void)msg
# define GC_MSG_PTR(msg, ptr) (void)ptr
#endif

#ifdef GC_THOUGHTS
#  define GC_THOUGHT(msg)     printf(msg)
# else
#  define GC_THOUGHT(msg)     (void)msg
# endif

GCState gGCState = {NULL, NULL};

#define GCGetRaw(obj) (void*)((char*)(obj) + sizeof(GCObject))

#define GCMark(obj) do { \
  if (obj) { \
    obj->marked = GC_MARKED; \
  } \
} while (0)

#define GCUnmark(obj) do { \
  if ((obj)) { \
    (obj)->marked = GC_NOT_MARKED; \
  } \
} while (0)

static int GCInvalidObj(GCObject* obj) {
  if (!obj ||
      obj->magic[0] != GC_MAGIC1 ||
      obj->magic[1] != GC_MAGIC2 ||
      obj->magic[2] != GC_MAGIC3 ||
      obj->magic[3] != GC_MAGIC4 ||
      obj->marked >= GC_MARKED_BAD) {
    return 1;
  }

  return 0;
}

static void GCInitMagic(GCObject* obj) {
  if (obj) {
    obj->magic[0] = GC_MAGIC1;
    obj->magic[1] = GC_MAGIC2;
    obj->magic[2] = GC_MAGIC3;
    obj->magic[3] = GC_MAGIC4;
  }
}

static void GCRegisterRoot(GCObject* obj) {
  if (GCInvalidObj(obj)) {
    return;
  }

  obj->next_root = gGCState.roots;
  gGCState.roots = obj;
}

static void GCInitObject(GCObject* obj) {
  if (obj) {
    GCInitMagic(obj);
    GCUnmark(obj);
    obj->destructor = NULL;
  }
}

void* GCMalloc(size_t size) {
  void* ret = NULL;
  GCObject* as_obj = NULL;
  
  if (size == 0) {
    return NULL;
  }

  ret = malloc(sizeof(GCObject) + size);
  if (!ret) {
    return NULL;
  }

  as_obj = (GCObject*)ret;
  GCInitObject(as_obj);

  as_obj->next = gGCState.all_ptrs;
  gGCState.all_ptrs = as_obj;

  GCRegisterRoot(as_obj);

  GC_MSG_PTR("GCMalloc: new pointer: %p\n", GCGetRaw(as_obj));
  GC_THOUGHT("GCMalloc: a new garbage?\n");

  return GCGetRaw(as_obj);
}

static void GCSweepStage(void) {
  GCObject** cur = &gGCState.all_ptrs;

  while (*cur != NULL) {
    if ((*cur)->marked != GC_MARKED) {
      GCObject* saved = *cur;

      GC_MSG_PTR("GC: pointer %p ", GCGetRaw(saved));
      
#ifdef GC_THOUGHTS
      GC_THOUGHT("comes to hell\n");
#else
      GC_MSG("haven't roots\n");
#endif

      *cur = (*cur)->next;

      if (saved->destructor) {
        saved->destructor(GCGetRaw(saved));
      }

      free(saved);
      continue;
    }

    GCUnmark(*cur);

    cur = &(*cur)->next;
  }
}

static void GCRootMarkStage(void) {
  GCObject* cur = NULL;
  cur = gGCState.roots;
  
  for (; cur != NULL; cur = cur->next_root) {
    GCMark(cur);
  }
}

void RunGC(void) {
  /* =-=-=-=-=-= MARK & SWEEP GC =-=-=-=-=-= */
  GC_MSG("GC: running...\n");

  /* 1. Marking living objects */
  GCRootMarkStage();

  /* 2. Deleting dead objects. */
  GCSweepStage();
  
  GC_MSG("GC: end of GC log\n");
  GC_THOUGHT("GC: garbage not stops.\n");
}

void GCRegDestructor(void* ptr, GCDestructor dsr) {
  GCObject* obj = NULL;
  
  if (!ptr || !dsr) {
    return;
  }
  
  obj = (GCObject*)(void*)((char*)ptr - sizeof(GCObject));

  if (GCInvalidObj(obj)) {
    return;
  }

  obj->destructor = dsr;
}

#endif /* GC_MALLOC_SOURCE */
