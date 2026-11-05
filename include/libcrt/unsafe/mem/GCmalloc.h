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

#ifndef GC_MALLOC_GC_MALLOC_H_
#define GC_MALLOC_GC_MALLOC_H_

#include <stddef.h>

#define GC_COLOR_WHITE 0
#define GC_COLOR_GREY  1
#define GC_COLOR_BLACK 2
#define GC_COLOR_BAD   3

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
  struct GCObject* next_grey;
  struct GCObject* next_dep;
  GCDestructor destructor;
  char magic[GC_MAGIC_TOTAL];
  GCu8 metadata;
} GCObject;

/* GC state struct */
typedef struct GCState {
  GCObject* roots;
  GCObject* prev_grey;
  GCObject* cur_grey;
  GCObject* prev_sweep;
  GCObject* cur_sweep;
  GCObject* prev_prev_grey;
  GCObject* greys;
  GCObject* all_ptrs;
  GCu8 state;
  GCu8 changed;
  GCu8 warn;
  GCu8 last_chance;
  GCu8 write_barrier_trigger;
} GCState;

/* Global GC state */
extern GCState gGCState;

/* Allocate memory.
   Return value is malloc() function like. */
void* GCMalloc(size_t size);

/* Allocate memory and set dependency DEP.
   Return value is malloc() function like. */
void* GCMallocDep(size_t size, void* dep);

/* Allocate memory but not register as root.
   Return value is malloc() function like. */
void* GCMallocRaw(size_t size);

/* Add dependency DEP to PTR */
void GCAddDependency(void* ptr, void* dep);

/* Safe write barrier for GC.
   Use before you changing PTR to VAL */
void GCWriteBarrier(void *ptr, void *val);

/* Collect garbage in program */
void RunGC(size_t steps);

/* Register destructor that will be called
   when PTR is free */
void GCRegDestructor(void* ptr, GCDestructor dsr);

#define GC_START 0
#define GC_DEPS  1
#define GC_SWEEP 2

#define GC_INIT() \
  GCObject* gc__end_roots = gGCState.roots

#define GC_DEINIT() \
  gGCState.roots = gc__end_roots

#define GCreturn \
  GC_DEINIT(); return

#define GC_SET_PTR(parent, field, child) do { \
  GCWriteBarrier((void*)(parent), (void*)(child)); \
  (parent)->field = (child); \
} while (0)

#define GC_FULL_RUN() do { \
  int loops = 100; \
  RunGC(1); \
  while (gGCState.state != GC_START && loops-- > 0) { \
    RunGC(999999); \
  } \
} while (0)

#endif /* GC_MALLOC_GC_MALLOC_H_ */

#ifdef GC_MALLOC_SOURCE

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

GCState gGCState = {
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0
};

static GCObject* GCPushObj(GCObject* list, GCObject* obj) {
  if (!obj) {
    return list;
  }

  obj->next = list;
  list = obj;

  return list;
}

static void GCPushGrey(GCObject* obj) {
  if (!obj) {
    return;
  }

  obj->next_grey = gGCState.greys;
  gGCState.greys = obj;
  gGCState.changed = 1;
}

static void* GCGetRaw(GCObject* obj) {
  return (void*)((char*)obj + sizeof(GCObject));
}

static void GCMarkWhite(GCObject* obj) {
  if (obj) {
    obj->metadata = GC_COLOR_WHITE;
  }
}

static void GCMarkGrey(GCObject* obj) {
  if (obj) {
    obj->metadata = GC_COLOR_GREY;
  }
}

static void GCMarkBlack(GCObject* obj) {
  if (obj) {
    obj->metadata = GC_COLOR_BLACK;
  }
}

static int GCInvalidObj(GCObject* obj) {
  if (!obj ||
      obj->magic[0] != GC_MAGIC1 ||
      obj->magic[1] != GC_MAGIC2 ||
      obj->magic[2] != GC_MAGIC3 ||
      obj->magic[3] != GC_MAGIC4 ||
      obj->metadata >= GC_COLOR_BAD) {
    return 1;
  }

  return 0;
}

static void GCInitMagic(GCObject* obj) {
  if (!obj) {
    return;
  }

  obj->magic[0] = GC_MAGIC1;
  obj->magic[1] = GC_MAGIC2;
  obj->magic[2] = GC_MAGIC3;
  obj->magic[3] = GC_MAGIC4;
}

static void GCInitObject(GCObject* obj) {
  if (obj) {
    if (gGCState.state == GC_START) {
      GCMarkWhite(obj);
    } else {
      GCMarkBlack(obj);
    }
    
    GCInitMagic(obj);
    obj->next = NULL;
    obj->next_root = NULL;
    obj->next_grey = NULL;
    obj->next_dep = NULL;
    obj->destructor = NULL;
  }
}

