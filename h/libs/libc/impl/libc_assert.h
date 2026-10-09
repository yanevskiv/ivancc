/*
 * C header file for diagnostics.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _LIBC_IMPL_ASSERT_H
#define _LIBC_IMPL_ASSERT_H

// The descriptor of standard error.
#define _LIBC_IMPL_ASSERT_STDERR 2

// The status the program exits with if SIGABRT did not end it.
#define _LIBC_IMPL_ASSERT_STATUS 127

// The base a line number is written in.
#define _LIBC_IMPL_ASSERT_LINE_BASE 10

// The size of a line number's text, UINT_MAX's included.
#define _LIBC_IMPL_ASSERT_LINE_SIZE 16

// The assert macro
void _Libc_Impl_Assert_Fail(const char *expr, const char *file, unsigned int line, const char *func);

// Report
void _Libc_Impl_Assert_Write(const char *str);
void _Libc_Impl_Assert_WriteLine(unsigned int line);
const char *_Libc_Impl_Assert_Program(void);

#endif // _LIBC_IMPL_ASSERT_H
