#ifndef ELF_H
#define ELF_H

#include "obj/Elf/types.h"

// Object lifecycle and header fields
const char *Elf_Intern(Elf *elf, const char *name);
Elf        *Elf_New(uint16_t type, uint16_t machine);
void        Elf_Free(Elf *elf);
void        Elf_SetEntry(Elf *elf, uint64_t vaddr);
void        Elf_SetType(Elf *elf, uint16_t type);
uint16_t    Elf_GetType(const Elf *elf);
const char *Elf_Error(const Elf *elf);

#endif // ELF_H
