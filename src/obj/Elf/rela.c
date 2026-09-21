#include <stdint.h>
#include <stdlib.h>
#include "obj/Elf/rela.h"

// Append a relocation to the section it patches and return it.
Elf_Rela *Elf_Rela_Add(Elf_Sec *target, uint64_t offset, Elf_Sym *sym, uint32_t type, int64_t addend)
{
    if (target->sec_nrelas == target->sec_caprelas) {
        target->sec_caprelas = target->sec_caprelas ? target->sec_caprelas * 2 : 8;
        target->sec_relas = realloc(target->sec_relas, target->sec_caprelas * sizeof(*target->sec_relas));
    }
    Elf_Rela *rel = &target->sec_relas[target->sec_nrelas++];
    rel->rel_offset = offset;
    rel->rel_sym    = sym;
    rel->rel_type   = type;
    rel->rel_addend = addend;
    return rel;
}

// Return the number of relocations patching a section.
size_t Elf_Rela_Count(const Elf_Sec *target)
{
    return target->sec_nrelas;
}

// Return relocation i of a section.
Elf_Rela *Elf_Rela_At(const Elf_Sec *target, size_t i)
{
    return (Elf_Rela *) &target->sec_relas[i];
}
