#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "obj/Elf/sym.h"
#include "obj/Elf/elf.h"

// Append a symbol and return it.  sec == NULL records an undefined reference.
Elf_Sym *Elf_Symbol_Add(Elf *elf, const char *name, Elf_Sec *sec, uint64_t value, uint8_t bind, uint8_t type)
{
    Elf_Sym *sym = calloc(1, sizeof(*sym));
    sym->sym_name  = Elf_Intern(elf, name);
    sym->sym_sec   = sec;
    sym->sym_value = value;
    sym->sym_bind  = bind;
    sym->sym_type  = type;

    if (elf->elf_nsyms == elf->elf_capsyms) {
        elf->elf_capsyms = elf->elf_capsyms ? elf->elf_capsyms * 2 : 16;
        elf->elf_syms = realloc(elf->elf_syms, elf->elf_capsyms * sizeof(*elf->elf_syms));
    }
    elf->elf_syms[elf->elf_nsyms++] = sym;
    return sym;
}

// Find a symbol by name, or return NULL.
Elf_Sym *Elf_Symbol_Find(Elf *elf, const char *name)
{
    for (size_t i = 0; i < elf->elf_nsyms; i++) {
        if (strcmp(elf->elf_syms[i]->sym_name, name) == 0) {
            return elf->elf_syms[i];
        }
    }
    return NULL;
}

// Return the number of symbols.
size_t Elf_Symbol_Count(const Elf *elf)
{
    return elf->elf_nsyms;
}

// Return symbol i.
Elf_Sym *Elf_Symbol_At(const Elf *elf, size_t i)
{
    return elf->elf_syms[i];
}
