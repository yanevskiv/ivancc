#ifndef ELF_WRITE_H
#define ELF_WRITE_H

#include <stdint.h>
#include <stdio.h>

#include "obj/Elf/types.h"

// Writing ELF files
uint32_t Elf_Write_Str(Elf_Buffer *strtab, const char *name);
uint32_t Elf_Write_SectionIndex(const Elf *elf, const Elf_Sec *sec, const uint32_t *secidx);
void     Elf_Write_Symtab(const Elf *elf, const uint32_t *secidx, Elf_Buffer *symtab, Elf_Buffer *strtab, uint32_t *slot, uint32_t *first_global);
void     Elf_Write_Relas(const Elf_Sec *sec, const uint32_t *slot, const Elf *elf, Elf_Buffer *out);
int      Elf_Write_Rel(const Elf *elf, FILE *out);
uint32_t Elf_Write_SegFlags(const Elf_Sec *sec);
uint64_t Elf_Write_PlaceOffset(uint64_t pos, uint64_t vaddr);
int      Elf_Write_Exec(const Elf *elf, FILE *out);
int      Elf_Write_File(const Elf *elf, FILE *out);
int      Elf_Write_Path(const Elf *elf, const char *path);

#endif // ELF_WRITE_H
