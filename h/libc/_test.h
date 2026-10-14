/*
 * C header file for the checks a libc test makes.
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

#ifndef _TEST_H
#define _TEST_H

// The statuses a test returns: every check held, or one did not.
#define _TEST_PASSED 0
#define _TEST_FAILED 1

// The string that is none.
#define _TEST_NULL ((char *) 0)

// The base a number is written in, and the digits a byte of it may need.
#define _TEST_NUMBER_BASE   10
#define _TEST_NUMBER_DIGITS 3

// The size of a number's text, an unsigned long's largest included.
#define _TEST_NUMBER_SIZE (sizeof(unsigned long) * _TEST_NUMBER_DIGITS + 1)

// The first and last characters written as themselves.
#define _TEST_PRINT_FIRST ' '
#define _TEST_PRINT_LAST  '~'

// The bits of an octal digit, its largest value, and the digits of a byte.
#define _TEST_OCTAL_BITS   3
#define _TEST_OCTAL_MASK   7
#define _TEST_OCTAL_DIGITS 3

// The bits of a hexadecimal digit, its largest value, and every digit.
#define _TEST_HEX_BITS   4
#define _TEST_HEX_MASK   15
#define _TEST_HEX_DIGITS "0123456789abcdef"

// The bytes of a long double that hold its value: the x87's ten of sixteen.
#define _TEST_FLOAT_SIZE 10

// Check a condition, a value, a string or an object where the test stands.
#define _TEST_TRUE(cond)               _Test_True((cond) != 0, #cond, __FILE__, __LINE__, __func__)
#define _TEST_INT(got, expected)       _Test_Int((got), (expected), #got, __FILE__, __LINE__, __func__)
#define _TEST_UINT(got, expected)      _Test_Uint((got), (expected), #got, __FILE__, __LINE__, __func__)
#define _TEST_FLOAT(got, expected)     _Test_Float((got), (expected), #got, __FILE__, __LINE__, __func__)
#define _TEST_STR(got, expected)       _Test_Str((got), (expected), #got, __FILE__, __LINE__, __func__)
#define _TEST_MEM(got, expected, size) _Test_Mem((got), (expected), (size), #got, __FILE__, __LINE__, __func__)

// The status so far.
extern int _Test_Result;

// Text
unsigned long _Test_Length(const char *str);
void          _Test_Write(const char *str);
void          _Test_WriteChar(char ch);

// Operands
void _Test_ShowInt(long value);
void _Test_ShowUint(unsigned long value);
void _Test_ShowFloat(long double value);
void _Test_ShowStr(const char *str);
void _Test_ShowMem(const void *obj, unsigned long size);

// Checks
void  _Test_Fail(const char *expr, const char *file, unsigned int line, const char *func);
_Bool _Test_True(long value, const char *expr, const char *file, unsigned int line, const char *func);
_Bool _Test_Int(long got, long expected, const char *expr, const char *file, unsigned int line, const char *func);
_Bool _Test_Uint(unsigned long got, unsigned long expected, const char *expr, const char *file, unsigned int line, const char *func);
_Bool _Test_Float(long double got, long double expected, const char *expr, const char *file, unsigned int line, const char *func);
_Bool _Test_Str(const char *got, const char *expected, const char *expr, const char *file, unsigned int line, const char *func);
_Bool _Test_Mem(const void *got, const void *expected, unsigned long size, const char *expr, const char *file, unsigned int line, const char *func);
int   _Test_Status(void);

#endif
