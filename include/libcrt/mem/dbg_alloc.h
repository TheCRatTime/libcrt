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

#ifndef LIBCRT_LIBCRT_MEM_DBG_ALLOC_H_
#define LIBCRT_LIBCRT_MEM_DBG_ALLOC_H_

#include <stddef.h>

#define CANARY_VALUE 0x43 /* Canary value */
#define CANARY_SIZE  8    /* Canary size */

#define POISON_VALUE 0xCC /* Poison value */
#define POISON_SIZE  8    /* Poison size */

#define SH_MEM_VALID 0x00 /* Shadow memory valid. */
#define SH_MEM_FREED 0xFD /* Shadow memory freed. */
#define SH_MEM_BAD   0xBD /* Shadow memory bad. */

#define DBG_MAGIC_1 0x01 /* Magic 1 */
#define DBG_MAGIC_2 0x02 /* Magic 2 */
#define DBG_MAGIC_3 0x03 /* Magic 3 */
#define DBG_MAGIC_4 0x04 /* Magic 4 */

typedef unsigned char CrtDbgUChar;

/* Internal header of memory data */
typedef struct CrtDbgSymbol {
  CrtDbgUChar magic[4];   /* Magic characters */

  CrtDbgUChar freed;      /* Was block freed */
  CrtDbgUChar padding[3]; /* Padding from 1 to 4 */
  
  size_t real_size;       /* Size of allocated data */
} CrtDbgSymbol;

/* Pointer list. */
typedef struct CrtPtrLinkedList {
  void* ptr;                     /* Real pointer */
  struct CrtPtrLinkedList* next; /* Next pointer */
} CrtPtrLinkedList;

/* Chunk of memory */
typedef struct CrtMemChunk {
  char* memory;       /* Pointer to memory */
  size_t memory_size; /* Total memory size */
  size_t left_bytes;  /* Left bytes of memory */
} CrtMemChunk;

/* Allocator context. */
typedef struct CrtAllocatorCtx {
  void* self_ptr;                /* For free */
  size_t total_allocated;        /* Total allocated pointers */
  size_t total_freed;            /* Total freed pointers */

  CrtPtrLinkedList* head_of_pll; /* Head of all pointers */
  char* pll_memory;              /* PtrLinkedList memory */
  size_t pll_memory_size;        /* PtrLinkedList memory size */
  size_t pll_left;               /* PtrLinkedList left bytes */
  CrtPtrLinkedList* cur_pll;     /* Current PtrLinkedList* node */
  CrtDbgUChar have_overflow;     /* Overflow check. */

  char* sh_memory;               /* Shadow memory */
  size_t sh_memory_size;         /* Shadow memory size */
  
  CrtMemChunk* memory;           /* Memory for allocate */
} CrtAllocatorCtx;

/* Initial function for allocator
   Returns 0 on success, -1 at error. */
int CrtInitAllocator(size_t);

/* Allocate memory.
   Returns pointer to data on success, NULL at error */
void* CrtDbgAlloc(size_t);

/* Free memory, that allocated by DbgAlloc */
void CrtDbgFree(void*);

#ifdef WRAP_REAL_MALLOC
# define DEFER_WRAP_MALLOC
#endif

/* Check pointer for corruption. */
void CrtCheckPtr(void*);

/* Print log about all information */
void CrtAllocatorLog(void);

/* Clear debug allocator */
void CrtKillAllocator(void);

/* Check all pointers in allocator. */
void CrtAllocatorFullCheck(void);

/* Check pointer for use now. */
void CrtCheckForUse(void*);

/* Get shadow memory offset for PTR */
size_t CrtGetShMemOffset(void*);

/* Get count bytes of shadow memory. */
size_t CrtGetShMemCountBytes(size_t);

/* Check all bytes in PTR with size SIZE */
#define SH_MEM_CHECK(ptr, size) do { \
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL); \
  size_t offset = CrtGetShMemOffset(ptr); \
  size_t count = CrtGetShMemCountBytes(size); \
  size_t i = 0; \
  \
  if (ptr == NULL) { \
    break; \
  } \
  \
  for (; i < count; i++) { \
    if (ctx->sh_memory[offset+i] != SH_MEM_VALID) { \
      printf("SH_MEM_CHECK: *** pointer to a canary/debug info/freed block " \
             "***\n=== aborted (core dumped) ===\n"); \
      abort(); \
    } \
  } \
} while(0)

