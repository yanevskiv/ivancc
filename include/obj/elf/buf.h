#ifndef BUF_H
#define BUF_H

#include <stdint.h>
#include <stddef.h>

// A growable byte buffer -- the only storage primitive, with no ELF knowledge.
typedef struct Buf_Elf Buf_Elf;
struct Buf_Elf {
    uint8_t *be_data;
    size_t   be_len;
    size_t   be_cap;
};

// Growable byte buffers
void   Buf_Elf_Init(Buf_Elf *buf);
void   Buf_Elf_Free(Buf_Elf *buf);
void   Buf_Elf_Reserve(Buf_Elf *buf, size_t n);
void  *Buf_Elf_At(Buf_Elf *buf, size_t off);
size_t Buf_Elf_Byte(Buf_Elf *buf, uint8_t value);
size_t Buf_Elf_Data(Buf_Elf *buf, const void *data, size_t n);
size_t Buf_Elf_U16(Buf_Elf *buf, uint16_t value);
size_t Buf_Elf_U32(Buf_Elf *buf, uint32_t value);
size_t Buf_Elf_U64(Buf_Elf *buf, uint64_t value);
size_t Buf_Elf_Zero(Buf_Elf *buf, size_t n);
size_t Buf_Elf_Align(Buf_Elf *buf, size_t align);

#endif // BUF_H
