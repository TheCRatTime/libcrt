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

#ifndef LIBCRT_LIBCRT_ENCODINGS_HEX_H_
#define LIBCRT_LIBCRT_ENCODINGS_HEX_H_

#include <stddef.h>

#define HEX_TOTAL 16

static const char kHexAlphabet[HEX_TOTAL+1] = "0123456789ABCDEF";

/* Encode DATA to hex format and write to OUTPUT.
   Returns 0 on success, -1 at error. */
int CrtEncodeToHex(const char*, char*);

/* Decode DATA from hex format and write to OUTPUT.
   Returns 0 on success, -1 at error. */
int CrtDecodeFromHex(const char*, char*);

/* Like EncodeToHex, but max len of OUTPUT is MAXLEN */
int CrtHexEncodeMaxLen(const char*, char*, size_t);

/* Like DecodeToHex, but max len of OUTPUT is MAXLEN */
int CrtHexDecodeMaxLen(const char*, char*, size_t);

#endif /* LIBCRT_LIBCRT_ENCODINGS_HEX_H_ */

#if (defined(LIBHEX_SOURCE) || defined(CRT_SOURCE)) && !defined(WAS_HEX)
#define WAS_HEX

#include <string.h>

int CrtEncodeToHex(const char* data, char* output) {
  size_t data_len = 0;
  size_t i = 0;
  size_t j = 0;

  if (data == NULL || output == NULL) {
    return -1;
  }

  data_len = strlen(data);

  if (data_len == 0) {
    return -1;
  }

  for (; i < data_len; i++) {
    output[j++] = kHexAlphabet[((data[i] >> 4) & 0xF)];
    output[j++] = kHexAlphabet[(data[i] & 0xF)];
  }

  output[j] = '\0';

  return 0;
}

static int ByteFromHex(char ch1, char ch2, char* output) {
  if (((ch1 < '0' || ch1 > '9') && (ch1 < 'A' || ch1 > 'F') &&
       (ch1 < 'a' || ch1 > 'f')) ||
      ((ch2 < '0' || ch2 > '9') && (ch2 < 'A' || ch2 > 'F') &&
       (ch2 < 'a' || ch2 > 'f'))) {
    return -1;
  }

  if (ch1 >= '0' && ch1 <= '9') {
    ch1 -= '0';
  } else if (ch1 >= 'a' && ch1 <= 'f') {
    ch1 = (ch1 - 'a') + 10;
  } else {
    ch1 = (ch1 - 'A') + 10;
  }
  
  if (ch2 >= '0' && ch2 <= '9') {
    ch2 -= '0';
  } else if (ch2 >= 'a' && ch2 <= 'f') {
    ch2 = (ch2 - 'a') + 10;
  } else {
    ch2 = (ch2 - 'A') + 10;
  }
  
  *output = (char)((ch1 << 4) + ch2);

  return 0;
}

int CrtDecodeFromHex(const char* data, char* output) {
  size_t data_len = 0;
  size_t i = 0;
  size_t j = 0;
  size_t data_write = 0;
  
  if (data == NULL || output == NULL) {
    return -1;
  }
  
  data_len = strlen(data);
  data_write = data_len & 0x1;

  if (data_len == 0) {
    return -1;
  }

  if (data_write) {
    return -1;
  }

  for (; i < data_len; i += 2) {
    if (ByteFromHex(data[i], data[i+1], &output[j++]) == -1) {
      return -1;
    }
  }
  
  output[j] = '\0';
  
  return 0;
}

int CrtHexEncodeMaxLen(const char* data, char* output, size_t maxlen) {
  if (data == NULL || output == NULL || maxlen < 3) {
    return -1;
  }

  if (maxlen < ((strlen(data) << 1) + 1)) {
    return -1;
  }

  return CrtEncodeToHex(data, output);
}

/* Like DecodeToHex, but max len of OUTPUT is MAXLEN */
int CrtHexDecodeMaxLen(const char* data, char* output, size_t maxlen) {
  if (data == NULL || output == NULL || maxlen < 2) {
    return -1;
  }

  if (strlen(data) & 0x1) {
    return -1;
  }

  if (maxlen < ((strlen(data) >> 1) + 1)) {
    return -1;
  }

  return CrtDecodeFromHex(data, output);
}

#undef HEX_SOURCE

#endif /* (HEX_SOURCE || CRT_SOURCE) && !WAS_HEX */
