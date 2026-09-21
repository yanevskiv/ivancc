#ifndef BUF_H
#define BUF_H

#include <stdint.h>
#include <stddef.h>

// A growable byte buffer -- the only storage primitive, with no ELF knowledge.
typedef struct Elf_Buffer Elf_Buffer;
struct Elf_Buffer {
    uint8_t *eb_data;
    size_t   eb_len;
    size_t   eb_cap;
};

// Growable byte buffers
void   Elf_Buffer_Init(Elf_Buffer *buf);
void   Elf_Buffer_Free(Elf_Buffer *buf);
void   Elf_Buffer_Reserve(Elf_Buffer *buf, size_t n);
void  *Elf_Buffer_At(Elf_Buffer *buf, size_t off);
size_t Elf_Buffer_Byte(Elf_Buffer *buf, uint8_t value);
size_t Elf_Buffer_Data(Elf_Buffer *buf, const void *data, size_t n);
size_t Elf_Buffer_U16(Elf_Buffer *buf, uint16_t value);
size_t Elf_Buffer_U32(Elf_Buffer *buf, uint32_t value);
size_t Elf_Buffer_U64(Elf_Buffer *buf, uint64_t value);
size_t Elf_Buffer_Zero(Elf_Buffer *buf, size_t n);
size_t Elf_Buffer_Align(Elf_Buffer *buf, size_t align);

#endif // BUF_H
