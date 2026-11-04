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

#ifndef LIBCRT_LIBCRT_H_
# error "This file can only included by crt.h"
#endif

#ifdef CRT_ENABLE_ALL
# define CRT_MEM_ATEXIT
# define CRT_MEM_DBG_ALLOC
# define CRT_MEM_BLOCK_ALLOC
# define CRT_MEM_GC_MALLOC
# define CRT_MEM_ARENA
# define CRT_ENCODINGS_HEX
# define CRT_ENCODINGS_RLE
# define CRT_ERRORS_CERRORS
#endif

#ifdef CRT_NOT_USE_NAMESPACE
# define ATEXIT_INC_PATH      "unsafe/mem/atexit.h"
# define DBG_ALLOC_INC_PATH   "unsafe/mem/dbg_alloc.h"
# define GC_MALLOC_INC_PATH   "unsafe/mem/GCmalloc.h"
# define BLOCK_ALLOC_INC_PATH "unsafe/mem/block_alloc.h"
# define ARENA_INC_PATH       "unsafe/mem/arena.h"
# define HEX_INC_PATH         "unsafe/encodings/hex.h"
# define RLE_INC_PATH         "unsafe/encodings/RLE.h"
# define CERRORS_INC_PATH     "unsafe/errors/cerrors.h"
#else
# define ATEXIT_INC_PATH      "mem/atexit.h"
# define DBG_ALLOC_INC_PATH   "mem/dbg_alloc.h"
# define GC_MALLOC_INC_PATH   "mem/GCmalloc.h"
# define BLOCK_ALLOC_INC_PATH "mem/block_alloc.h"
# define ARENA_INC_PATH       "mem/arena.h"
# define HEX_INC_PATH         "encodings/hex.h"
# define RLE_INC_PATH         "encodings/RLE.h"
# define CERRORS_INC_PATH     "errors/cerrors.h"
#endif

#ifdef CRT_SOURCE
# define ATEXIT_SOURCE
# define DBG_ALLOC_SOURCE
# define GC_MALLOC_SOURCE
# define BLOCK_ALLOC_SOURCE
# define ARENA_SOURCE

# if !defined(DEBUG) || defined(NDEBUG)
#  define FAST_ARENA
# else
#  define DEBUG_ARENA
# endif

# define LIBHEX_SOURCE
# define RLE_SOURCE
#endif

#ifdef CRT_MEM_ATEXIT
# include ATEXIT_INC_PATH
#endif

#ifdef CRT_MEM_DBG_ALLOC
# include DBG_ALLOC_INC_PATH
#endif

#ifdef CRT_MEM_GC_MALLOC
# include GC_MALLOC_INC_PATH
#endif

#ifdef CRT_MEM_BLOCK_ALLOC
# include BLOCK_ALLOC_INC_PATH
#endif

#ifdef CRT_MEM_ARENA
# include ARENA_INC_PATH
#endif

#ifdef CRT_ENCODINGS_HEX
# include HEX_INC_PATH
#endif

#ifdef CRT_ENCODINGS_RLE
# include RLE_INC_PATH
#endif

#ifdef CRT_ERRORS_CERRORS
# include CERRORS_INC_PATH
#endif
