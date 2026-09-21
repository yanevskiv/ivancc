#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "obj/elf/buf.h"

// Initialize an empty byte buffer.
void Buf_Elf_Init(Buf_Elf *buf)
{
    buf->be_data = NULL;
    buf->be_len  = 0;
    buf->be_cap  = 0;
}

// Free a buffer's storage and clear it.
void Buf_Elf_Free(Buf_Elf *buf)
{
    free(buf->be_data);
    buf->be_data = NULL;
    buf->be_len  = 0;
    buf->be_cap  = 0;
}

// Grow a buffer so it can hold at least n more bytes.
void Buf_Elf_Reserve(Buf_Elf *buf, size_t n)
{
    if (buf->be_len + n <= buf->be_cap) {
        return;
    }
    size_t cap = buf->be_cap ? buf->be_cap : 256;
    while (buf->be_len + n > cap) {
        cap *= 2;
    }
    buf->be_data = realloc(buf->be_data, cap);
    buf->be_cap  = cap;
}

// Return a pointer to byte off within a buffer, for in-place patching.
void *Buf_Elf_At(Buf_Elf *buf, size_t off)
{
    return buf->be_data + off;
}

// Append one byte, returning the offset it began at.
size_t Buf_Elf_Byte(Buf_Elf *buf, uint8_t value)
{
    size_t off = buf->be_len;
    Buf_Elf_Reserve(buf, 1);
    buf->be_data[buf->be_len++] = value;
    return off;
}

// Append n raw bytes, returning the offset they began at.
size_t Buf_Elf_Data(Buf_Elf *buf, const void *data, size_t n)
{
    size_t off = buf->be_len;
    Buf_Elf_Reserve(buf, n);
    memcpy(buf->be_data + buf->be_len, data, n);
    buf->be_len += n;
    return off;
}

// Append a little-endian 16-bit value, returning its offset.
size_t Buf_Elf_U16(Buf_Elf *buf, uint16_t value)
{
    uint8_t bytes[2] = { value & 0xFF, (value >> 8) & 0xFF };
    return Buf_Elf_Data(buf, bytes, 2);
}

// Append a little-endian 32-bit value, returning its offset.
size_t Buf_Elf_U32(Buf_Elf *buf, uint32_t value)
{
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Buf_Elf_Data(buf, bytes, 4);
}

// Append a little-endian 64-bit value, returning its offset.
size_t Buf_Elf_U64(Buf_Elf *buf, uint64_t value)
{
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Buf_Elf_Data(buf, bytes, 8);
}

// Append n zero bytes, returning the offset they began at.
size_t Buf_Elf_Zero(Buf_Elf *buf, size_t n)
{
    size_t off = buf->be_len;
    Buf_Elf_Reserve(buf, n);
    memset(buf->be_data + buf->be_len, 0, n);
    buf->be_len += n;
    return off;
}

// Pad the buffer with zeros up to a multiple of align, returning new length.
size_t Buf_Elf_Align(Buf_Elf *buf, size_t align)
{
    if (align > 1) {
        while (buf->be_len % align != 0) {
            Buf_Elf_Byte(buf, 0);
        }
    }
    return buf->be_len;
}
