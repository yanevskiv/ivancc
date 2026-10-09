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
#include <_crt.h>

// The message's texts.
#include <_err.h>

// The signal that ends the program.
#include <signal.h>

// The system calls that report and end the program.
#include <_sys.h>

// The string functions the message is built with.
#include <string.h>

// Report a failed assertion and end the program as abort would.
void _Assert_Fail(const char *expr, const char *file, unsigned int line, const char *func)
{
    const char *program = _Assert_Program();

    if (*program != '\0') {
        _Assert_Write(program);
        _Assert_Write(_ERR_ASSERT_PROGRAM);
    }
    _Assert_Write(file);
    _Assert_Write(_ERR_ASSERT_FILE);
    _Assert_WriteLine(line);
    _Assert_Write(_ERR_ASSERT_LINE);
    _Assert_Write(func);
    _Assert_Write(_ERR_ASSERT_FUNC);
    _Assert_Write(expr);
    _Assert_Write(_ERR_ASSERT_EXPR);
    _Sys_Kill(_Sys_Getpid(), SIGABRT);
    _Sys_Exit(_ASSERT_STATUS);
}

// Write the string str to standard error.
void _Assert_Write(const char *str)
{
    _Sys_Write(_ASSERT_STDERR, str, strlen(str));
}

// Write the line number line to standard error.
void _Assert_WriteLine(unsigned int line)
{
    char text[_ASSERT_LINE_SIZE];
    char *ptr = text + _ASSERT_LINE_SIZE - 1;

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + line % _ASSERT_LINE_BASE);
        line /= _ASSERT_LINE_BASE;
    } while (line != 0);
    _Assert_Write(ptr);
}

// Return the program's name as glibc's message gives it, or an empty string.
const char *_Assert_Program(void)
{
    const char *slash;

    if (_Crt_Argv0 == NULL) {
        return "";
    }
    slash = strrchr(_Crt_Argv0, '/');
    return slash != NULL ? slash + 1 : _Crt_Argv0;
}
