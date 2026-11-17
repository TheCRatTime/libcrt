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

#ifndef IWASHERE_IWASHERE_H_
#define IWASHERE_IWASHERE_H_

#include <stdio.h>

#define iwh___str1(x)  #x
#define iwh___tostr(x) iwh___str1(x)

#define iwashere() do { \
  fputs("i was here (" iwh___tostr(__LINE__) ")\n", stderr); \
  fflush(stderr); \
} while (0)

#define I_was_here() do { \
  fputs("i was here (" iwh___tostr(__LINE__) ")\n", stderr); \
  fflush(stderr); \
} while (0)

#define i_was_here() do { \
  fputs("i was here (" iwh___tostr(__LINE__) ")\n", stderr); \
  fflush(stderr); \
} while (0)

#endif /* IWASHERE_IWASHERE_H_ */
