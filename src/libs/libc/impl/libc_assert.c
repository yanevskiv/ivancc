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
#include <libc/impl/libc_assert.h>

// The program's name.
#include <crt/crt.h>

// The message's texts.
#include <libc/libc_err.h>

// The signal that ends the program.
#include <libc/impl/libc_signal.h>

// The system calls that report and end the program.
#include <libc/libc_sys.h>

// The string functions the message is built with.
#include <libc/impl/libc_string.h>

// Report a failed assertion and end the program as abort would.
void _Libc_Impl_Assert_Fail(const char *expr, const char *file, unsigned int line, const char *func)
{
    const char *program = _Libc_Impl_Assert_Program();

    if (*program != '\0') {
        _Libc_Impl_Assert_Write(program);
        _Libc_Impl_Assert_Write(_LIBC_ERR_ASSERT_PROGRAM);
    }
    _Libc_Impl_Assert_Write(file);
    _Libc_Impl_Assert_Write(_LIBC_ERR_ASSERT_FILE);
    _Libc_Impl_Assert_WriteLine(line);
    _Libc_Impl_Assert_Write(_LIBC_ERR_ASSERT_LINE);
    _Libc_Impl_Assert_Write(func);
    _Libc_Impl_Assert_Write(_LIBC_ERR_ASSERT_FUNC);
    _Libc_Impl_Assert_Write(expr);
    _Libc_Impl_Assert_Write(_LIBC_ERR_ASSERT_EXPR);
    _Libc_Sys_kill(_Libc_Sys_getpid(), _LIBC_IMPL_SIGNAL_SIGABRT);
    _Libc_Sys_exit(_LIBC_IMPL_ASSERT_STATUS);
}

// Write the string str to standard error.
void _Libc_Impl_Assert_Write(const char *str)
{
    _Libc_Sys_write(_LIBC_IMPL_ASSERT_STDERR, str, _Libc_Impl_String_strlen(str));
}

// Write the line number line to standard error.
void _Libc_Impl_Assert_WriteLine(unsigned int line)
{
    char text[_LIBC_IMPL_ASSERT_LINE_SIZE];
    char *ptr = text + _LIBC_IMPL_ASSERT_LINE_SIZE - 1;

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + line % _LIBC_IMPL_ASSERT_LINE_BASE);
        line /= _LIBC_IMPL_ASSERT_LINE_BASE;
    } while (line != 0);
    _Libc_Impl_Assert_Write(ptr);
}

// Return the program's name as glibc's message gives it, or an empty string.
const char *_Libc_Impl_Assert_Program(void)
{
    const char *slash;

    if (_Crt_Argv0 == _LIBC_IMPL_STDDEF_NULL) {
        return "";
    }
    slash = _Libc_Impl_String_strrchr(_Crt_Argv0, '/');
    return slash != _LIBC_IMPL_STDDEF_NULL ? slash + 1 : _Crt_Argv0;
}
