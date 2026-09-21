#ifndef ELF_SEC_H
#define ELF_SEC_H

#include "obj/Elf/types.h"

// Sections
Elf_Sec    *Elf_Section_Add(Elf *elf, const char *name, uint32_t type, uint64_t flags);
Elf_Sec    *Elf_Section_Find(Elf *elf, const char *name);
Elf_Sec    *Elf_Section_Get(Elf *elf, const char *name, uint32_t type, uint64_t flags);
size_t      Elf_Section_Count(const Elf *elf);
Elf_Sec    *Elf_Section_At(const Elf *elf, size_t i);
Elf_Buffer *Elf_Section_Data(Elf_Sec *sec);
void        Elf_Section_Addr(Elf_Sec *sec, uint64_t addr);

#endif // ELF_SEC_H
