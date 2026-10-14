/*
 * C header file for formatted input.
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

#ifndef _SCAN_H
#define _SCAN_H

// The streams formatted input comes from.
#include <_file.h>

// A pointer to nothing, of any type.
#define _SCAN_NULL ((void *) 0)

// The greatest int, and the greatest size.
#define _SCAN_INT_MAX  0x7FFFFFFF
#define _SCAN_SIZE_MAX ((unsigned long) -1)

// The most bytes a character has in any locale.
#define _SCAN_BYTES_MAX 16

// The greatest character a number's item may hold.
#define _SCAN_ASCII_MAX 0x7F

// The base a width is written in.
#define _SCAN_COUNT_BASE 10

// The width of a directive that gives none.
#define _SCAN_WIDTH_NONE 0

// The lengths a conversion allows, each a bit: none alone, an integer's,
// a floating number's, and the l of wide characters.
#define _SCAN_LENGTHS_NONE    (1 << _SCAN_LENGTH_NONE)
#define _SCAN_LENGTHS_INTEGER (~(1 << _SCAN_LENGTH_LDOUBLE))
#define _SCAN_LENGTHS_REAL    (1 << _SCAN_LENGTH_NONE | 1 << _SCAN_LENGTH_LONG | 1 << _SCAN_LENGTH_LDOUBLE)
#define _SCAN_LENGTHS_WIDE    (1 << _SCAN_LENGTH_LONG)

// How many conversions there are, a letter in capitals being one of its own.
#define _SCAN_RULES 23

// An integer's base: the one its prefix names, 8, 10 or 16.
#define _SCAN_BASE_PREFIX  0
#define _SCAN_BASE_OCTAL   8
#define _SCAN_BASE_DECIMAL 10
#define _SCAN_BASE_HEX     16

// The size of an array that holds an integer: more digits overflow.
#define _SCAN_ITEM 32

// A null pointer as %p reads it, and the least width that holds it.
#define _SCAN_NIL       "(nil)"
#define _SCAN_NIL_WIDTH 5

// A floating number's characters: a sign, a digit of either base, the point,
// either exponent's mark, the words, and what a NaN's parentheses hold.
#define _SCAN_SIGNS    "+-"
#define _SCAN_DECIMALS "0123456789"
#define _SCAN_HEXES    "0123456789abcdef"
#define _SCAN_POINT    "."
#define _SCAN_EXPONENT "e"
#define _SCAN_POWER    "p"
#define _SCAN_INFINITY "infinity"
#define _SCAN_NAN      "nan"
#define _SCAN_NCHARS   "0123456789abcdefghijklmnopqrstuvwxyz_"

// The argument list a format's directives take from.
typedef __builtin_va_list _Scan_VaList;

// The widest unsigned integer, which %ju stores.
typedef unsigned long _Scan_UintmaxType;

// A wide character.
typedef int _Scan_WcharType;

// The characters a format and its source are.
typedef enum _Scan_Text _Scan_Text;
enum _Scan_Text {
    _SCAN_TEXT_NARROW, // bytes, as scanf's are
    _SCAN_TEXT_WIDE    // wide characters, as wscanf's are
};

// What a wide stream's source has read ahead of the characters taken.
typedef enum _Scan_Ahead _Scan_Ahead;
enum _Scan_Ahead {
    _SCAN_AHEAD_NONE, // nothing
    _SCAN_AHEAD_CHAR, // a wide character, pushed back if it is not taken
    _SCAN_AHEAD_END   // the stream's end, or bytes that are no character
};

// A directive's length modifier.
typedef enum _Scan_Length _Scan_Length;
enum _Scan_Length {
    _SCAN_LENGTH_NONE,    // none
    _SCAN_LENGTH_CHAR,    // hh
    _SCAN_LENGTH_SHORT,   // h
    _SCAN_LENGTH_LONG,    // l
    _SCAN_LENGTH_LLONG,   // ll
    _SCAN_LENGTH_INTMAX,  // j
    _SCAN_LENGTH_SIZE,    // z
    _SCAN_LENGTH_PTRDIFF, // t
    _SCAN_LENGTH_LDOUBLE  // L
};

// What a directive converts, the conversions alike in all but a field as one.
typedef enum _Scan_Kind _Scan_Kind;
enum _Scan_Kind {
    _SCAN_KIND_PERCENT, // %%, a percent sign
    _SCAN_KIND_CHARS,   // %c and %lc, as many characters as the width
    _SCAN_KIND_STRING,  // %s and %ls, a word
    _SCAN_KIND_SET,     // %[ and %l[, the characters a scanset takes
    _SCAN_KIND_COUNT,   // %n, the characters read so far, stored
    _SCAN_KIND_INTEGER, // %d %i %o %u %x %X, an integer in a base
    _SCAN_KIND_POINTER, // %p, a pointer
    _SCAN_KIND_REAL     // %a %e %f %g and their capitals, a floating number
};

// The characters a directive stores what it reads as.
typedef enum _Scan_Chars _Scan_Chars;
enum _Scan_Chars {
    _SCAN_CHARS_NARROW, // the bytes read, as they are
    _SCAN_CHARS_WIDE    // the wide characters the bytes encode
};

// Whether an integer is converted as a signed one.
typedef enum _Scan_Sign _Scan_Sign;
enum _Scan_Sign {
    _SCAN_SIGN_SIGNED,  // as %d and %i do
    _SCAN_SIGN_UNSIGNED // as %o, %u and %x do
};

// What a directive does with the white space before its input.
typedef enum _Scan_Space _Scan_Space;
enum _Scan_Space {
    _SCAN_SPACE_SKIP, // skip it, as most do
    _SCAN_SPACE_KEEP  // read it as input, as %c and %[ do
};

// What a directive does with what it converts.
typedef enum _Scan_Assign _Scan_Assign;
enum _Scan_Assign {
    _SCAN_ASSIGN_STORE,   // store it at the next argument
    _SCAN_ASSIGN_SUPPRESS // store nothing, the directive having an asterisk
};

// The characters a scanset takes: those it lists, or all the others.
typedef enum _Scan_Accept _Scan_Accept;
enum _Scan_Accept {
    _SCAN_ACCEPT_LISTED,  // the characters between its brackets
    _SCAN_ACCEPT_UNLISTED // every other, the scanset beginning with ^
};

// What stopped formatted input: nothing, no input or input that does not fit.
typedef enum _Scan_Failure _Scan_Failure;
enum _Scan_Failure {
    _SCAN_FAILURE_NONE,    // nothing has
    _SCAN_FAILURE_INPUT,   // the input ended, could not be read or decoded
    _SCAN_FAILURE_MATCHING // the input is not what a directive takes
};

// Where formatted input comes from: a stream, or else a string.
typedef struct _Scan_Source _Scan_Source;
struct _Scan_Source {
    _Scan_Text ss_text;       // the characters it and its format are
    _File_Stream *ss_stream;  // the stream, or _SCAN_NULL for a string
    const void *ss_str;       // the string of its text, if no stream
    _Scan_Ahead ss_ahead;     // what was read ahead of a wide stream
    _Scan_WcharType ss_wc;    // the wide character read ahead, if one was
    unsigned long ss_count;   // the characters read so far
    int ss_stored;            // the assignments made so far
    _Scan_Failure ss_failure; // what stopped the input, if anything has
};

// A conversion: its letter, the lengths that go with it and what they mean.
typedef struct _Scan_Rule _Scan_Rule;
struct _Scan_Rule {
    char sr_letter;       // the conversion's letter
    int sr_lengths;       // the lengths it allows, _SCAN_LENGTHS_*
    _Scan_Kind sr_kind;   // what it converts
    _Scan_Chars sr_chars; // the characters it stores as
    _Scan_Space sr_space; // what it does with white space before its input
    _Scan_Sign sr_sign;   // whether an integer is signed
    int sr_base;          // an integer's base
};

// A directive as the format gives it, with where it stores.
typedef struct _Scan_Directive _Scan_Directive;
struct _Scan_Directive {
    _Scan_Kind sd_kind;     // what it converts
    int sd_width;           // its width, or _SCAN_WIDTH_NONE
    _Scan_Length sd_length; // its length modifier
    _Scan_Chars sd_chars;   // the characters it stores as
    _Scan_Space sd_space;   // what it does with white space before its input
    _Scan_Sign sd_sign;     // whether an integer is signed
    int sd_base;            // an integer's base
    void *sd_dest;          // where it stores, or _SCAN_NULL for nowhere
    const void *sd_set;     // the format a scanset's characters are in
    unsigned long sd_from;  // where in it they begin
    unsigned long sd_count; // how many they are
    _Scan_Accept sd_accept; // whether the scanset takes them or the others
};

// An input item of any length: its characters, and how many more it may take.
typedef struct _Scan_Item _Scan_Item;
struct _Scan_Item {
    char *si_text;         // the characters and a null, allocated, or null
    unsigned long si_len;  // how many characters there are
    unsigned long si_size; // the bytes si_text points to
    unsigned long si_room; // the characters the directive's width still allows
};

// Where a directive's characters go, converted if the source's are not such.
typedef struct _Scan_Target _Scan_Target;
struct _Scan_Target {
    _Scan_Chars st_chars;          // the characters it stores as
    char *st_str;                  // where the next byte goes, or null
    _Scan_WcharType *st_wstr;      // where the next wide one goes, or null
    char st_byte[_SCAN_BYTES_MAX]; // the bytes of a character not yet whole
    unsigned long st_count;        // how many such bytes there are
};

// The conversions, a letter that means two things by its length being two.
extern const _Scan_Rule _Scan_Rules[_SCAN_RULES];

// Characters
int   _Scan_GetChar(const void *format, _Scan_Text text, unsigned long index);
_Bool _Scan_IsSpace(int ch, _Scan_Text text);
_Bool _Scan_IsDigit(int ch);
_Bool _Scan_IsHexDigit(int ch);
_Bool _Scan_IsInSet(const _Scan_Directive *dir, _Scan_Text text, int ch);
_Bool _Scan_IsTaken(const _Scan_Directive *dir, _Scan_Text text, int ch);

// Converting
int           _Scan_ToLower(int ch);
long          _Scan_ToSigned(const char *str, int base);
unsigned long _Scan_ToUnsigned(const char *str, int base);
_Bool         _Scan_ToReal(const char *str, _Scan_Length length, void *dest);

// Source
int  _Scan_PeekSource(_Scan_Source *source);
void _Scan_AdvanceSource(_Scan_Source *source);
void _Scan_FailSource(_Scan_Source *source, int ch);
int  _Scan_EndSource(_Scan_Source *source);

// Matching
unsigned long     _Scan_MatchSpace(const void *format, _Scan_Text text, unsigned long index);
unsigned long     _Scan_MatchText(const void *format, _Scan_Text text, unsigned long index);
unsigned long     _Scan_MatchHead(const void *format, _Scan_Text text, unsigned long index, _Scan_Assign *assign, _Scan_Directive *dir);
unsigned long     _Scan_MatchSet(const void *format, _Scan_Text text, unsigned long index, _Scan_Directive *dir);
const _Scan_Rule *_Scan_FindRule(int letter, _Scan_Length length);
unsigned long     _Scan_MatchDirective(const void *format, _Scan_Text text, unsigned long index, _Scan_VaList *list, _Scan_Directive *dir);

// Items
unsigned long _Scan_TakeDigits(_Scan_Source *source, int width, int base, char *item);
void          _Scan_StoreInteger(void *dest, _Scan_Length length, unsigned long long value);
unsigned long _Scan_TakeChars(_Scan_Source *source, _Scan_Item *item, const char *set, unsigned long max);
unsigned long _Scan_TakeWord(_Scan_Source *source, _Scan_Item *item, const char *word);
void          _Scan_TakeReal(_Scan_Source *source, int width, _Scan_Item *item);

// Target
void _Scan_BeginTarget(_Scan_Target *target, const _Scan_Directive *dir);
void _Scan_PutChar(_Scan_Source *source, _Scan_Target *target, int ch);
void _Scan_EndTarget(_Scan_Source *source, _Scan_Target *target, _Scan_Kind kind);

// Formatted input
void _Scan_SkipSpace(_Scan_Source *source);
void _Scan_ReadText(_Scan_Source *source, const void *format, unsigned long index, unsigned long len);
void _Scan_ReadPercent(_Scan_Source *source);
void _Scan_ReadChars(_Scan_Source *source, const _Scan_Directive *dir);
void _Scan_ReadInteger(_Scan_Source *source, const _Scan_Directive *dir);
void _Scan_ReadPointer(_Scan_Source *source, const _Scan_Directive *dir);
void _Scan_ReadReal(_Scan_Source *source, const _Scan_Directive *dir);
void _Scan_ReadDirective(_Scan_Source *source, const _Scan_Directive *dir);
int  _Scan_ReadFormat(_Scan_Source *source, const void *format, _Scan_VaList args);

#endif // _SCAN_H
