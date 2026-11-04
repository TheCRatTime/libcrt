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

/* GCObject struct */
typedef struct GCObject {
  void* raw_ptr;
  struct GCObject* next;
  struct GCObject* next_root;
  struct GCObject* next_grey;
  struct GCObject* next_dep;
  char magic[GC_MAGIC_TOTAL];
  GCu8 metadata;
} GCObject;

extern GCObject* gGCRoots;
extern GCObject* gGCGreys;
extern GCObject* gGCAllPtrs;

/* Allocate memory.
   Return value is malloc() function like. */
void* GCMalloc(size_t size);

/* Allocate memory and set dependency DEP.
   Return value is malloc() function like. */
void* GCMallocDep(size_t size, void* dep);

/* Allocate memory but not register as root.
   Return value is malloc() function like. */
void* GCMallocRaw(size_t size);

/* Allocate memory and set dependency DEP.
   Return value is malloc() function like. */
void* GCMallocDep(size_t size, void* dep);

/* Register pointer that will be saved in
   GC collection. */
void GCRegisterRoot(void* ptr);

/* Unegister pointer that was registered by
   GCRegisterRoot() */
void GCUnregisterRoot(void* ptr);

/* Add dependency DEP to PTR */
void GCAddDependency(void* ptr, void* dep);

/* Collect garbage in program */
void RunGC(void);

#define GC_INIT() \
  GCObject* gc__end_roots = gGCRoots

#define GC_DEINIT() \
  gGCRoots = gc__end_roots

#define GCreturn \
  GC_DEINIT(); return

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

GCObject* gGCRoots = NULL;
GCObject* gGCGreys = NULL;
GCObject* gGCAllPtrs = NULL;

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

  obj->next_grey = gGCGreys;
  gGCGreys = obj;
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

static void GCInitObject(GCObject* obj, void* ptr) {
  if (obj) {
    GCMarkWhite(obj);
    GCInitMagic(obj);
    obj->raw_ptr = ptr;
    obj->next = NULL;
    obj->next_root = NULL;
    obj->next_grey = NULL;
    obj->next_dep = NULL;
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
  GCInitObject(as_obj, (void*)((char*)ret + sizeof(GCObject)));
  gGCAllPtrs = GCPushObj(gGCAllPtrs, as_obj);

  GC_MSG_PTR("GCMalloc: new pointer: %p\n",
             (void*)((char*)ret + sizeof(GCObject)));
  GC_THOUGHT("GCMalloc: a new garbage?\n");

  return (void*)((char*)ret + sizeof(GCObject));
}

void GCRegisterRoot(void* ptr) {
  GCObject* obj = NULL;
  
  if (!ptr) {
    return;
  }

  obj = (GCObject*)(void*)((char*)ptr - sizeof(GCObject));
  if (GCInvalidObj(obj)) {
    return;
  }

  obj->next_root = gGCRoots;
  gGCRoots = obj;
}

void GCUnregisterRoot(void* ptr) {
  GCObject* prev = NULL;
  GCObject* cur = gGCRoots;

  if (!ptr || !cur) {
    return;
  }

  while (cur != NULL) {
    if (cur->raw_ptr == ptr) {
      if (!prev) {
        gGCRoots = cur->next_root;
      } else {
        prev->next_root = cur->next_root;
      }
      break;
    }

    if (!prev) {
      gGCRoots = cur;
    }

    prev = cur;
    cur = cur->next_root;
  }

  if (!prev) {
    gGCRoots = NULL;
  }
}

static void GCSweepStage(void) {
  GCObject* prev = NULL;
  GCObject* cur = gGCAllPtrs;

  while (cur != NULL) {
    if (cur->metadata != GC_COLOR_BLACK) {
      GCObject* saved = cur;

      GC_MSG_PTR("GC: pointer %p ", cur->raw_ptr);
      
#ifdef GC_THOUGHTS
      GC_THOUGHT("comes to hell\n");
#else
      GC_MSG("haven't roots\n");
#endif
      cur = cur->next;
      if (prev) {
        prev->next = cur;
      }

      free(saved);
      continue;
    }
  
    GCMarkWhite(cur);
    if (!prev) {
      gGCAllPtrs = cur;
    }

    prev = cur;
    cur = cur->next;
  }

  if (prev == NULL) {
    gGCAllPtrs = NULL;
  }
}

static int GCDepsHandle(GCObject* obj) {
  GCObject* dep = NULL;
  int changed = 0;
  
  if (!obj) {
    return 0;
  }

  dep = obj->next_dep;

  if (dep->metadata == GC_COLOR_BLACK) {
    GC_MSG("GC: cycle dependency blocked\n");
    return 0;
  }
  
  if (dep->next_dep && dep->next_dep->metadata != GC_COLOR_BLACK) {
    GCPushGrey(dep->next_dep);
    changed = 1;
  }

  GC_MSG_PTR("GC: dependency %p changed to black.\n", dep->raw_ptr);

  GCMarkBlack(dep);

  return changed;
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

  free->next_dep = obj2;
}

#define GC_WARN_LIMIT 10

static void GCDepsStage(void) {
  int changed = 0;
  GCObject* cur = gGCGreys;
  GCObject* prev = NULL;
  int warn = 0;

  while (cur != NULL) {
    if (cur->metadata == GC_COLOR_BLACK) {
      if (warn >= GC_WARN_LIMIT) {
        GC_MSG("GC: I get in cycle dependency\n");
        GC_THOUGHT("GC: bug or you?\n");
        break;
      }
      
      GC_MSG_PTR("GC: pointer %p already black\n", cur->raw_ptr);
      cur = cur->next_grey;
      warn++;

      if (prev) {
        cur = prev;
      }

      continue;
    }
    
    changed = 0;
    warn = 0;
    
    if (cur->next_dep != NULL && cur->next_dep->metadata == GC_COLOR_WHITE) {
      prev = cur->next_grey;
      changed = GCDepsHandle(cur);
    }

    GC_MSG_PTR("GC: pointer %p keep alive\n", cur->raw_ptr);

    GCMarkBlack(cur);
    cur = (changed ? gGCGreys : cur->next_grey);
  }

  gGCGreys = NULL;
}

void RunGC(void) {
  /* =-=-=-=-=-= TRI-COLOR GC =-=-=-=-=-= */
  GCObject* cur = NULL;

  GC_MSG("GC: running...\n");

  /* 1. Mark roots */
  cur = gGCRoots;
  for (; cur != NULL; cur = cur->next_root) {
    GCMarkGrey(cur);
    GCPushGrey(cur);
  }

  /* 2. Dependencies mark */
  GCDepsStage();

  /* 3. Sweep stage. */
  GCSweepStage();

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

#endif /* GC_MALLOC_SOURCE */
