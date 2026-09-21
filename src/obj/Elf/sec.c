#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "obj/Elf/buf.h"
#include "obj/Elf/sec.h"
#include "obj/Elf/elf.h"

// Append a new section and return it.
Elf_Sec *Elf_Section_Add(Elf *elf, const char *name, uint32_t type, uint64_t flags)
{
    Elf_Sec *sec = calloc(1, sizeof(*sec));
    sec->sec_name      = Elf_Intern(elf, name);
    sec->sec_type      = type;
    sec->sec_flags     = flags;
    sec->sec_addralign = 1;
    Elf_Buffer_Init(&sec->sec_data);

    if (elf->elf_nsecs == elf->elf_capsecs) {
        elf->elf_capsecs = elf->elf_capsecs ? elf->elf_capsecs * 2 : 8;
        elf->elf_secs = realloc(elf->elf_secs, elf->elf_capsecs * sizeof(*elf->elf_secs));
    }
    elf->elf_secs[elf->elf_nsecs++] = sec;
    return sec;
}

// Find a section by name, or return NULL.
Elf_Sec *Elf_Section_Find(Elf *elf, const char *name)
{
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        if (strcmp(elf->elf_secs[i]->sec_name, name) == 0) {
            return elf->elf_secs[i];
        }
    }
    return NULL;
}

// Find a section by name, creating it with the given type and flags if absent.
Elf_Sec *Elf_Section_Get(Elf *elf, const char *name, uint32_t type, uint64_t flags)
{
    Elf_Sec *sec = Elf_Section_Find(elf, name);
    if (sec) {
        return sec;
    }
    return Elf_Section_Add(elf, name, type, flags);
}

// Return the number of sections.
size_t Elf_Section_Count(const Elf *elf)
{
    return elf->elf_nsecs;
}

// Return section i.
Elf_Sec *Elf_Section_At(const Elf *elf, size_t i)
{
    return elf->elf_secs[i];
}

// Return the byte buffer a section's contents are appended to.
Elf_Buffer *Elf_Section_Data(Elf_Sec *sec)
{
    return &sec->sec_data;
}

// Place a section at a load address.
void Elf_Section_Addr(Elf_Sec *sec, uint64_t addr)
{
    sec->sec_addr = addr;
}
