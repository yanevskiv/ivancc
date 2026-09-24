// C source file for string utilities.

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"
#include "util/str.h"

// Return an owned copy of str.
char *Str_Clone(const char *str)
{
    return str ? Str_Slice(str, 0, (int) strlen(str)) : NULL;
}

// Return an owned copy of the bytes of str from start up to end.
char *Str_Slice(const char *str, size_t start, size_t end)
{
    size_t len = strlen(str);

    if (start > end || end > len) {
        Log_ShowError("slice [%zu, %zu) of a string of %zu bytes", start, end, len);
    }

    size_t want = end - start;
    char *out = malloc(want + 1);

    memcpy(out, str + start, want);
    out[want] = '\0';
    return out;
}

// Format an owned string like printf(3).
char *Str_Format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *out = Str_FormatVa(fmt, ap);
    va_end(ap);
    return out;
}

// Format an owned string from a va_list.
char *Str_FormatVa(const char *fmt, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);

    int len = vsnprintf(NULL, 0, fmt, ap);
    char *out = malloc(len + 1);
    vsnprintf(out, len + 1, fmt, ap2);
    va_end(ap2);
    return out;
}

// Change or append a file extension ('main.c' -> 'main.s').
char *Str_ChangeOrAppendExt(const char *input, const char *suffix)
{
    const char *slash = strrchr(input, '/');
    const char *dot = strrchr(input, '.');

    if (! dot || (slash && dot < slash)) {
        dot = NULL;
    }

    size_t stem = dot ? (size_t) (dot - input) : strlen(input);
    size_t slen = strlen(suffix);
    char *out = malloc(stem + slen + 1);
    memcpy(out, input, stem);
    memcpy(out + stem, suffix, slen + 1);
    return out;
}

// Test whether two strings are equal.
int Str_Equals(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

// Test whether str begins with prefix.
int Str_StartsWith(const char *str, const char *prefix)
{
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

// Trim leading and trailing whitespace in place.
char *Str_Trim(char *str)
{
    while (*str && strchr(" \t\r\n\f\v", *str)) {
        str++;
    }
    char *end = str + strlen(str);
    while (end > str && strchr(" \t\r\n\f\v", end[-1])) {
        *--end = '\0';
    }
    return str;
}

// Free str, ignoring NULL.
void Str_Free(char *str)
{
    if (str) {
        free(str);
    }
}

// Split str on sep into a list of owned pieces.
Str_List Str_Split(const char *str, const char *sep)
{
    Str_List list = { NULL, 0 };
    size_t seplen = strlen(sep);

    const char *start = str;
    for (;;) {
        const char *hit = strstr(start, sep);
        size_t len = hit ? (size_t) (hit - start) : strlen(start);

        list.sl_items = realloc(list.sl_items, (list.sl_count + 1) * sizeof(*list.sl_items));
        list.sl_items[list.sl_count++] = Str_Slice(start, 0, len);

        if (! hit) {
            break;
        }
        start = hit + seplen;
    }
    return list;
}

// Free a list and its pieces.
void Str_ListFree(Str_List *list)
{
    for (size_t i = 0; i < list->sl_count; i++) {
        Str_Free(list->sl_items[i]);
    }
    free(list->sl_items);
    list->sl_items = NULL;
    list->sl_count = 0;
}

// Return the value of a digit, or -1 when it is not one.
int Str_DigitValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + STR_BASE_DECIMAL;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + STR_BASE_DECIMAL;
    }
    return -1;
}

// Scan up to count digits of the given base, advancing the position.
unsigned long Str_ScanDigits(const char *p, size_t len, size_t *pos, int base, size_t count)
{
    unsigned long value = 0;

    for (size_t n = 0; n < count && *pos < len; n++) {
        int digit = Str_DigitValue(p[*pos]);
        if (digit < 0 || digit >= base) {
            break;
        }
        value = value * base + digit;
        (*pos)++;
    }
    return value;
}

// Read one little-endian element of width bytes.
unsigned long Str_GetValue(const char *p, size_t width)
{
    unsigned long value = 0;

    for (size_t i = 0; i < width; i++) {
        value |= (unsigned long) (unsigned char) p[i] << (i * STR_BITS_PER_BYTE);
    }
    return value;
}

// Append one little-endian element of width bytes holding value.
void Str_PutValue(char *buf, size_t *len, size_t width, unsigned long value)
{
    for (size_t i = 0; i < width; i++) {
        buf[(*len)++] = (char) (value >> (i * STR_BITS_PER_BYTE));
    }
}