#endif /* LIBCRT_LIBCRT_MEM_DBG_ALLOC_H_ */

#if (defined(DBG_ALLOC_SOURCE) || defined(CRT_SOURCE)) && \
    !defined(WAS_DBG_ALLOC)
#define WAS_DBG_ALLOC

#include <stdio.h>
#include <stdlib.h>

static CrtAllocatorCtx* AllocatorCtxManager(int mode, CrtAllocatorCtx* ctx) {
  static CrtAllocatorCtx* internal_ctx = NULL;
  if (mode == 1) {
    internal_ctx = ctx;
  }

  return internal_ctx;
}

static CrtPtrLinkedList* PLLAllocate(CrtAllocatorCtx* ctx) {
  size_t aligned = (sizeof(CrtPtrLinkedList) + 7) & (size_t)~7;
  CrtPtrLinkedList* pll = NULL;
  
  if (ctx == NULL) {
    return NULL;
  }

  if (ctx->pll_left < aligned) {
    return NULL;
  }

  pll = (CrtPtrLinkedList*)(void*)&ctx->
    pll_memory[ctx->pll_memory_size - ctx->pll_left];

  ctx->pll_left -= aligned;

  pll->ptr = NULL;
  pll->next = NULL;

  return pll;
}

static void FillShadowMem(char* sh_mem, size_t size) {
  size_t i = 0;
  
  for (; i < size; i++) {
    sh_mem[i] = SH_MEM_VALID;
  }
}

int CrtInitAllocator(size_t mem_size) {
  size_t aligned1 = 0;
  size_t aligned2 = 0;
  size_t aligned3 = 0;
  size_t aligned4 = 0;
  size_t aligned5 = 0;
  size_t total = 0;
  size_t for_ptrs = 0;
  char* slice = NULL;
  CrtAllocatorCtx* allocator_ctx = NULL;
  CrtMemChunk* chunk = NULL;
  
  if (mem_size == 0) {
    return -1;
  }

  for_ptrs = (mem_size >> 3) + (mem_size & 7 ? 1 : 0);

  aligned1 = (sizeof(CrtAllocatorCtx) + 7) & (size_t)~7;
  aligned2 = (sizeof(CrtMemChunk) + 7) & (size_t)~7;
  aligned3 = (mem_size + 7) & (size_t)~7;
  aligned4 = (for_ptrs + 7) & (size_t)~7;
  aligned5 = ((aligned3 >> 3) + 7) & (size_t)~7;
  total = aligned1 + aligned2 + aligned3 + aligned4 + aligned5;

  slice = (char*)malloc(total);

  if (slice == NULL) {
    return -1;
  }

  allocator_ctx = (CrtAllocatorCtx*)(void*)slice;
  
  chunk = (CrtMemChunk*)(void*)(slice + aligned1);
  chunk->memory = (slice + aligned1 + aligned2);
  chunk->memory_size = aligned3;
  chunk->left_bytes = aligned3;

  allocator_ctx->memory = chunk;
  allocator_ctx->pll_memory = (slice + aligned1 + aligned2 + aligned3);
  allocator_ctx->pll_memory_size = aligned4;
  allocator_ctx->pll_left = aligned4;
  allocator_ctx->total_allocated = 0;
  allocator_ctx->total_freed = 0;
  allocator_ctx->self_ptr = (void*)slice;
  allocator_ctx->head_of_pll = PLLAllocate(allocator_ctx);
  allocator_ctx->cur_pll = allocator_ctx->head_of_pll;
  allocator_ctx->have_overflow = 0;

  allocator_ctx->sh_memory = slice + aligned1 + aligned2 + aligned3 + aligned4;
  allocator_ctx->sh_memory_size = aligned5;

  FillShadowMem(allocator_ctx->sh_memory, aligned5);

  (void)AllocatorCtxManager(1, allocator_ctx);
  
  return 0;
}

static void* ChunkAlloc(CrtMemChunk* chunk, size_t bytes) {
  if (chunk == NULL) {
    return NULL;
  }

  if (chunk->left_bytes < bytes) {
    return NULL;
  }

  chunk->left_bytes -= bytes;

  return (void*)&chunk->memory[chunk->memory_size - (chunk->left_bytes+bytes)];
}

