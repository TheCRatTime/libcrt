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

#ifndef LIBCRT_LIBCRT_ENCODINGS_RLE_H_
#define LIBCRT_LIBCRT_ENCODINGS_RLE_H_

#include <stddef.h>

/* ASCII US (Unit Separator) */
#define US (0x1F)

/* RLE encode string. */
void CrtRLEEncode(const char*, char*, size_t);

/* RLE decode string. */
void CrtRLEDecode(const char*, char*, size_t);

#endif /* LIBCRT_LIBCRT_ENCODINGS_RLE_H_ */

#if (defined(RLE_SOURCE) || defined(CRT_SOURCE)) && !defined(WAS_RLE)
#define WAS_RLE

#define WriteToBuf(chr) \
  if (wrote < len) { \
    buf[wrote] = chr; \
  } else { \
    return; \
  } \
  wrote++

static void ItoA(int num, char* buf, size_t len) {
  size_t wrote = 0;
  size_t start = 0;
  size_t end = 0;

  if (num == 0) {
    WriteToBuf('0');
    WriteToBuf('\0');
    return;
  }

  if (num <= 9) {
    WriteToBuf((char)(num + '0'));
    WriteToBuf('\0');
    return;
  }

  for(; num != 0; num /= 10) {
    WriteToBuf((char)(num % 10) + '0');
  }

  WriteToBuf('\0');

  start = 0;
  end = wrote - 2;

  while(end > start) {
    char temp = buf[start];
    buf[start] = buf[end];
    buf[end] = temp;
    start++;
    end--;
  }
}

#undef WriteToBuf

static int AtoI(const char** str) {
  int ret = **str - '0';
  
  if (**str == '\0') {
    return 0;
  }
  
  (*str)++;
  
  for (; **str >= '0' && **str <= '9'; (*str)++) {
    ret = (ret * 10) + (**str - '0');
  }

  return ret;
}

#define AppendChar(chr) \
  if (wrote < maxlen) { \
    out[wrote] = chr; \
    wrote++; \
  } else { \
    return; \
  } \
  (void)out
  
void CrtRLEEncode(const char* data, char* out, size_t maxlen) {
  char pattern = '\0';
  size_t wrote = 0;
  int repeat = 0;

  while (*data != '\0') {
    char* ptr = NULL;
    char itoa_result[32] = {0};
    repeat = 1;
    pattern = *(data++);

    while (*data == pattern && *data != '\0') {
      data++;
      repeat++;
    }

    ItoA(repeat, itoa_result, 31);

    for (ptr = &itoa_result[0]; *ptr != '\0'; ptr++) {
      AppendChar(*ptr);
    }

    if (pattern >= '0' && pattern <= '9') {
      AppendChar(US);
    }

    AppendChar(pattern);
  }
}

void CrtRLEDecode(const char* data, char* out, size_t maxlen) {
  char pattern = '\0';
  size_t wrote = 0;
  int repeat = 0;

  while(*data != '\0') {
    repeat = 0;
    
    repeat = AtoI(&data);
    if (*data == US) {
      data++;
    }

    pattern = *(data++);

    while(repeat != 0) {
      AppendChar(pattern);
      repeat--;
    }
  }
}

#undef AppendChar

#undef RLE_SOURCE

#endif /* (RLE_SOURCE || CRT_SOURCE) && !WAS_RLE */
