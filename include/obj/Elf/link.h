#ifndef LINK_H
#define LINK_H

#include <stdint.h>
#include "obj/Elf/elf.h"

// Most -place requests one link may carry.
#define ELF_LINK_MAX_PLACE 16

// One -place request: load the named section at a fixed address.
typedef struct Elf_Link_Place Elf_Link_Place;
struct Elf_Link_Place {
    const char *lp_name;
    uint64_t    lp_addr;
};

// Options controlling a link.
typedef struct Elf_Link_Options Elf_Link_Options;
struct Elf_Link_Options {
    const char     *lo_entry;        // entry symbol (NULL selects _start)
    int             lo_relocatable;  // -r: merge into an ET_REL object, keep relocs
    Elf_Link_Place  lo_places[ELF_LINK_MAX_PLACE];
    int             lo_nplaces;
};

// Object indices and global lookup
long     Elf_Link_SectionIndex(const Elf *elf, const Elf_Sec *target);
long     Elf_Link_SymbolIndex(const Elf *elf, const Elf_Sym *target);
Elf_Sym *Elf_Link_FindGlobal(Elf *elf, const char *name);

// Merging objects
void Elf_Link_Merge(Elf *out, Elf *in);
void Elf_Link_MergeFiles(Elf *out, const char *const *paths, int npaths);

// Placing sections and checking symbols
uint64_t Elf_Link_PlacedAddr(const Elf_Link_Options *opts, const char *name, int *placed);
void     Elf_Link_PlaceSections(Elf *elf, const Elf_Link_Options *opts);
void     Elf_Link_CheckDefined(Elf *elf);

// Linking
void Elf_Link_Exec(Elf *elf, const Elf_Link_Options *opts);
Elf *Elf_Link_Run(const char *const *paths, int npaths, const Elf_Link_Options *opts);

#endif // LINK_H
