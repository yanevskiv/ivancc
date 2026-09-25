// C source file for string utilities.

// Module header.
#include "util/str.h"

// The storage behind a Str_Buf.
struct Str_Buf {
    char  *sb_data;
    size_t sb_len;
    size_t sb_cap;
};

// Return an owned copy of str.
char *Str_Clone(const char *str)
{
    return str ? Str_Slice(str, 0, strlen(str)) : NULL;
}

// Return an owned copy of the bytes of str from start up to end.
char *Str_Slice(const char *str, size_t start, size_t end)
{
    size_t len = strlen(str);

    Err_Assert(start <= end && end <= len, ERR_STR_SLICE_OUT_OF_RANGE, start, end, len);

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

    size_t len = (size_t) vsnprintf(NULL, 0, fmt, ap);
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
bool Str_Equals(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

// Test whether str begins with prefix.
bool Str_StartsWith(const char *str, const char *prefix)
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

// Start an empty buffer.
Str_Buf *Str_BufNew(void)
{
    Str_Buf *buf = malloc(sizeof(*buf));

    buf->sb_data = malloc(STR_BUF_MIN_CAP);
    buf->sb_data[0] = '\0';
    buf->sb_len = 0;
    buf->sb_cap = STR_BUF_MIN_CAP;
    return buf;
}

// Return the text a buffer holds.
const char *Str_BufData(const Str_Buf *buf)
{
    return buf->sb_data;
}

// Return the length of the text a buffer holds.
size_t Str_BufLen(const Str_Buf *buf)
{
    return buf->sb_len;
}

// Make room for n more bytes.
void Str_BufReserve(Str_Buf *buf, size_t n)
{
    size_t want = buf->sb_len + n + 1;

    if (want <= buf->sb_cap) {
        return;
    }
    while (buf->sb_cap < want) {
        buf->sb_cap *= 2;
    }
    buf->sb_data = realloc(buf->sb_data, buf->sb_cap);
}

// Append one byte.
void Str_BufPutByte(Str_Buf *buf, char byte)
{
    Str_BufReserve(buf, 1);
    buf->sb_data[buf->sb_len++] = byte;
    buf->sb_data[buf->sb_len] = '\0';
}

// Append len bytes.
void Str_BufPutBytes(Str_Buf *buf, const char *data, size_t len)
{
    Str_BufReserve(buf, len);
    memcpy(buf->sb_data + buf->sb_len, data, len);
    buf->sb_len += len;
    buf->sb_data[buf->sb_len] = '\0';
}

// Append a NUL-terminated string.
void Str_BufPutText(Str_Buf *buf, const char *text)
{
    Str_BufPutBytes(buf, text, strlen(text));
}

// Append text formatted like printf(3).
void Str_BufPrint(Str_Buf *buf, const char *fmt, ...)
{
    va_list ap;
    va_list ap2;

    va_start(ap, fmt);
    va_copy(ap2, ap);

    size_t len = (size_t) vsnprintf(NULL, 0, fmt, ap);

    Str_BufReserve(buf, len);
    vsnprintf(buf->sb_data + buf->sb_len, len + 1, fmt, ap2);
    buf->sb_len += len;
    va_end(ap2);
    va_end(ap);
}

// Turn a buffer into its text.
char *Str_BufTake(Str_Buf *buf)
{
    char *data = buf->sb_data;

    free(buf);
    return data;
}

// Free a buffer and its text.
void Str_BufFree(Str_Buf *buf)
{
    if (buf) {
        free(buf->sb_data);
        free(buf);
    }
}
