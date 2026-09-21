#ifndef LINK_H
#define LINK_H

#include <stdint.h>
#include "obj/elf/elf.h"

// Most -place requests one link may carry.
#define LINK_ELF_MAX_PLACE 16

// One -place request: load the named section at a fixed address.
typedef struct Link_Elf_Place Link_Elf_Place;
struct Link_Elf_Place {
    const char *lp_name;
    uint64_t    lp_addr;
};

// Options controlling a link.
typedef struct Link_Elf_Options Link_Elf_Options;
struct Link_Elf_Options {
    const char     *lo_entry;        // entry symbol (NULL selects _start)
    int             lo_relocatable;  // -r: merge into an ET_REL object, keep relocs
    Link_Elf_Place  lo_places[LINK_ELF_MAX_PLACE];
    int             lo_nplaces;
};

// Object indices and global lookup
long     Link_Elf_SectionIndex(const Elf *elf, const Elf_Sec *target);
long     Link_Elf_SymbolIndex(const Elf *elf, const Elf_Sym *target);
Elf_Sym *Link_Elf_FindGlobal(Elf *elf, const char *name);

// Merging objects
void Link_Elf_Merge(Elf *out, Elf *in);
void Link_Elf_MergeFiles(Elf *out, const char *const *paths, int npaths);

// Placing sections and checking symbols
uint64_t Link_Elf_PlacedAddr(const Link_Elf_Options *opts, const char *name, int *placed);
void     Link_Elf_PlaceSections(Elf *elf, const Link_Elf_Options *opts);
void     Link_Elf_CheckDefined(Elf *elf);

// Linking
void Link_Elf_Exec(Elf *elf, const Link_Elf_Options *opts);
Elf *Link_Elf_Run(const char *const *paths, int npaths, const Link_Elf_Options *opts);

#endif // LINK_H