void GCWriteBarrier(void* ptr, void* val) {
  GCObject* obj1 = NULL;
  GCObject* obj2 = NULL;
  
  if (!ptr || !val) {
    return;
  }
  
  obj1 = (GCObject*)(void*)((char*)ptr - sizeof(GCObject));
  obj2 = (GCObject*)(void*)((char*)val - sizeof(GCObject));

  if (GCInvalidObj(obj1) || GCInvalidObj(obj2)) {
    return;
  }

  if (obj1->metadata == GC_COLOR_BLACK && obj2->metadata == GC_COLOR_WHITE) {
    GCMarkGrey(obj2);
    GCPushGrey(obj2);
    gGCState.write_barrier_trigger = 1;
  }
}

static void* GCMallocLogic(size_t size) {
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
  gGCState.all_ptrs = GCPushObj(gGCState.all_ptrs, as_obj);

  GC_MSG_PTR("GCMalloc: new pointer: %p\n", GCGetRaw(as_obj));
  GC_THOUGHT("GCMalloc: a new garbage?\n");

  return GCGetRaw(as_obj);
}

static void GCRegisterRoot(void* ptr) {
  GCObject* obj = NULL;
  
  if (!ptr) {
    return;
  }

  obj = (GCObject*)(void*)((char*)ptr - sizeof(GCObject));
  if (GCInvalidObj(obj)) {
    return;
  }

  obj->next_root = gGCState.roots;
  gGCState.roots = obj;
}

static int GCSweepStage(size_t steps) {
  if (!gGCState.cur_sweep) {
    gGCState.cur_sweep = gGCState.all_ptrs;
  }

  while (gGCState.cur_sweep != NULL && steps-- > 0) {
    if (gGCState.cur_sweep->metadata != GC_COLOR_BLACK) {
      GCObject* saved = gGCState.cur_sweep;

      GC_MSG_PTR("GC: pointer %p ", GCGetRaw(gGCState.cur_sweep));
      
#ifdef GC_THOUGHTS
      GC_THOUGHT("comes to hell\n");
#else
      GC_MSG("haven't roots\n");
#endif
      gGCState.cur_sweep = gGCState.cur_sweep->next;
      if (gGCState.prev_sweep) {
        gGCState.prev_sweep->next = gGCState.cur_sweep;
      }

      if (saved->destructor) {
        saved->destructor(GCGetRaw(saved));
      }

      free(saved);
      continue;
    }
  
    GCMarkWhite(gGCState.cur_sweep);
    if (!gGCState.prev_sweep) {
      gGCState.all_ptrs = gGCState.cur_sweep;
    }

    gGCState.prev_sweep = gGCState.cur_sweep;
    gGCState.cur_sweep = gGCState.cur_sweep->next;
  }

  if (gGCState.cur_sweep) {
    return 1;
  }

  if (gGCState.prev_sweep == NULL) {
    gGCState.all_ptrs = NULL;
  }

  gGCState.cur_sweep = NULL;
  gGCState.prev_sweep = NULL;

  return 0;
}

static void GCDepsHandle(GCObject* obj) {
  GCObject* dep = NULL;
  
  if (!obj || !obj->next_dep) {
    return;
  }

  dep = obj->next_dep;

  if (dep->metadata == GC_COLOR_BLACK) {
    GC_MSG("GC: cycle dependency blocked\n");
    return;
  }
  
  if (dep->next_dep && dep->next_dep->metadata != GC_COLOR_BLACK) {
    GCPushGrey(dep->next_dep);
  }

  GC_MSG_PTR("GC: dependency %p changed to black.\n", GCGetRaw(dep));

  GCMarkBlack(dep);
}

void GCAddDependency(void* ptr, void* dep) {
  GCObject* obj1 = NULL;
  GCObject* obj2 = NULL;
  GCObject* free = NULL;
  
  if (!ptr || !dep) {
    return;
  }
  
  obj1 = (GCObject*)(void*)((char*)ptr - sizeof(GCObject));
  obj2 = (GCObject*)(void*)((char*)dep - sizeof(GCObject));

  if (GCInvalidObj(obj1) || GCInvalidObj(obj2)) {
    return;
  }

  if (obj1->next_dep != NULL) {
    free = obj1->next_dep;
    while (free->next_dep != NULL) {
      free = free->next_dep;
    }
  } else {
    free = obj1;
  }

  GCWriteBarrier(free, dep);
  free->next_dep = obj2;
}

#define GC_WARN_LIMIT 10

