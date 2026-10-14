/*
 * C header file for diagnostics.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

// No guard: each inclusion defines assert again.
// [C99:7.2p1]: assert, redefined at each inclusion of <assert.h>
#undef assert

// The descriptor of standard error.
#define _ASSERT_STDERR 2

// The base a line number is written in.
#define _ASSERT_LINE_BASE 10

// The size of a line number's text, UINT_MAX's included.
#define _ASSERT_LINE_SIZE 16

// The assert macro
// [C99:7.2.1.1]: assert
#ifdef NDEBUG
#define assert(ignore) ((void) 0)
#else
#define assert(expression) ((expression) ? (void) 0 : _Assert_ReportFailure(#expression, __FILE__, __LINE__, __func__))
#endif

// Failed assertions
void _Assert_ReportFailure(const char *expr, const char *file, unsigned int line, const char *func);

// Report
void        _Assert_WriteString(const char *str);
void        _Assert_WriteLineNumber(unsigned int line);
const char *_Assert_GetProgramName(void);