void* CrtDbgAlloc(size_t bytes) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  
  size_t aligned = (bytes + 7) & (size_t)~7;
  size_t canary = ((CANARY_SIZE << 1) + 7) & (size_t)~7;
  size_t symbol = (sizeof(CrtDbgSymbol) + 7) & (size_t)~7;
  
  size_t total = (symbol + aligned + canary + 7) & (size_t)~7;
  
  char* mem = NULL;
  size_t i = 0;
  CrtDbgSymbol* header = NULL;
  void* ret = NULL;
  char* tmp = NULL;
  size_t sh_mem_offset = 0;
  size_t sh_mem_count = 0;
  
  if (bytes == 0) {
    return NULL;
  }

  if (ctx == NULL) {
    printf("CrtDbgAlloc: *** context not initialized ***\n"
           "=== aborted ===\n");
    abort();
  }

  if (aligned == 0) {
    printf("CrtDbgAlloc: *** size integer overflow ***\n"
           "=== aborted ===\n");
    abort();
  }

  mem = (char*)ChunkAlloc(ctx->memory, total);
  if (mem == NULL) {
    return NULL;
  }

  header = (CrtDbgSymbol*)(void*)mem;
  header->magic[0] = DBG_MAGIC_1;
  header->magic[1] = DBG_MAGIC_2;
  header->magic[2] = DBG_MAGIC_3;
  header->magic[3] = DBG_MAGIC_4;
  header->freed = 0;
  header->real_size = aligned;
  
  sh_mem_offset = (size_t)(((char*)header - ctx->memory->memory) >> 3);
  sh_mem_count = ((sizeof(CrtDbgSymbol) + 7) & (size_t)~7) >> 3;

  for (; i < sh_mem_count; i++) {
    ctx->sh_memory[sh_mem_offset+i] = SH_MEM_BAD;
  }

  tmp = mem + symbol;
  
  sh_mem_offset = (size_t)((tmp - ctx->memory->memory) >> 3);
  sh_mem_count = ((CANARY_SIZE + 7) & (size_t)~7) >> 3;
  
  for (i = 0; i < sh_mem_count; i++) {
    ctx->sh_memory[sh_mem_offset+i] = SH_MEM_BAD;
  }

  for (i = 0; i < CANARY_SIZE; i++) {
    *(tmp++) = CANARY_VALUE;
  }

  tmp = mem + symbol + CANARY_SIZE + aligned;

  sh_mem_offset = (size_t)((tmp - ctx->memory->memory) >> 3);
  sh_mem_count = ((CANARY_SIZE + 7) & (size_t)~7) >> 3;
  
  for (i = 0; i < sh_mem_count; i++) {
    ctx->sh_memory[sh_mem_offset+i] = SH_MEM_BAD;
  }

  for (i = 0; i < CANARY_SIZE; i++) {
    *(tmp++) = CANARY_VALUE;
  }

  ret = (void*)(mem + symbol + CANARY_SIZE);

  ctx->total_allocated++;

  if (!ctx->have_overflow) {
    ctx->cur_pll->ptr = ret;
    ctx->cur_pll->next = PLLAllocate(ctx);
    if (ctx->cur_pll->next == NULL) {
      ctx->have_overflow = 1;
    } else {
      ctx->cur_pll = ctx->cur_pll->next;
    }
  }

  return ret;
}

