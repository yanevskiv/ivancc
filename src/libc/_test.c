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

// Write the number value to standard error.
void _Test_WriteUint(unsigned long value)
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

// Write the number value to standard error, with its sign where negative.
void _Test_WriteInt(long value)
{
    if (value < 0) {
        _Test_WriteChar('-');
        _Test_WriteUint(-(unsigned long) value);
    } else {
        _Test_WriteUint((unsigned long) value);
    }
}

// Write a byte as a string literal would spell it, in octal where it must.
void _Test_WriteByte(unsigned char byte)
{
    int shift = _TEST_OCTAL_BITS * _TEST_OCTAL_DIGITS;

    if (byte == '"' || byte == '\\') {
        _Test_WriteChar('\\');
        _Test_WriteChar((char) byte);
    } else if (byte >= _TEST_PRINT_FIRST && byte <= _TEST_PRINT_LAST) {
        _Test_WriteChar((char) byte);
    } else {
        _Test_WriteChar('\\');
        while (shift > 0) {
            shift -= _TEST_OCTAL_BITS;
            _Test_WriteChar((char) ('0' + ((byte >> shift) & _TEST_OCTAL_MASK)));
        }
    }
}

// Write the size bytes of obj as a string literal.
void _Test_WriteBytes(const void *obj, unsigned long size)
{
    unsigned long k;
    const unsigned char *bytes = obj;

    _Test_WriteChar('"');
    for (k = 0; k < size; k++) {
        _Test_WriteByte(bytes[k]);
    }
    _Test_WriteChar('"');
}

// Write the string str as a string literal, or say that it is none.
void _Test_WriteStr(const char *str)
{
    if (str == _TEST_NULL) {
        _Test_Write(_ERR_TEST_NULL);
    } else {
        _Test_WriteBytes(str, _Test_Length(str));
    }
}

// Note a failed check and open its report with where it stands.
void _Test_Fail(const char *expr, const char *file, unsigned int line, const char *func)
{
    _Test_Result = _TEST_FAILED;
    _Test_Write(file);
    _Test_Write(_ERR_TEST_FILE);
    _Test_WriteUint(line);
    _Test_Write(_ERR_TEST_LINE);
    _Test_Write(func);
    _Test_Write(_ERR_TEST_FUNC);
    _Test_Write(expr);
}

// Tell whether the size bytes of obj1 and obj2 are the same.
int _Test_Same(const void *obj1, const void *obj2, unsigned long size)
{
    unsigned long k = 0;
    const unsigned char *bytes1 = obj1;
    const unsigned char *bytes2 = obj2;

    while (k < size && bytes1[k] == bytes2[k]) {
        k++;
    }
    return k == size;
}

// Tell whether str1 and str2 are the same string, or both none.
int _Test_Equal(const char *str1, const char *str2)
{
    unsigned long k = 0;

    if (str1 == _TEST_NULL || str2 == _TEST_NULL) {
        return str1 == str2;
    }
    while (str1[k] == str2[k] && str1[k] != '\0') {
        k++;
    }
    return str1[k] == str2[k];
}

// Check that a condition holds, and report it where it does not.
int _Test_True(int value, const char *expr, const char *file, unsigned int line, const char *func)
{
    if (! value) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_FALSE);
    }
    return value;
}

// Check that a signed number is the one expected, and report another.
int _Test_Int(long got, long expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    if (got != expected) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_WriteInt(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_WriteInt(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return got == expected;
}

// Check that an unsigned number is the one expected, and report another.
int _Test_Uint(unsigned long got, unsigned long expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    if (got != expected) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_WriteUint(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_WriteUint(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return got == expected;
}

// Check that a string is the one expected, and report another.
int _Test_Str(const char *got, const char *expected, const char *expr, const char *file, unsigned int line, const char *func)
{
    int equal = _Test_Equal(got, expected);

    if (! equal) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_WriteStr(got);
        _Test_Write(_ERR_TEST_GOT);
        _Test_WriteStr(expected);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return equal;
}

// Check that an object holds the bytes expected, and report others.
int _Test_Mem(const void *got, const void *expected, unsigned long size, const char *expr, const char *file, unsigned int line, const char *func)
{
    int same = _Test_Same(got, expected, size);

    if (! same) {
        _Test_Fail(expr, file, line, func);
        _Test_Write(_ERR_TEST_EXPR);
        _Test_WriteBytes(got, size);
        _Test_Write(_ERR_TEST_GOT);
        _Test_WriteBytes(expected, size);
        _Test_Write(_ERR_TEST_EXPECTED);
    }
    return same;
}

// Return the status a test returns: whether every check so far held.
int _Test_Status(void)
{
    return _Test_Result;
}
