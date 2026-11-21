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

#ifndef BETTER_BETTER_H_
#define BETTER_BETTER_H_

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

#define better_concat1(a, b) a##b
#define better_concat(a, b) better_concat1(a, b)

/* Static assert */
# define static_require(name, expr) \
  static char static_require_##name[!(expr) ? -1 : 1]

#ifndef BETTER_NO_BIT_ERROR
/* More programs needs 8 bit in char type */
static_require(char_bits, CHAR_BIT == 8);
#endif

/* u8 */
typedef unsigned char u8;
typedef signed char i8;
typedef u8 byte;

#define Fu8 "%u"
#define Fi8 "%c"

/* u16 */
#if SHRT_MAX == 32767
typedef unsigned short u16;
typedef signed short i16;

# define Fu16 "%hu"
# define Fi16 "%hd"
#elif INT_MAX == 32767
typedef unsigned int u16;
typedef signed int i16;

# define Fu16 "%u"
# define Fi16 "%d"
#else
# error "Cannot define u16 and i16"
#endif

/* u32 */
#if ULONG_MAX == 4294967295
typedef unsigned long u32;
typedef signed long i32;

# define Fu32 "%lu"
# define Fi32 "%ld"
#elif UINT_MAX == 4294967295
typedef unsigned int u32;
typedef signed int i32;

# define Fu32 "%u"
# define Fi32 "%d"
#else
# error "Cannot define u16 and i16"
#endif

/* Other names */
typedef u8 uint8;
typedef i8 int8;
typedef u16 uint16;
typedef i16 int16;
typedef u32 uint32;
typedef i32 int32;

/* size_t and void */
typedef size_t usize;
typedef void any;
typedef void none;

/* NULL */
#define nil  NULL
#define null NULL

/* Also other names */
typedef unsigned char uchar;
typedef signed char ichar;
typedef unsigned short ushort;
typedef signed short ishort;
typedef unsigned long ulong;
typedef signed long ilong;
typedef long double ldouble;

/* printf's format strings */
#define Fuchar   "%u"
#define Fichar   "%c"
#define Fushort  "%hu"
#define Fishort  "%hd"
#define Fulong   "%lu"
#define Filong   "%ld"
#define Fldouble "%Lf"

/* static keyword */
#define local    static
#define internal static

/* For f32 needed 32 (4*8) bits in float */
static_require(float_size, sizeof(float) == (32 / CHAR_BIT));

typedef float f32;
#define Ff32 "%f"

/* Cannot define f64 because type 'double' can also have 32 bits. */

/* Useful macro */
#define cast(type, object) ((type)(object))
#define as(type, object)   ((type)(object))
#define charptr(ptr)       ((char*)(ptr))
#define unused(var)        ((void)(var))
#define zeroize(var, siz)  memset((var), 0, (siz))
#define nothing            ((void)0)
#define ispow2(num)        (((num) & ((num)-1)) == 0)

/* "require" word more good reads than "assert" */
#define require(expr)      assert(expr)

/* Checks */
#define zero(item)   (!(item))
#define isnull(item) zero(item)

/* Loops */
#define loop for(;;)

/* Remove spaces at end and start of STR */
void trim_string(char** str);

/* Get input and remove newline at end */
char* input(char* buf, int siz);

/* #undef needed */
#undef better_require
#undef better_concat
#undef better_concat1

#endif /* BETTER_BETTER_H_ */

#ifdef BETTER_SOURCE

#include <stdio.h>

char* input(char* buf, int siz) {
  char* ret = NULL;
  
  if (siz <= 1 || buf == NULL) { return NULL; }
  
  ret = fgets(buf, siz, stdin);

  buf[strcspn(buf, "\r\n")] = '\0';

  return ret;
}

void trim_string(char** str) {
  if (!str || !(*str)) { return; }

  /* Just remove spaces */
  (*str) += strspn(*str, " \f\t\r\n\v");
  (*str)[strcspn(*str, " \f\t\r\n\v")] = '\0';
}

#endif /* BETTER_SOURCE */

