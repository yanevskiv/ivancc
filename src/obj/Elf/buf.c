#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "obj/Elf/buf.h"

// Initialize an empty byte buffer.
void Elf_Buffer_Init(Elf_Buffer *buf)
{
    buf->eb_data = NULL;
    buf->eb_len  = 0;
    buf->eb_cap  = 0;
}

// Free a buffer's storage and clear it.
void Elf_Buffer_Free(Elf_Buffer *buf)
{
    free(buf->eb_data);
    buf->eb_data = NULL;
    buf->eb_len  = 0;
    buf->eb_cap  = 0;
}

// Grow a buffer so it can hold at least n more bytes.
void Elf_Buffer_Reserve(Elf_Buffer *buf, size_t n)
{
    if (buf->eb_len + n <= buf->eb_cap) {
        return;
    }
    size_t cap = buf->eb_cap ? buf->eb_cap : 256;
    while (buf->eb_len + n > cap) {
        cap *= 2;
    }
    buf->eb_data = realloc(buf->eb_data, cap);
    buf->eb_cap  = cap;
}

// Return a pointer to byte off within a buffer, for in-place patching.
void *Elf_Buffer_At(Elf_Buffer *buf, size_t off)
{
    return buf->eb_data + off;
}

// Append one byte, returning the offset it began at.
size_t Elf_Buffer_Byte(Elf_Buffer *buf, uint8_t value)
{
    size_t off = buf->eb_len;
    Elf_Buffer_Reserve(buf, 1);
    buf->eb_data[buf->eb_len++] = value;
    return off;
}

// Append n raw bytes, returning the offset they began at.
size_t Elf_Buffer_Data(Elf_Buffer *buf, const void *data, size_t n)
{
    size_t off = buf->eb_len;
    Elf_Buffer_Reserve(buf, n);
    memcpy(buf->eb_data + buf->eb_len, data, n);
    buf->eb_len += n;
    return off;
}

// Append a little-endian 16-bit value, returning its offset.
size_t Elf_Buffer_U16(Elf_Buffer *buf, uint16_t value)
{
    uint8_t bytes[2] = { value & 0xFF, (value >> 8) & 0xFF };
    return Elf_Buffer_Data(buf, bytes, 2);
}

// Append a little-endian 32-bit value, returning its offset.
size_t Elf_Buffer_U32(Elf_Buffer *buf, uint32_t value)
{
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Elf_Buffer_Data(buf, bytes, 4);
}

// Append a little-endian 64-bit value, returning its offset.
size_t Elf_Buffer_U64(Elf_Buffer *buf, uint64_t value)
{
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Elf_Buffer_Data(buf, bytes, 8);
}

// Append n zero bytes, returning the offset they began at.
size_t Elf_Buffer_Zero(Elf_Buffer *buf, size_t n)
{
    size_t off = buf->eb_len;
    Elf_Buffer_Reserve(buf, n);
    memset(buf->eb_data + buf->eb_len, 0, n);
    buf->eb_len += n;
    return off;
}

// Pad the buffer with zeros up to a multiple of align, returning new length.
size_t Elf_Buffer_Align(Elf_Buffer *buf, size_t align)
{
    if (align > 1) {
        while (buf->eb_len % align != 0) {
            Elf_Buffer_Byte(buf, 0);
        }
    }
    return buf->eb_len;
}
