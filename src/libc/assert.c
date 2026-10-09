/*
 * C source file for diagnostics.
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

// Module header.
#include <assert.h>

// The program's name.
#include <ivancc/crt.h>

// The system calls that report and end the program.
#include <ivancc/libc_sys.h>

// The string functions the message is built with.
#include <string.h>

// The descriptor of standard error.
#define __LIBC_ASSERT_STDERR 2

// Linux's SIGABRT, which ends a program whose assertion fails.
#define __LIBC_ASSERT_SIGABRT 6

// The status the program exits with if SIGABRT did not end it.
#define __LIBC_ASSERT_STATUS 127

// The base a line number is written in.
#define __LIBC_ASSERT_LINE_BASE 10

// The size of a line number's text, UINT_MAX's included.
#define __LIBC_ASSERT_LINE_SIZE 16

// Write the string str to standard error.
static void __libc_assert_write(const char *str)
{
    __libc_sys_write(__LIBC_ASSERT_STDERR, str, strlen(str));
}

// Write the line number line to standard error.
static void __libc_assert_write_line(unsigned int line)
{
    char text[__LIBC_ASSERT_LINE_SIZE];
    char *ptr = text + __LIBC_ASSERT_LINE_SIZE - 1;

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + line % __LIBC_ASSERT_LINE_BASE);
        line /= __LIBC_ASSERT_LINE_BASE;
    } while (line != 0);
    __libc_assert_write(ptr);
}

// Return the program's name as glibc's message gives it, or an empty string.
static const char *__libc_assert_program(void)
{
    const char *slash;

    if (__libc_crt_argv0 == NULL) {
        return "";
    }
    slash = strrchr(__libc_crt_argv0, '/');
    return slash != NULL ? slash + 1 : __libc_crt_argv0;
}

// Report a failed assertion and end the program as abort would.
void __libc_assert_fail(const char *expr, const char *file, unsigned int line, const char *func)
{
    const char *program = __libc_assert_program();

    if (*program != '\0') {
        __libc_assert_write(program);
        __libc_assert_write(": ");
    }
    __libc_assert_write(file);
    __libc_assert_write(":");
    __libc_assert_write_line(line);
    __libc_assert_write(": ");
    __libc_assert_write(func);
    __libc_assert_write(": Assertion `");
    __libc_assert_write(expr);
    __libc_assert_write("' failed.\n");
    __libc_sys_kill(__libc_sys_getpid(), __LIBC_ASSERT_SIGABRT);
    __libc_sys_exit(__LIBC_ASSERT_STATUS);
}
