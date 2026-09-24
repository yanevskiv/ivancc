// C header file for string utilities.

#ifndef STR_H
#define STR_H

#include <stdarg.h>
#include <stddef.h>

// Bits in a byte, for packing one element of a decoded literal.
#define STR_BITS_PER_BYTE 8

// Bits in the unsigned long an escape sequence is read into.
#define STR_LONG_BITS 64

// All bits of one byte set.
#define STR_BYTE_MASK 0xFF

// Element size of a narrow literal.
#define STR_NARROW_WIDTH 1

// Element size of the widest literal.
#define STR_MAX_ELEMENT_SIZE 4

// The bases an escape sequence is written in.
#define STR_BASE_OCTAL   8
#define STR_BASE_DECIMAL 10
#define STR_BASE_HEX     16

// Digits an escape sequence may carry.
#define STR_MAX_OCTAL_DIGITS 3
#define STR_MAX_HEX_DIGITS   8
#define STR_UCN_SHORT_DIGITS 4
#define STR_UCN_LONG_DIGITS  8

// The first code point UTF-8 spends two, three and four bytes on.
#define STR_UTF8_MAX_ONE   0x80
#define STR_UTF8_MAX_TWO   0x800
#define STR_UTF8_MAX_THREE 0x10000

// The byte counts those ranges take.
#define STR_UTF8_LEN_TWO   2
#define STR_UTF8_LEN_THREE 3
#define STR_UTF8_LEN_FOUR  4

// The tag, payload mask and shift of one UTF-8 continuation byte.
#define STR_UTF8_CONT  0x80
#define STR_UTF8_MASK  0x3F
#define STR_UTF8_SHIFT 6

// A list of owned strings, as produced by Str_Split.
typedef struct Str_List Str_List;
struct Str_List {
    char **sl_items;
    size_t sl_count;
};

// String utility functions
char *Str_Clone(const char *str);
char *Str_Slice(const char *str, size_t start, size_t end);
char *Str_Format(const char *fmt, ...);
char *Str_FormatVa(const char *fmt, va_list ap);
char *Str_ChangeOrAppendExt(const char *input, const char *suffix);
int Str_Equals(const char *a, const char *b);
int Str_StartsWith(const char *str, const char *prefix);
char *Str_Trim(char *str);
void Str_Free(char *str);

// String splitting
Str_List Str_Split(const char *str, const char *sep);
void Str_ListFree(Str_List *list);

// C literal escape decoding
int Str_DigitValue(char c);
unsigned long Str_ScanDigits(const char *p, size_t len, size_t *pos, int base, size_t count);
unsigned long Str_GetValue(const char *p, size_t width);
void Str_PutValue(char *buf, size_t *len, size_t width, unsigned long value);
void Str_PutEscape(char *buf, size_t *len, size_t width, unsigned long value);
void Str_PutUtf8(char *buf, size_t *len, unsigned long value);
char *Str_Unescape(const char *p, size_t len, size_t width, size_t *out_len);

#endif // STR_H
