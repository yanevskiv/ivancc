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

#ifndef __LIBC_ASSERT_H__
#define __LIBC_ASSERT_H__

// The descriptor of standard error.
#define __LIBC_ASSERT_STDERR 2

// The status the program exits with if SIGABRT did not end it.
#define __LIBC_ASSERT_STATUS 127

// The base a line number is written in.
#define __LIBC_ASSERT_LINE_BASE 10

// The size of a line number's text, UINT_MAX's included.
#define __LIBC_ASSERT_LINE_SIZE 16

// (S7.2.1.1) The assert macro
void __libc_assert_fail(const char *expr, const char *file, unsigned int line, const char *func);

// Report
void __libc_assert_write(const char *str);
void __libc_assert_write_line(unsigned int line);
const char *__libc_assert_program(void);

#endif // __LIBC_ASSERT_H__
