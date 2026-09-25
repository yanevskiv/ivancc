// C header file for the x86-64 static linker.

#ifndef LINK_X86_64_H
#define LINK_X86_64_H

// Standard headers.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/object/elf.h"

// One -place request: load the named section at a fixed address.
typedef struct Link_x86_64_Place Link_x86_64_Place;
struct Link_x86_64_Place {
    const char *lp_name;
    uint64_t    lp_addr;
};

// Options controlling a link.
typedef struct Link_x86_64_Options Link_x86_64_Options;
struct Link_x86_64_Options {
    const char        *lo_entry;        // entry symbol (NULL selects _start)
    bool               lo_relocatable;  // -r: merge into an ET_REL object, keep relocs
    Link_x86_64_Place *lo_places;       // -place requests, in the order given
    size_t             lo_nplaces;      // requests lo_places holds
};

// Relocations
uint64_t Link_x86_64_RelSymbolAddr(const Elf_Sym *sym);
void     Link_x86_64_RelPatchLE(Elf_Sec *sec, uint64_t offset, uint64_t value, size_t width);
void     Link_x86_64_RelApplyOne(Elf_Sec *sec, const Elf_Rela *rel);
void     Link_x86_64_RelApply(Elf *elf);

// Linking
int64_t  Link_x86_64_SectionIndex(const Elf *elf, const Elf_Sec *target);
int64_t  Link_x86_64_SymbolIndex(const Elf *elf, const Elf_Sym *target);
Elf_Sym *Link_x86_64_FindGlobal(Elf *elf, const char *name);
void     Link_x86_64_Merge(Elf *out, Elf *in);
void     Link_x86_64_MergeFiles(Elf *out, const char *const *paths, size_t npaths);
void     Link_x86_64_AddPlace(Link_x86_64_Options *opts, const char *name, uint64_t addr);
uint64_t Link_x86_64_PlacedAddr(const Link_x86_64_Options *opts, const char *name, bool *placed);
void     Link_x86_64_PlaceSections(Elf *elf, const Link_x86_64_Options *opts);
void     Link_x86_64_CheckDefined(Elf *elf);
void     Link_x86_64_Exec(Elf *elf, const Link_x86_64_Options *opts);
Elf     *Link_x86_64_Run(const char *const *paths, size_t npaths, const Link_x86_64_Options *opts);

#endif // LINK_X86_64_H
