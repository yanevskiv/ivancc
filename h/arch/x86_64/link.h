/*
 * C header file for the x86-64 static linker.
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

#ifndef LINK_X86_64_H
#define LINK_X86_64_H

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
typedef enum Link_x86_64_Trace Link_x86_64_Trace;
enum Link_x86_64_Trace {
    LINK_X86_64_TRACE_NONE,
    LINK_X86_64_TRACE_FILES,   // each file named, as it is read
    LINK_X86_64_TRACE_MEMBERS  // and each archive member pulled in
};

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
    Link_x86_64_Trace  lo_trace;        // -t: what to print as the inputs are read
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
bool     Link_x86_64_MemberNeeded(Elf *out, const Lib_ArMember *member);
void     Link_x86_64_MergeMember(Elf *out, const char *path, const Lib_ArMember *member, const Link_x86_64_Options *opts);
void     Link_x86_64_MergeArchive(Elf *out, const char *path, const Lib_Ar *ar, const Link_x86_64_Options *opts);
Lib_Ar  *Link_x86_64_ReadArchive(const char *path, const uint8_t *bytes, size_t len);
void     Link_x86_64_MergeFiles(Elf *out, const char *const *paths, size_t npaths, const Link_x86_64_Options *opts);
void     Link_x86_64_AddPlace(Link_x86_64_Options *opts, const char *name, uint64_t addr);
uint64_t Link_x86_64_PlacedAddr(const Link_x86_64_Options *opts, const char *name, bool *placed);
void     Link_x86_64_PlaceSections(Elf *elf, const Link_x86_64_Options *opts);
void     Link_x86_64_CheckDefined(Elf *elf);
void     Link_x86_64_Exec(Elf *elf, const Link_x86_64_Options *opts);
Elf     *Link_x86_64_Run(const char *const *paths, size_t npaths, const Link_x86_64_Options *opts);

#endif // LINK_X86_64_H
