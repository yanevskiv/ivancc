#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "obj/Elf/buf.h"
#include "obj/Elf/elf.h"
#include "util/str.h"

// Intern a name into the object's string pool, returning an owned copy.
const char *Elf_Intern(Elf *elf, const char *name)
{
    if (! name) {
        name = "";
    }
    char *copy = malloc(strlen(name) + 1);
    strcpy(copy, name);
    if (elf->elf_npool == elf->elf_cappool) {
        elf->elf_cappool = elf->elf_cappool ? elf->elf_cappool * 2 : 16;
        elf->elf_pool = realloc(elf->elf_pool, elf->elf_cappool * sizeof(*elf->elf_pool));
    }
    elf->elf_pool[elf->elf_npool++] = copy;
    return copy;
}

// Create an empty ELF object of the given type and machine.
Elf *Elf_New(uint16_t type, uint16_t machine)
{
    Elf *elf = calloc(1, sizeof(*elf));
    elf->elf_type    = type;
    elf->elf_machine = machine;
    return elf;
}

// Free an ELF object and everything it owns.
void Elf_Free(Elf *elf)
{
    if (! elf) {
        return;
    }
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        Elf_Buffer_Free(&elf->elf_secs[i]->sec_data);
        free(elf->elf_secs[i]->sec_relas);
        free(elf->elf_secs[i]);
    }
    free(elf->elf_secs);
    for (size_t i = 0; i < elf->elf_nsyms; i++) {
        free(elf->elf_syms[i]);
    }
    free(elf->elf_syms);
    for (size_t i = 0; i < elf->elf_npool; i++) {
        Str_Free(elf->elf_pool[i]);
    }
    free(elf->elf_pool);
    free(elf);
}

// Set the entry virtual address (ET_EXEC).
void Elf_SetEntry(Elf *elf, uint64_t vaddr)
{
    elf->elf_entry = vaddr;
}

// Set the object type (ELF_ET_*).
void Elf_SetType(Elf *elf, uint16_t type)
{
    elf->elf_type = type;
}

// Return the object type (ELF_ET_*).
uint16_t Elf_GetType(const Elf *elf)
{
    return elf->elf_type;
}

// Return the last error message recorded on the object, or NULL.
const char *Elf_Error(const Elf *elf)
{
    return elf->elf_err;
}
