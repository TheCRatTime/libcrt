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

#ifndef IMPORT_NOERROR
# if !defined(CRT_IMPORT) && \
     !defined(CRT_IMPORT2) && \
     !defined(CRT_IMPORT3) && \
     !defined(CRT_IMPORT4) && \
     !defined(CRT_IMPORT5) && \
     !defined(CRT_IMPORT6)
#  error "Nothing imported, remove include or import something."
# endif
#endif

#undef IMPORT_NOERROR

#ifdef CRT_IMPORT
# include CRT_IMPORT
#endif

#ifdef CRT_IMPORT2
# include CRT_IMPORT2
#endif

#ifdef CRT_IMPORT3
# include CRT_IMPORT3
#endif

#ifdef CRT_IMPORT4
# include CRT_IMPORT4
#endif

#ifdef CRT_IMPORT5
# include CRT_IMPORT5
#endif

#ifdef CRT_IMPORT6
# include CRT_IMPORT6
#endif

#undef CRT_IMPORT
#undef CRT_IMPORT2
#undef CRT_IMPORT3
#undef CRT_IMPORT4
#undef CRT_IMPORT5
#undef CRT_IMPORT6
