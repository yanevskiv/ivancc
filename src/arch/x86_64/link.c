/*
 * C source file for the x86-64 static linker.
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

// Module header.
#include "arch/x86_64/link.h"

// Virtual address a symbol resolves to.
uint64_t Link_x86_64_RelSymbolAddr(const Elf_Sym *sym)
{
    if (sym->sym_sec) {
        return sym->sym_sec->sec_addr + sym->sym_value;
    }
    return sym->sym_value;
}

// Patch width little-endian bytes at a section offset with value.
void Link_x86_64_RelPatchLE(Elf_Sec *sec, uint64_t offset, uint64_t value, size_t width)
{
    uint8_t *at = Elf_BufferAt(Elf_SectionData(sec), offset);
    for (size_t i = 0; i < width; i++) {
        at[i] = (value >> (8 * i)) & 0xFF;
    }
}

// Apply one relocation, computing S (symbol), A (addend) and P (patch site).
void Link_x86_64_RelApplyOne(Elf_Sec *sec, const Elf_Rela *rel)
{
    uint64_t S = Link_x86_64_RelSymbolAddr(rel->rel_sym);
    int64_t A = rel->rel_addend;
    uint64_t P = sec->sec_addr + rel->rel_offset;

    switch (rel->rel_type) {
        case R_X86_64_PC32:
        case R_X86_64_PLT32: {
            Link_x86_64_RelPatchLE(sec, rel->rel_offset, (uint32_t) (int32_t) (S + A - P), 4);
        } break;
        case R_X86_64_32:
        case R_X86_64_32S: {
            Link_x86_64_RelPatchLE(sec, rel->rel_offset, (uint32_t) (S + A), 4);
        } break;
        case R_X86_64_64: {
            Link_x86_64_RelPatchLE(sec, rel->rel_offset, S + A, 8);
        } break;
        default: {
            Err_Raise(ERR_LINK_RELOCATION_NOT_SUPPORTED, rel->rel_type);
        }
    }
}

// Apply every relocation in a placed object, patching each section's bytes.
void Link_x86_64_RelApply(Elf *elf)
{
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        Elf_Sec *sec = Elf_SectionAt(elf, i);
        for (size_t r = 0; r < Elf_RelaCount(sec); r++) {
            Link_x86_64_RelApplyOne(sec, Elf_RelaAt(sec, r));
        }
    }
}

// Index of a section within an object.
int64_t Link_x86_64_SectionIndex(const Elf *elf, const Elf_Sec *target)
{
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        if (Elf_SectionAt(elf, i) == target) {
            return (int64_t) i;
        }
    }
    return -1;
}

// Index of a symbol within an object.
int64_t Link_x86_64_SymbolIndex(const Elf *elf, const Elf_Sym *target)
{
    for (size_t i = 0; i < Elf_SymbolCount(elf); i++) {
        if (Elf_SymbolAt(elf, i) == target) {
            return (int64_t) i;
        }
    }
    return -1;
}

// Find an existing global symbol by name.
Elf_Sym *Link_x86_64_FindGlobal(Elf *elf, const char *name)
{
    for (size_t i = 0; i < Elf_SymbolCount(elf); i++) {
        Elf_Sym *sym = Elf_SymbolAt(elf, i);
        if (sym->sym_bind != ELF_BIND_LOCAL && Str_Equals(sym->sym_name, name)) {
            return sym;
        }
    }
    return NULL;
}

// Merge one input object into the output.
void Link_x86_64_Merge(Elf *out, Elf *in)
{
    size_t nsec = Elf_SectionCount(in);
    size_t nsym = Elf_SymbolCount(in);

    Elf_Sec **secmap = calloc(nsec ? nsec : 1, sizeof(*secmap));
    Elf_Sym **symmap = calloc(nsym ? nsym : 1, sizeof(*symmap));
    uint64_t *secbase = calloc(nsec ? nsec : 1, sizeof(*secbase));

    // Phase: merge section bytes, recording each input section's new base.
    for (size_t i = 0; i < nsec; i++) {
        Elf_Sec *sec = Elf_SectionAt(in, i);
        Elf_Sec *dst = Elf_SectionGet(out, sec->sec_name, sec->sec_type, sec->sec_flags);
        Elf_Buffer *db = Elf_SectionData(dst);
        if (sec->sec_addralign > dst->sec_addralign) {
            dst->sec_addralign = sec->sec_addralign;
        }
        Elf_BufferAlign(db, sec->sec_addralign);
        secbase[i] = db->eb_len;
        Elf_BufferData(db, sec->sec_data.eb_data, sec->sec_data.eb_len);
        secmap[i] = dst;
    }

    // Phase: copy symbols, unifying globals and resolving undefined references.
    for (size_t i = 0; i < nsym; i++) {
        Elf_Sym *sym = Elf_SymbolAt(in, i);
        Elf_Sec *dsec = NULL;
        uint64_t value = 0;
        if (sym->sym_sec) {
            int64_t j = Link_x86_64_SectionIndex(in, sym->sym_sec);
            dsec  = secmap[j];
            value = secbase[j] + sym->sym_value;
        }

        if (sym->sym_bind == ELF_BIND_LOCAL) {
            symmap[i] = Elf_SymbolAdd(out, sym->sym_name, dsec, value, ELF_BIND_LOCAL, sym->sym_type);
            continue;
        }

        Elf_Sym *existing = Link_x86_64_FindGlobal(out, sym->sym_name);
        if (! existing) {
            symmap[i] = Elf_SymbolAdd(out, sym->sym_name, dsec, value, sym->sym_bind, sym->sym_type);
            continue;
        }
        if (dsec) {
            Err_Assert(! existing->sym_sec, ERR_LINK_MULTIPLE_DEFINITION, sym->sym_name);
            existing->sym_sec   = dsec;
            existing->sym_value = value;
            existing->sym_type  = sym->sym_type;
        }
        symmap[i] = existing;
    }

    // Phase: rebase each relocation onto the merged section and out symbol.
    for (size_t i = 0; i < nsec; i++) {
        Elf_Sec *sec = Elf_SectionAt(in, i);
        for (size_t r = 0; r < Elf_RelaCount(sec); r++) {
            Elf_Rela *rel = Elf_RelaAt(sec, r);
            int64_t k = Link_x86_64_SymbolIndex(in, rel->rel_sym);
            if (k < 0) {
                continue;
            }
            Elf_RelaAdd(secmap[i], secbase[i] + rel->rel_offset, symmap[k], rel->rel_type, rel->rel_addend);
        }
    }

    free(secmap);
    free(secbase);
    free(symmap);
}

// True if a member defines a symbol out references but has not defined.
bool Link_x86_64_MemberNeeded(Elf *out, const Lib_ArMember *member)
{
    for (const char **iter = member->lam_globals; *iter; iter++) {
        Elf_Sym *sym = Link_x86_64_FindGlobal(out, *iter);
        if (sym && ! sym->sym_sec) {
            return true;
        }
    }
    return false;
}

// Read one archive member as an object and merge it into out.
void Link_x86_64_MergeMember(Elf *out, const char *path, const Lib_ArMember *member, const Link_x86_64_Options *opts)
{
    Elf *in = Elf_ReadMem(member->lam_data, member->lam_size);
    Err_Assert(in, ERR_LINK_MEMBER_NOT_READABLE, member->lam_name, path);
    if (opts->lo_trace >= LINK_X86_64_TRACE_MEMBERS) {
        printf("(%s)%s\n", path, member->lam_name);
    }
    Link_x86_64_Merge(out, in);
    Elf_Free(in);
}

// Merge each member out needs, scanning again until a pass pulls in none.
void Link_x86_64_MergeArchive(Elf *out, const char *path, const Lib_Ar *ar, const Link_x86_64_Options *opts)
{
    bool pulled = true;
    while (pulled) {
        pulled = false;
        for (size_t i = 0; i < Lib_ArMemberCount(ar); i++) {
            Lib_ArMember *member = Lib_ArMemberAt(ar, i);
            if (Link_x86_64_MemberNeeded(out, member)) {
                Link_x86_64_MergeMember(out, path, member, opts);
                pulled = true;
            }
        }
    }
}

// Read archive bytes, refusing an archive that is malformed.
Lib_Ar *Link_x86_64_ReadArchive(const char *path, const uint8_t *bytes, size_t len)
{
    Lib_ArStatus status = LIB_AR_STATUS_OK;
    Lib_Ar *ar = Lib_ArReadMem(bytes, len, &status);
    Err_Assert(status != LIB_AR_STATUS_MEMBER_TRUNCATED, ERR_LINK_MEMBER_TRUNCATED, path);
    Err_Assert(status != LIB_AR_STATUS_HEADER_MALFORMED, ERR_LINK_HEADER_MALFORMED, path);
    Err_Assert(status != LIB_AR_STATUS_NAME_NOT_FOUND, ERR_LINK_NAME_NOT_FOUND, path);
    return ar;
}

// Merge each object into out, and of each archive the members out needs.
void Link_x86_64_MergeFiles(Elf *out, const char *const *paths, size_t npaths, const Link_x86_64_Options *opts)
{
    for (size_t i = 0; i < npaths; i++) {
        size_t len = 0;
        uint8_t *bytes = Elf_ReadBytes(paths[i], &len);
        Err_Assert(bytes, ERR_LINK_INPUT_NOT_READABLE, paths[i], strerror(errno));
        if (opts->lo_trace >= LINK_X86_64_TRACE_FILES) {
            printf("%s\n", paths[i]);
        }

        if (Lib_ArReadMagic(bytes, len)) {
            Lib_Ar *ar = Link_x86_64_ReadArchive(paths[i], bytes, len);
            Link_x86_64_MergeArchive(out, paths[i], ar, opts);
            Lib_ArFree(ar);
        } else {
            Elf *in = Elf_ReadMem(bytes, len);
            Err_Assert(in, ERR_LINK_OBJECT_NOT_READABLE, paths[i]);
            Link_x86_64_Merge(out, in);
            Elf_Free(in);
        }
        free(bytes);
    }
}

// Record a -place request, growing the list to hold it.
void Link_x86_64_AddPlace(Link_x86_64_Options *opts, const char *name, uint64_t addr)
{
    opts->lo_places = realloc(opts->lo_places, (opts->lo_nplaces + 1) * sizeof(*opts->lo_places));
    opts->lo_places[opts->lo_nplaces].lp_name = name;
    opts->lo_places[opts->lo_nplaces].lp_addr = addr;
    opts->lo_nplaces++;
}

// Load address requested for a section by name.
uint64_t Link_x86_64_PlacedAddr(const Link_x86_64_Options *opts, const char *name, bool *placed)
{
    for (size_t i = 0; i < opts->lo_nplaces; i++) {
        if (Str_Equals(opts->lo_places[i].lp_name, name)) {
            *placed = true;
            return opts->lo_places[i].lp_addr;
        }
    }
    *placed = false;
    return 0;
}

// Assign each allocatable section its -place address, else the next free page.
void Link_x86_64_PlaceSections(Elf *elf, const Link_x86_64_Options *opts)
{
    uint64_t next = ELF_BASE + ELF_PAGE;
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        Elf_Sec *sec = Elf_SectionAt(elf, i);
        if (! (sec->sec_flags & ELF_SHF_ALLOC)) {
            continue;
        }
        bool placed;
        uint64_t addr = Link_x86_64_PlacedAddr(opts, sec->sec_name, &placed);
        if (! placed) {
            addr = next;
        }
        Elf_SectionAddr(sec, addr);
        uint64_t end = addr + sec->sec_data.eb_len;
        if (end > next) {
            next = (end + ELF_PAGE - 1) / ELF_PAGE * ELF_PAGE;
        }
    }
}

// Abort if any relocation references a symbol that was never defined.
void Link_x86_64_CheckDefined(Elf *elf)
{
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        Elf_Sec *sec = Elf_SectionAt(elf, i);
        for (size_t r = 0; r < Elf_RelaCount(sec); r++) {
            Elf_Sym *sym = Elf_RelaAt(sec, r)->rel_sym;
            Err_Assert(sym && sym->sym_sec, ERR_LINK_SYMBOL_NOT_DEFINED, sym ? sym->sym_name : "?");
        }
    }
}

// Finalize an in-memory object into a static executable.
void Link_x86_64_Exec(Elf *elf, const Link_x86_64_Options *opts)
{
    const char *entry = opts->lo_entry ? opts->lo_entry : "_start";

    Link_x86_64_PlaceSections(elf, opts);
    Link_x86_64_CheckDefined(elf);

    Elf_Sym *sym = Elf_SymbolFind(elf, entry);
    Err_Assert(sym && sym->sym_sec, ERR_LINK_ENTRY_NOT_DEFINED, entry);
    Elf_SetEntry(elf, sym->sym_sec->sec_addr + sym->sym_value);

    Link_x86_64_RelApply(elf);
    Elf_SetType(elf, ELF_ET_EXEC);
}

// Read and link the given objects and archives into one Elf.
Elf *Link_x86_64_Run(const char *const *paths, size_t npaths, const Link_x86_64_Options *opts)
{
    Elf *out = Elf_New(ELF_ET_REL, ELF_EM_X86_64);
    Link_x86_64_MergeFiles(out, paths, npaths, opts);

    if (! opts->lo_relocatable) {
        Link_x86_64_Exec(out, opts);
    }
    return out;
}
