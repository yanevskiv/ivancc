/*
 * C source file for the checks a libc test makes.
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
#include <_test.h>

// The report's texts.
#include <_err.h>

// The system call that writes the report.
#include <_sys.h>

// The status so far.
int _Test_Result = _TEST_PASSED;

// Return the length of the string str.
unsigned long _Test_Length(const char *str)
{
    unsigned long len = 0;

    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Write the string str to standard error.
void _Test_Write(const char *str)
{
    _Sys_Write(_SYS_STDERR_FILENO, str, _Test_Length(str));
}

// Write the character ch to standard error.
void _Test_WriteChar(char ch)
{
    _Sys_Write(_SYS_STDERR_FILENO, &ch, sizeof(ch));
}

// Show a signed number as _TEST_INT reports it, its sign where negative.
void _Test_ShowInt(long value)
{
    if (value < 0) {
        _Test_WriteChar('-');
        _Test_ShowUint(-(unsigned long) value);
    } else {
        _Test_ShowUint((unsigned long) value);
    }
}

// Show an unsigned number as _TEST_UINT reports it, in decimal.
void _Test_ShowUint(unsigned long value)
{
    char text[_TEST_NUMBER_SIZE];
    char *ptr = text + _TEST_NUMBER_SIZE - 1;

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + value % _TEST_NUMBER_BASE);
        value /= _TEST_NUMBER_BASE;
    } while (value != 0);
    _Test_Write(ptr);
}

// Show a floating value as _TEST_FLOAT reports it, its bytes as one hex number.
void _Test_ShowFloat(long double value)
{
    unsigned long k = _TEST_FLOAT_SIZE;
    const unsigned char *bytes = (const unsigned char *) &value;

    _Test_Write(_ERR_TEST_HEX);
    while (k > 0) {
        k--;
        _Test_WriteChar(_TEST_HEX_DIGITS[bytes[k] >> _TEST_HEX_BITS]);
        _Test_WriteChar(_TEST_HEX_DIGITS[bytes[k] & _TEST_HEX_MASK]);
    }
}

// Show a string as _TEST_STR reports it, a string literal or none.
void _Test_ShowStr(const char *str)
{
    if (str == _TEST_NULL) {
        _Test_Write(_ERR_TEST_NULL);
    } else {
        _Test_ShowMem(str, _Test_Length(str));
    }
}

// Show an object as _TEST_MEM reports it, a string literal of its size bytes.
void _Test_ShowMem(const void *obj, unsigned long size)
{
    int shift;
    unsigned long k;
    const unsigned char *bytes = obj;

    _Test_WriteChar('"');
    for (k = 0; k < size; k++) {
        if (bytes[k] == '"' || bytes[k] == '\\') {
            _Test_WriteChar('\\');
            _Test_WriteChar((char) bytes[k]);
        } else if (bytes[k] >= _TEST_PRINT_FIRST && bytes[k] <= _TEST_PRINT_LAST) {
            _Test_WriteChar((char) bytes[k]);
        } else {
            shift = _TEST_OCTAL_BITS * _TEST_OCTAL_DIGITS;
            _Test_WriteChar('\\');
            while (shift > 0) {
                shift -= _TEST_OCTAL_BITS;
                _Test_WriteChar((char) ('0' + ((bytes[k] >> shift) & _TEST_OCTAL_MASK)));
            }
        }
    }
    _Test_WriteChar('"');
}

// Note a failed check and open its report with where it stands.
void _Test_Fail(const char *expr, const char *file, unsigned int line, const char *func)
{
    _Test_Result = _TEST_FAILED;
    _Test_Write(file);
    _Test_Write(_ERR_TEST_FILE);
    _Test_ShowUint(line);
    _Test_Write(_ERR_TEST_LINE);
    _Test_Write(func);
    _Test_Write(_ERR_TEST_FUNC);
    _Test_Write(expr);
}

// Check that a value is not zero, and report it where it is.
_Bool _Test_True(long value, const char *expr, const char *file, unsigned int line, const char *func)
{
    if (value == 0) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_FALSE);
    }
    return value != 0;
}

// Check that a signed number is the one expected, and report another.
_Bool _Test_Int(long got, long expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    if (got != expected) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_ShowInt(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_ShowInt(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return got == expected;
}

// Check that an unsigned number is the one expected, and report another.
_Bool _Test_Uint(unsigned long got, unsigned long expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    if (got != expected) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_ShowUint(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_ShowUint(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return got == expected;
}

// Check that a floating value has the bytes expected, or neither is a number.
_Bool _Test_Float(long double got, long double expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    unsigned long k = 0;
    const unsigned char *got_bytes = (const unsigned char *) &got;
    const unsigned char *expected_bytes = (const unsigned char *) &expected;

    if (got != got && expected != expected) {
        k = _TEST_FLOAT_SIZE;
    }
    while (k < _TEST_FLOAT_SIZE && got_bytes[k] == expected_bytes[k]) {
        k++;
    }
    if (k < _TEST_FLOAT_SIZE) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_ShowFloat(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_ShowFloat(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return k == _TEST_FLOAT_SIZE;
}

// Check that a string is the one expected, or both none, and report another.
_Bool _Test_Str(const char *got, const char *expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    _Bool equal = got == expected;
    unsigned long k = 0;

    if (got != _TEST_NULL && expected != _TEST_NULL) {
        while (got[k] == expected[k] && got[k] != '\0') {
            k++;
        }
        equal = got[k] == expected[k];
    }
    if (! equal) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_ShowStr(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_ShowStr(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return equal;
}

// Check that an object holds the bytes expected, and report others.
_Bool _Test_Mem(const void *got, const void *expected, unsigned long size, const char *expr, const char *file, unsigned int line, const char *func)
{
    unsigned long k = 0;
    const unsigned char *got_bytes = got;
    const unsigned char *expected_bytes = expected;

    while (k < size && got_bytes[k] == expected_bytes[k]) {
        k++;
    }
    if (k < size) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_ShowMem(got, size);
        _Test_Write(_ERR_TEST_GOT);
        _Test_ShowMem(expected, size);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return k == size;
}

// Return the status a test returns: whether every check so far held.
int _Test_Status(void)
{
    return _Test_Result;
}
