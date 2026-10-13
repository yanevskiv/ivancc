/*
 * C header file for the UTF-8 encoder and decoder.
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

#ifndef _UTF8_H
#define _UTF8_H

// The ranges of each class and the shifts of each case mapping.
#include <_utf8_table.h>

// The longest sequence and the largest code it encodes, glibc's 31 bits.
#define _UTF8_MAX      6
#define _UTF8_CODE_MAX 0x7FFFFFFFU

// The largest code one byte encodes, all of "C"'s encoding, and its bytes.
#define _UTF8_ASCII_MAX 0x7FU
#define _UTF8_ASCII_LEN 1

// The tag characters "C"'s encoding drops, as glibc's.
#define _UTF8_TAG_FIRST 0xE0000
#define _UTF8_TAG_LAST  0xE007F

// The least lead byte of a longer sequence, past the overlong 0xC0 and 0xC1.
#define _UTF8_LEAD_MIN 0xC2U

// The ones of every lead byte, shifted right by its sequence's length.
#define _UTF8_LEAD_ONES 0xFF00U

// The mark of a continuation byte, the mask over it and the bits after it.
#define _UTF8_TAIL_MARK  0x80U
#define _UTF8_TAIL_MASK  0xC0U
#define _UTF8_TAIL_VALUE 0x3FU
#define _UTF8_TAIL_BITS  6

// The surrogates, which no sequence encodes.
#define _UTF8_SURROGATE_FIRST 0xD800U
#define _UTF8_SURROGATE_LAST  0xDFFFU

// What decoding gives for a sequence that is invalid or cut short.
#define _UTF8_INVALID    (-1)
#define _UTF8_INCOMPLETE (-2)

// The classes a code may be in.
typedef enum _Utf8_Class _Utf8_Class;
enum _Utf8_Class {
    _UTF8_CLASS_ALNUM, // a letter or a decimal digit
    _UTF8_CLASS_ALPHA, // a letter
    _UTF8_CLASS_BLANK, // white space that separates words in a line
    _UTF8_CLASS_CNTRL, // a control code
    _UTF8_CLASS_DIGIT, // a decimal digit, ASCII's ten alone
    _UTF8_CLASS_GRAPH, // a code with a visible form
    _UTF8_CLASS_LOWER, // a lower-case letter
    _UTF8_CLASS_PRINT, // a code that takes a place when printed
    _UTF8_CLASS_PUNCT, // a code with a visible form, no letter and no digit
    _UTF8_CLASS_SPACE, // white space
    _UTF8_CLASS_UPPER, // an upper-case letter
    _UTF8_CLASS_XDIGIT // a hex digit, ASCII's alone
};

// Whether LC_CTYPE's locale encodes in UTF-8.
extern _Bool _Utf8_Enabled;

// Coding
int _Utf8_GetLeadLength(unsigned int lead);
int _Utf8_GetCodeLength(unsigned int code);
int _Utf8_DecodeSequence(const unsigned char *str, unsigned long n, unsigned int *code);
int _Utf8_EncodeCode(unsigned int code, unsigned char *str);

// Classes
_Bool        _Utf8_IsInRanges(const _Utf8_Table_Range *ranges, unsigned long count, unsigned int code);
_Bool        _Utf8_IsInClass(unsigned int code, _Utf8_Class class);
unsigned int _Utf8_ToShifted(const _Utf8_Table_Shift *shifts, unsigned long count, unsigned int code);
unsigned int _Utf8_ToUpper(unsigned int code);
unsigned int _Utf8_ToLower(unsigned int code);

// Characters
unsigned long _Utf8_GetCharMax(void);
int           _Utf8_DecodeChar(const char *str, unsigned long n, int *wc);
int           _Utf8_EncodeChar(int wc, char *str);

#endif // _UTF8_H