void CrtDbgFree(void* ptr) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  CrtDbgSymbol* debuginfo = NULL;
  char* canary1 = NULL;
  char* canary2 = NULL;
  size_t i = 0;
  size_t sh_mem_offset = 0;
  size_t sh_mem_count = 0;

  if (ptr == NULL) {
    return;
  }

  if (ctx == NULL) {
    printf("CrtDbgFree: *** not set context ***\n=== abort ===\n");
    abort();
  }

  if ((char*)ptr < (ctx->memory->memory + ((sizeof(CrtDbgSymbol) + 7)
                                           & (size_t)~7)) ||
      (char*)ptr >= (ctx->memory->memory + ctx->memory->memory_size - 1)) {
    printf("CrtDbgFree: *** dangling pointer detected ***\n"
           "(pointer: %p)\n"
           "=== aborted (core dumped) ===\n", ptr);
    abort();
  }

  debuginfo = (CrtDbgSymbol*)(void*)((char*)ptr - ((sizeof(CrtDbgSymbol)+7) &
                                                    (size_t)~7) - CANARY_SIZE);

  if (debuginfo->magic[0] != DBG_MAGIC_1 ||
      debuginfo->magic[1] != DBG_MAGIC_2 ||
      debuginfo->magic[2] != DBG_MAGIC_3 ||
      debuginfo->magic[3] != DBG_MAGIC_4) {
    printf("CrtDbgFree: *** heap corruption (invalid magic) ***\n"
           "=== aborted (core dumped) ===\n");
    abort();
  }

  if (debuginfo->freed) {
    printf("CrtDbgFree: *** double free detected ***\n"
           "(pointer: %p)\n"
           "=== aborted (core dumped) ===\n", ptr);
    abort();
  }

  sh_mem_offset = (size_t)(((char*)ptr - ctx->memory->memory) >> 3);
  sh_mem_count = (debuginfo->real_size) >> 3;

  for (; i < sh_mem_count; i++) {
    if (ctx->sh_memory[sh_mem_offset+i] != SH_MEM_VALID) {
      printf("CrtDbgFree: *** pointer to a canary/debug info/freed block ***\n"
             "(pointer: %p)\n"
             "=== aborted (core dumped) ===\n", ptr);
      abort();
    }
  }

  canary1 = (char*)((char*)ptr - CANARY_SIZE);
  canary2 = (char*)((char*)ptr + debuginfo->real_size);

  for (i = 0; i < CANARY_SIZE; i++) {
    if (canary1[i] != CANARY_VALUE || canary2[i] != CANARY_VALUE) {
      printf("CrtDbgFree: *** stack smashing detected ***\n"
             "(pointer: %p)\n"
             "=== aborted (core dumped) ===\n", ptr);
      abort();
    }
  }
  
  debuginfo->freed = 1;

  for (i = 0; i < POISON_SIZE; i++) {
    ((char*)ptr)[i] = POISON_VALUE;
  }
  
  for (i = 0; i < sh_mem_count; i++) {
    ctx->sh_memory[sh_mem_offset+i] = SH_MEM_FREED; 
  }

  ctx->total_freed++;
}

void CrtCheckPtr(void* ptr) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  CrtDbgSymbol* debuginfo = NULL;
  char* canary1 = NULL;
  char* canary2 = NULL;
  int i = 0;

  if (ptr == NULL) {
    return;
  }

  if (ctx == NULL) {
    printf("CrtCheckPtr: *** not set context ***\n=== abort ===\n");
    abort();
  }

  if ((char*)ptr < (ctx->memory->memory + ((sizeof(CrtDbgSymbol) + 7)
                                           & (size_t)~7)) ||
      (char*)ptr >= (ctx->memory->memory + ctx->memory->memory_size - 1)) {
    printf("CrtCheckPtr: *** dangling pointer detected ***\n"
           "(pointer: %p)\n"
           "=== aborted (core dumped) ===\n", ptr);
    abort();
  }

  debuginfo = (CrtDbgSymbol*)(void*)((char*)ptr - ((sizeof(CrtDbgSymbol) + 7) &
                                                   (size_t)~7) - CANARY_SIZE);

  if (debuginfo->magic[0] != DBG_MAGIC_1 ||
      debuginfo->magic[1] != DBG_MAGIC_2 ||
      debuginfo->magic[2] != DBG_MAGIC_3 ||
      debuginfo->magic[3] != DBG_MAGIC_4) {
    printf("CrtCheckPtr: *** heap corruption (invalid magic) ***\n"
           "=== aborted (core dumped) ===\n");
    abort();
  }

  canary1 = (char*)((char*)ptr - CANARY_SIZE);
  canary2 = (char*)((char*)ptr + debuginfo->real_size);

  for (; i < CANARY_SIZE; i++) {
    if (canary1[i] != CANARY_VALUE || canary2[i] != CANARY_VALUE) {
      printf("CrtCheckPtr: *** stack smashing detected ***\n"
           "(pointer: %p)\n"
             "=== aborted (core dumped) ===\n", ptr);
      abort();
    }
  }
}

