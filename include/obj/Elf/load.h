#ifndef ELF_LOAD_H
#define ELF_LOAD_H

#include <stdint.h>

#include "obj/Elf/types.h"

// Address arithmetic
uint64_t Elf_Load_AlignDown(uint64_t addr, uint64_t align);
uint64_t Elf_Load_AlignUp(uint64_t addr, uint64_t align);

// Loading an executable into flat memory
int   Elf_Load_ReadExec(const char *path, Elf_LoadImage *img);
void *Elf_Load_At(const Elf_LoadImage *img, uint64_t vaddr, uint64_t size);
void  Elf_Load_Free(Elf_LoadImage *img);

#endif // ELF_LOAD_H
