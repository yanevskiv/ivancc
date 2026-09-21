#ifndef ELF_SYM_H
#define ELF_SYM_H

#include "obj/Elf/types.h"

// Symbols
Elf_Sym *Elf_Symbol_Add(Elf *elf, const char *name, Elf_Sec *sec, uint64_t value, uint8_t bind, uint8_t type);
Elf_Sym *Elf_Symbol_Find(Elf *elf, const char *name);
size_t   Elf_Symbol_Count(const Elf *elf);
Elf_Sym *Elf_Symbol_At(const Elf *elf, size_t i);

#endif // ELF_SYM_H