void CrtAllocatorLog(void) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  CrtDbgSymbol* debuginfo = NULL;
  CrtPtrLinkedList* cur_pll = NULL;

  if (ctx == NULL) {
    return;
  }

  cur_pll = ctx->head_of_pll;

  printf("=== ALLOCATOR DUMP ===\n");

  while (cur_pll != NULL && cur_pll->ptr != NULL) {
    printf("--- pointer: %p ---\n", cur_pll->ptr);
    debuginfo = (CrtDbgSymbol*)(void*)((char*)cur_pll->ptr -
                                       ((sizeof(CrtDbgSymbol) + 7) &
                                        (size_t)~7) - CANARY_SIZE);
    printf("size: %lu\n", (unsigned long)debuginfo->real_size);
    printf("freed: %s\n", (debuginfo->freed ? "yes" : "no"));
    printf("magic: %x%x%x%x\n",
           debuginfo->magic[0], debuginfo->magic[1],
           debuginfo->magic[2], debuginfo->magic[3]);
    
    cur_pll = cur_pll->next;
  }

  if (ctx->have_overflow) {
    printf("... (There isn't all pointers) ...\n");
  }

  printf("----------------------\n");

  printf("Allocated blocks: %lu\n"
         "Freed blocks:     %lu\n",
         (unsigned long)ctx->total_allocated,
         (unsigned long)ctx->total_freed);

  printf("======================\n");
}

void CrtKillAllocator(void) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  
  if (ctx == NULL) {
    return;
  }

  CrtAllocatorFullCheck();

  AllocatorCtxManager(1, NULL);

  free(ctx->self_ptr);
}

void CrtAllocatorFullCheck(void) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  CrtPtrLinkedList* pll = NULL;
  
  if (ctx == NULL) {
    return;
  }

  pll = ctx->head_of_pll;

  while (pll != NULL && pll->ptr != NULL) {
    int i = 0;
    
    CrtCheckPtr(pll->ptr);

    if (((CrtDbgSymbol*)(void*)((char*)pll->ptr - CANARY_SIZE -
        ((sizeof(CrtDbgSymbol) + 7) & (size_t)~7)))->freed == 0) {
      printf("CrtCheckPtr: *** memory leak detected ***\n"
             "(pointer: %p)\n"
             "=== aborted (core dumped) ===\n", pll->ptr);
      abort();
    }

    for (; i < POISON_SIZE; i++) {
      if (((char*)(pll->ptr))[i] != POISON_VALUE) {
        printf("CrtCheckPtr: *** Use-After-Free detected (poison corrupted) ***\n"
               "(pointer: %p)\n"
               "=== aborted (core dumped) ===\n", pll->ptr);
        abort();
      }
    }
    
    pll = pll->next;
  }

  if (ctx->total_allocated > ctx->total_freed) {
    printf("CrtCheckPtr: *** memory leak detected ***\n"
           "=== aborted (core dumped) ===\n");
    abort();
  }
}

void CrtCheckForUse(void* ptr) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  CrtDbgSymbol* debuginfo = NULL;
  size_t i = 0;
  size_t sh_mem_offset = 0;
  size_t sh_mem_count = 0;

  if (ptr == NULL) {
    return;
  }

  if (ctx == NULL) {
    printf("CrtCheckForUse: *** not set context ***\n=== abort ===\n");
    abort();
  }

  CrtCheckPtr(ptr);

  debuginfo = (CrtDbgSymbol*)(void*)((char*)ptr - ((sizeof(CrtDbgSymbol) + 7) &
                                                   (size_t)~7) - CANARY_SIZE);

  sh_mem_offset = (size_t)(((char*)ptr - ctx->memory->memory) >> 3);
  sh_mem_count = (debuginfo->real_size) >> 3;

  for (; i < sh_mem_count; i++) {
    if (ctx->sh_memory[sh_mem_offset+i] != SH_MEM_VALID) {
      printf("CrtCheckForUse: *** pointer to a canary/debug info/freed block ***\n"
             "(pointer: %p)\n"
             "=== aborted (core dumped) ===\n", ptr);
      abort();
    }
  }
}

size_t CrtGetShMemOffset(void* ptr) {
  CrtAllocatorCtx* ctx = AllocatorCtxManager(0, NULL);
  
  if (ctx == NULL) {
    printf("CrtGetShMemOffset: *** not set context ***\n=== abort ===\n");
    abort();
  }
  
  CrtCheckPtr(ptr);

  return (size_t)(((char*)ptr - ctx->memory->memory) >> 3);
}

size_t CrtGetShMemCountBytes(size_t bytes) {
  return ((bytes + 7) & (size_t)~7) >> 3;
}

#undef DBG_ALLOC_SOURCE

#endif /* (DBG_ALLOC_SOURCE || CRT_SOURCE) && !WAS_DBG_ALLOC */

#ifdef DEFER_WRAP_MALLOC
# define malloc CrtDbgAlloc
# define free   CrtDbgFree
#endif

#undef DEFER_WRAP_MALLOC
