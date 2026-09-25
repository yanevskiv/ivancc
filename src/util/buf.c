// C source file for growable strings.

// Module header.
#include "util/buf.h"

// The storage behind a Buf.
struct Buf {
    char  *buf_data;
    size_t buf_len;
    size_t buf_cap;
};

// Start an empty buffer.
Buf *Buf_New(void)
{
    Buf *buf = malloc(sizeof(*buf));

    buf->buf_data = malloc(BUF_MIN_CAP);
    buf->buf_data[0] = '\0';
    buf->buf_len = 0;
    buf->buf_cap = BUF_MIN_CAP;
    return buf;
}

// Return the text a buffer holds.
const char *Buf_Data(const Buf *buf)
{
    return buf->buf_data;
}

// Return the length of the text a buffer holds.
size_t Buf_Len(const Buf *buf)
{
    return buf->buf_len;
}

// Make room for n more bytes.
void Buf_Reserve(Buf *buf, size_t n)
{
    size_t want = buf->buf_len + n + 1;

    if (want <= buf->buf_cap) {
        return;
    }
    while (buf->buf_cap < want) {
        buf->buf_cap *= 2;
    }
    buf->buf_data = realloc(buf->buf_data, buf->buf_cap);
}

// Append one byte.
void Buf_PutByte(Buf *buf, char byte)
{
    Buf_Reserve(buf, 1);
    buf->buf_data[buf->buf_len++] = byte;
    buf->buf_data[buf->buf_len] = '\0';
}

// Append len bytes.
void Buf_PutBytes(Buf *buf, const char *data, size_t len)
{
    Buf_Reserve(buf, len);
    memcpy(buf->buf_data + buf->buf_len, data, len);
    buf->buf_len += len;
    buf->buf_data[buf->buf_len] = '\0';
}

// Append a NUL-terminated string.
void Buf_PutText(Buf *buf, const char *text)
{
    Buf_PutBytes(buf, text, strlen(text));
}

// Append text formatted like printf(3).
void Buf_Print(Buf *buf, const char *fmt, ...)
{
    va_list ap;
    va_list ap2;

    va_start(ap, fmt);
    va_copy(ap2, ap);

    size_t len = (size_t) vsnprintf(NULL, 0, fmt, ap);

    Buf_Reserve(buf, len);
    vsnprintf(buf->buf_data + buf->buf_len, len + 1, fmt, ap2);
    buf->buf_len += len;
    va_end(ap2);
    va_end(ap);
}

// Turn a buffer into its text.
char *Buf_Take(Buf *buf)
{
    char *data = buf->buf_data;

    free(buf);
    return data;
}

// Free a buffer and its text.
void Buf_Free(Buf *buf)
{
    if (buf) {
        free(buf->buf_data);
        free(buf);
    }
}
