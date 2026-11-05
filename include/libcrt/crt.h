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
#define LIBCRT_LIBCRT_H_

#define CRT_VERSION "1.0.3"
#define CRT_VERSION_NUM 010003L

#if !defined(CRT_MEM_ATEXIT) && \
    !defined(CRT_MEM_DBG_ALLOC) && \
    !defined(CRT_MEM_BLOCK_ALLOC) && \
    !defined(CRT_MEM_GC_MALLOC) && \
    !defined(CRT_MEM_ARENA) && \
    !defined(CRT_ENCODINGS_HEX) && \
    !defined(CRT_ENCODINGS_RLE) && \
    !defined(CRT_ERRORS_CERRORS) && \
    !defined(CRT_ENABLE_ALL) && \
    !defined(CRT_IMPORT) && \
    !defined(CRT_IMPORT2) && \
    !defined(CRT_IMPORT3) && \
    !defined(CRT_IMPORT4) && \
    !defined(CRT_IMPORT5) && \
    !defined(CRT_IMPORT6)
# error "You don't define one of macro. Remove include or define macro."
#endif

#include "crt_parse_macro.h"

#define CRT_IMPORT_NOERROR

#include "crt_import.h"

#endif /* LIBCRT_LIBCRT_H_ */