static void GCSavePrevPrev(GCObject* cur) {
  gGCState.prev_prev_grey = gGCState.prev_grey;
  gGCState.prev_grey = cur->next_grey;
}

static int GCDepsStage(size_t steps) {
  if (!gGCState.cur_grey) {
    gGCState.cur_grey = gGCState.greys;
  }
  
  while (gGCState.cur_grey != NULL && steps-- > 0) {
    if (gGCState.cur_grey->metadata == GC_COLOR_BLACK) {
      if (gGCState.last_chance) {
        gGCState.cur_grey = gGCState.prev_prev_grey;
      }

      if (gGCState.cur_grey == NULL) {
        GC_MSG("GC: last chance unused\n");
        break;
      }

      gGCState.last_chance = 0;
      gGCState.prev_prev_grey = NULL;
      
      if (gGCState.warn >= GC_WARN_LIMIT) {
        GC_MSG("GC: I get in cycle dependency\n");
        GC_THOUGHT("GC: bug or you?\n");
        gGCState.last_chance = 1;
        continue;
      }
      
      GC_MSG_PTR("GC: pointer %p already black\n",
                 GCGetRaw(gGCState.cur_grey));
      gGCState.warn++;

      gGCState.cur_grey = gGCState.prev_grey ? gGCState.prev_grey
                                             : gGCState.cur_grey->next_grey;

      continue;
    }

    gGCState.changed = 0;
    gGCState.warn = 0;
    
    if (gGCState.cur_grey->next_dep != NULL &&
        gGCState.cur_grey->next_dep->metadata == GC_COLOR_WHITE) {
      GCSavePrevPrev(gGCState.cur_grey);
      GCDepsHandle(gGCState.cur_grey);
    }

    GC_MSG_PTR("GC: pointer %p keep alive\n", GCGetRaw(gGCState.cur_grey));

    GCMarkBlack(gGCState.cur_grey);
    gGCState.cur_grey = (gGCState.changed ? gGCState.greys
                                          : gGCState.cur_grey->next_grey);
  }

  if (gGCState.cur_grey) {
    return 1;
  }

  gGCState.greys = NULL;
  gGCState.cur_grey = NULL;
  gGCState.prev_grey = NULL;
  gGCState.prev_prev_grey = NULL;

  return 0;
}

static void GCRootMarkStage(void) {
  GCObject* cur = NULL;
  cur = gGCState.roots;
  for (; cur != NULL; cur = cur->next_root) {
    GCMarkGrey(cur);
    GCPushGrey(cur);
  }
}

void RunGC(size_t steps) {
  int cont = 0;
  
  /* =-=-=-=-=-= INCREMENTAL TRI-COLOR GC =-=-=-=-=-= */
  GC_MSG("GC: running...\n");

  if (gGCState.state == GC_SWEEP && gGCState.write_barrier_trigger) {
    gGCState.state = GC_DEPS;
    gGCState.cur_sweep = NULL;
    gGCState.prev_sweep = NULL;
  }

  switch (gGCState.state) {
    case GC_START:
    default:
      /* 1. Mark roots */
      GCRootMarkStage();
      gGCState.state = GC_DEPS;
      return;
    case GC_DEPS:
      /* 2. Dependencies mark */
      cont = GCDepsStage(steps);
      if (!cont) {
        gGCState.state = GC_SWEEP;
      }
      
      return;
    case GC_SWEEP:
      /* 3. Sweep stage. */
      cont = GCSweepStage(steps);
      if (!cont) {
        gGCState.state = GC_START;
      }
      
      return;
  }

  GC_MSG("GC: end of GC log\n");
  GC_THOUGHT("GC: garbage not stops.\n");
}

#define GC_ROOT 1
#define GC_DEP  2
#define GC_RAW  3
#define GC_BAD  4

static void* RealGCMalloc(size_t size, GCu8 opt, void* dep) {
  void* ret = NULL;
  
  if (size == 0 || opt >= GC_BAD) {
    return NULL;
  }

  ret = GCMallocLogic(size);
  if (!ret) {
    return NULL;
  }

  if (opt == GC_ROOT) {
    GCRegisterRoot(ret);
  } else if (opt == GC_DEP && dep != NULL) {
    GCAddDependency(ret, dep);
  }

  return ret;
}

void* GCMalloc(size_t size) {
  return RealGCMalloc(size, GC_ROOT, NULL);
}

void* GCMallocDep(size_t size, void* dep) {
  return RealGCMalloc(size, GC_DEP, dep);
}

void* GCMallocRaw(size_t size) {
  return RealGCMalloc(size, GC_RAW, NULL);
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
