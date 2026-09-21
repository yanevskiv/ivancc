#ifndef ELF_RELA_H
#define ELF_RELA_H

#include "obj/Elf/types.h"

// Relocations
Elf_Rela *Elf_Rela_Add(Elf_Sec *target, uint64_t offset, Elf_Sym *sym, uint32_t type, int64_t addend);
size_t    Elf_Rela_Count(const Elf_Sec *target);
Elf_Rela *Elf_Rela_At(const Elf_Sec *target, size_t i);

#endif // ELF_RELA_H
