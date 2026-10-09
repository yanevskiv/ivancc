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
#include <ivancc/impl/libc_assert.h>

// The program's name.
#include <crt/crt.h>

// The message's texts.
#include <ivancc/libc_err.h>

// The system calls that report and end the program.
#include <ivancc/libc_sys.h>

// The string functions the message is built with.
#include <ivancc/impl/libc_string.h>

// Report a failed assertion and end the program as abort would.
void __libc_impl_assert_fail(const char *expr, const char *file, unsigned int line, const char *func)
{
    const char *program = __libc_impl_assert_program();

    if (*program != '\0') {
        __libc_impl_assert_write(program);
        __libc_impl_assert_write(__LIBC_ERR_ASSERT_PROGRAM);
    }
    __libc_impl_assert_write(file);
    __libc_impl_assert_write(__LIBC_ERR_ASSERT_FILE);
    __libc_impl_assert_write_line(line);
    __libc_impl_assert_write(__LIBC_ERR_ASSERT_LINE);
    __libc_impl_assert_write(func);
    __libc_impl_assert_write(__LIBC_ERR_ASSERT_FUNC);
    __libc_impl_assert_write(expr);
    __libc_impl_assert_write(__LIBC_ERR_ASSERT_EXPR);
    __libc_sys_kill(__libc_sys_getpid(), __LIBC_SYS_SIGABRT);
    __libc_sys_exit(__LIBC_IMPL_ASSERT_STATUS);
}

// Write the string str to standard error.
void __libc_impl_assert_write(const char *str)
{
    __libc_sys_write(__LIBC_IMPL_ASSERT_STDERR, str, __libc_impl_string_strlen(str));
}

// Write the line number line to standard error.
void __libc_impl_assert_write_line(unsigned int line)
{
    char text[__LIBC_IMPL_ASSERT_LINE_SIZE];
    char *ptr = text + __LIBC_IMPL_ASSERT_LINE_SIZE - 1;

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + line % __LIBC_IMPL_ASSERT_LINE_BASE);
        line /= __LIBC_IMPL_ASSERT_LINE_BASE;
    } while (line != 0);
    __libc_impl_assert_write(ptr);
}

// Return the program's name as glibc's message gives it, or an empty string.
const char *__libc_impl_assert_program(void)
{
    const char *slash;

    if (__crt_argv0 == NULL) {
        return "";
    }
    slash = __libc_impl_string_strrchr(__crt_argv0, '/');
    return slash != NULL ? slash + 1 : __crt_argv0;
}
