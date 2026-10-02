/*
 * C header file for the static linker.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LINK_H
#define LINK_H

// Standard headers.
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/object/elf.h"
#include "util/object/lib.h"
#include "util/str.h"

// How much a link reports of its inputs, as GNU ld's -t given once or twice.
typedef enum Link_Trace Link_Trace;
enum Link_Trace {
    LINK_TRACE_NONE,
    LINK_TRACE_FILES,   // each file named, as it is read
    LINK_TRACE_MEMBERS  // and each archive member pulled in
};

// One -place request: load the named section at a fixed address.
typedef struct Link_Place Link_Place;
struct Link_Place {
    const char *lp_name;
    uint64_t    lp_addr;
};

// Options controlling a link.
typedef struct Link_Options Link_Options;
struct Link_Options {
    const char        *lo_entry;        // entry symbol (NULL selects _start)
    bool               lo_relocatable;  // -r: merge into an ET_REL object, keep relocs
    Link_Place *lo_places;       // -place requests, in the order given
    size_t             lo_nplaces;      // requests lo_places holds
    Link_Trace  lo_trace;        // -t: what to print as the inputs are read
};

// Relocations
uint64_t Link_RelSymbolAddr(const Elf_Sym *sym);
void     Link_RelPatchLE(Elf_Sec *sec, uint64_t offset, uint64_t value, size_t width);
void     Link_x86_64_RelApplyOne(Elf_Sec *sec, const Elf_Rela *rel);
void     Link_RelApplyOne(const Elf *elf, Elf_Sec *sec, const Elf_Rela *rel);
void     Link_RelApply(Elf *elf);

// Objects
int64_t  Link_SectionIndex(const Elf *elf, const Elf_Sec *target);
int64_t  Link_SymbolIndex(const Elf *elf, const Elf_Sym *target);
Elf_Sym *Link_SymbolFindGlobal(Elf *elf, const char *name);
void     Link_Merge(Elf *out, Elf *in, const char *name);

// Archives
Lib_Ar  *Link_ArRead(const char *path, const uint8_t *bytes, size_t len);
bool     Link_ArMemberNeeded(Elf *out, const Lib_ArMember *member);
void     Link_ArMergeMember(Elf *out, const char *path, const Lib_ArMember *member, const Link_Options *opts);
void     Link_ArMerge(Elf *out, const char *path, const Lib_Ar *ar, const Link_Options *opts);

// Placement
void     Link_PlaceAdd(Link_Options *opts, const char *name, uint64_t addr);
uint64_t Link_PlaceAddr(const Link_Options *opts, const char *name, bool *placed);
void     Link_PlaceSections(Elf *elf, const Link_Options *opts);

// Executables
void     Link_ExecCheckDefined(Elf *elf);
void     Link_ExecFinalize(Elf *elf, const Link_Options *opts);

// Linking
void     Link_MergeFiles(Elf *out, const char *const *paths, size_t npaths, const Link_Options *opts);
Elf     *Link_Build(const char *const *paths, size_t npaths, const Link_Options *opts);

#endif // LINK_H