// Append the element an escape sequence stands for.
void Str_PutEscape(char *buf, size_t *len, size_t width, unsigned long value)
{
    unsigned long room = ~0UL >> (STR_LONG_BITS - width * STR_BITS_PER_BYTE);

    if (value > room) {
        Log_ShowError("escape sequence out of range for a %zu-byte character", width);
    }
    Str_PutValue(buf, len, width, value);
}

// Append a code point as its UTF-8 bytes.
void Str_PutUtf8(char *buf, size_t *len, unsigned long value)
{
    if (value < STR_UTF8_MAX_ONE) {
        Str_PutValue(buf, len, STR_NARROW_WIDTH, value);
        return;
    }

    size_t n = STR_UTF8_LEN_TWO;

    if (value >= STR_UTF8_MAX_THREE) {
        n = STR_UTF8_LEN_FOUR;
    } else if (value >= STR_UTF8_MAX_TWO) {
        n = STR_UTF8_LEN_THREE;
    }

    unsigned long lead = (STR_BYTE_MASK << (STR_BITS_PER_BYTE - n)) & STR_BYTE_MASK;

    Str_PutValue(buf, len, STR_NARROW_WIDTH, lead | (value >> ((n - 1) * STR_UTF8_SHIFT)));
    for (int k = n - 1; k > 0; k--) {
        Str_PutValue(buf, len, STR_NARROW_WIDTH, STR_UTF8_CONT | ((value >> ((k - 1) * STR_UTF8_SHIFT)) & STR_UTF8_MASK));
    }
}

// Decode a quoted-literal body into elements of width bytes.
char *Str_Unescape(const char *p, size_t len, size_t width, size_t *out_len)
{
    size_t n = 0;
    char *buf = calloc(len + 1, STR_MAX_ELEMENT_SIZE);

    for (size_t i = 0; i < len; i++) {
        if (p[i] == '"') {
            break;
        }
        if (p[i] != '\\' || i + 1 == len) {
            Str_PutValue(buf, &n, width, (unsigned char) p[i]);
            continue;
        }

        size_t pos = i + 2;

        switch (p[i + 1]) {
            case 'a': {
                Str_PutValue(buf, &n, width, '\a');
            } break;
            case 'b': {
                Str_PutValue(buf, &n, width, '\b');
            } break;
            case 'f': {
                Str_PutValue(buf, &n, width, '\f');
            } break;
            case 'n': {
                Str_PutValue(buf, &n, width, '\n');
            } break;
            case 'r': {
                Str_PutValue(buf, &n, width, '\r');
            } break;
            case 't': {
                Str_PutValue(buf, &n, width, '\t');
            } break;
            case 'v': {
                Str_PutValue(buf, &n, width, '\v');
            } break;
            case '\\': {
                Str_PutValue(buf, &n, width, '\\');
            } break;
            case '\'': {
                Str_PutValue(buf, &n, width, '\'');
            } break;
            case '"': {
                Str_PutValue(buf, &n, width, '"');
            } break;
            case '?': {
                Str_PutValue(buf, &n, width, '?');
            } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                pos = i + 1;
                Str_PutEscape(buf, &n, width, Str_ScanDigits(p, len, &pos, STR_BASE_OCTAL, STR_MAX_OCTAL_DIGITS));
            } break;
            case 'x': {
                size_t start = pos;
                unsigned long value = Str_ScanDigits(p, len, &pos, STR_BASE_HEX, STR_MAX_HEX_DIGITS);
                if (pos == start) {
                    Log_ShowError("\\x used with no following hex digits");
                }
                Str_PutEscape(buf, &n, width, value);
            } break;
            case 'u':
            case 'U': {
                size_t count = p[i + 1] == 'u' ? STR_UCN_SHORT_DIGITS : STR_UCN_LONG_DIGITS;
                size_t start = pos;
                unsigned long value = Str_ScanDigits(p, len, &pos, STR_BASE_HEX, count);
                if (pos - start != count) {
                    Log_ShowError("incomplete universal character name");
                }
                if (width > STR_NARROW_WIDTH) {
                    Str_PutEscape(buf, &n, width, value);
                } else {
                    Str_PutUtf8(buf, &n, value);
                }
            } break;
            default: {
                Log_ShowError("unknown escape sequence '\\%c'", p[i + 1]);
            } break;
        }
        i = pos - 1;
    }

    *out_len = n;
    return buf;
}
