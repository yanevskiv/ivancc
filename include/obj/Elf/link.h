#ifndef ELF_LINK_H
#define ELF_LINK_H

#include "obj/Elf/types.h"

// Object indices and global lookup
long     Elf_Link_SectionIndex(const Elf *elf, const Elf_Sec *target);
long     Elf_Link_SymbolIndex(const Elf *elf, const Elf_Sym *target);
Elf_Sym *Elf_Link_FindGlobal(Elf *elf, const char *name);

// Merging objects
void Elf_Link_Merge(Elf *out, Elf *in);
void Elf_Link_MergeFiles(Elf *out, const char *const *paths, int npaths);

// Placing sections and checking symbols
void     Elf_Link_AddPlace(Elf_LinkOptions *opts, const char *name, uint64_t addr);
uint64_t Elf_Link_PlacedAddr(const Elf_LinkOptions *opts, const char *name, int *placed);
void     Elf_Link_PlaceSections(Elf *elf, const Elf_LinkOptions *opts);
void     Elf_Link_CheckDefined(Elf *elf);

// Linking
void Elf_Link_Exec(Elf *elf, const Elf_LinkOptions *opts);
Elf *Elf_Link_Run(const char *const *paths, int npaths, const Elf_LinkOptions *opts);

#endif // ELF_LINK_H
