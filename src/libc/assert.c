/*
 * C source file for diagnostics.
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

// Module header.
#include <assert.h>

// The program's name.
#include <_crt.h>

// The message's texts.
#include <_err.h>

// The abort that ends the program.
#include <stdlib.h>

// The system call that writes the report.
#include <_sys.h>

// The string functions the message is built with.
#include <string.h>

// Report a failed assertion and end the program by abort.
void _Assert_ReportFailure(const char *expr, const char *file, unsigned int line, const char *func)
{
    const char *program = _Assert_GetProgramName();

    if (*program != '\0') {
        _Assert_WriteString(program);
        _Assert_WriteString(_ERR_ASSERT_PROGRAM);
    }
    _Assert_WriteString(file);
    _Assert_WriteString(_ERR_ASSERT_FILE);
    _Assert_WriteLineNumber(line);
    _Assert_WriteString(_ERR_ASSERT_LINE);
    _Assert_WriteString(func);
    _Assert_WriteString(_ERR_ASSERT_FUNC);
    _Assert_WriteString(expr);
    _Assert_WriteString(_ERR_ASSERT_EXPR);
    abort();
}

// Write the string str to standard error.
void _Assert_WriteString(const char *str)
{
    _Sys_InvokeWrite(_ASSERT_STDERR, str, strlen(str));
}

// Write the line number line to standard error.
void _Assert_WriteLineNumber(unsigned int line)
{
    char text[_ASSERT_LINE_SIZE];
    char *ptr = text + _ASSERT_LINE_SIZE - 1;

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + line % _ASSERT_LINE_BASE);
        line /= _ASSERT_LINE_BASE;
    } while (line != 0);
    _Assert_WriteString(ptr);
}

// Get the program's name as glibc's message gives it, or an empty string.
const char *_Assert_GetProgramName(void)
{
    const char *slash;

    if (_Crt_Argv0 == NULL) {
        return "";
    }
    slash = strrchr(_Crt_Argv0, '/');
    return slash != NULL ? slash + 1 : _Crt_Argv0;
}
