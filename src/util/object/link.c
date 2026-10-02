/*
 * C source file for the static linker.
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
#include "util/object/link.h"

// Virtual address a symbol resolves to.
uint64_t Link_ElfSymbolAddr(const Elf_Sym *sym)
{
    if (sym->sym_sec) {
        return sym->sym_sec->sec_addr + sym->sym_value;
    }
    return sym->sym_value;
}

// Patch width little-endian bytes at a section offset with value.
void Link_ElfPatchLE(Elf_Sec *sec, uint64_t offset, uint64_t value, size_t width)
{
    uint8_t *at = Elf_BufferAt(Elf_SectionData(sec), offset);
    for (size_t i = 0; i < width; i++) {
        at[i] = (value >> (8 * i)) & 0xFF;
    }
}

// Apply one x86-64 relocation from S (symbol), A (addend) and P (site).
void Link_x86_64_ElfApplyRela(Elf_Sec *sec, const Elf_Rela *rela)
{
    uint64_t S = Link_ElfSymbolAddr(rela->rela_sym);
    int64_t A = rela->rela_addend;
    uint64_t P = sec->sec_addr + rela->rela_offset;

    switch (rela->rela_type) {
        case R_X86_64_PC32:
        case R_X86_64_PLT32: {
            Link_ElfPatchLE(sec, rela->rela_offset, (uint32_t) (int32_t) (S + A - P), 4);
        } break;
        case R_X86_64_32:
        case R_X86_64_32S: {
            Link_ElfPatchLE(sec, rela->rela_offset, (uint32_t) (S + A), 4);
        } break;
        case R_X86_64_64: {
            Link_ElfPatchLE(sec, rela->rela_offset, S + A, 8);
        } break;
        default: {
            Err_Raise(ERR_LINK_RELOCATION_NOT_SUPPORTED, rela->rela_type);
        }
    }
}

// Apply one relocation by the rules of the object's machine.
void Link_ElfApplyRela(const Elf *elf, Elf_Sec *sec, const Elf_Rela *rela)
{
    switch (elf->elf_machine) {
        case ELF_EM_X86_64: {
            Link_x86_64_ElfApplyRela(sec, rela);
        } break;
        default: {
            Err_Raise(ERR_LINK_MACHINE_NOT_SUPPORTED, (unsigned) elf->elf_machine);
        }
    }
}

// Apply every relocation in a placed object, patching each section's bytes.
void Link_ElfApplyRelas(Elf *elf)
{
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        Elf_Sec *sec = Elf_SectionAt(elf, i);
        for (size_t r = 0; r < Elf_RelaCount(sec); r++) {
            Link_ElfApplyRela(elf, sec, Elf_RelaAt(sec, r));
        }
    }
}

// Index of a section within an object.
int64_t Link_ElfSectionIndex(const Elf *elf, const Elf_Sec *target)
{
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        if (Elf_SectionAt(elf, i) == target) {
            return (int64_t) i;
        }
    }
    return -1;
}

// Index of a symbol within an object.
int64_t Link_ElfSymbolIndex(const Elf *elf, const Elf_Sym *target)
{
    for (size_t i = 0; i < Elf_SymbolCount(elf); i++) {
        if (Elf_SymbolAt(elf, i) == target) {
            return (int64_t) i;
        }
    }
    return -1;
}

// Find an existing global symbol by name.
Elf_Sym *Link_ElfFindGlobal(Elf *elf, const char *name)
{
    for (size_t i = 0; i < Elf_SymbolCount(elf); i++) {
        Elf_Sym *sym = Elf_SymbolAt(elf, i);
        if (sym->sym_bind != ELF_BIND_LOCAL && Str_Equals(sym->sym_name, name)) {
            return sym;
        }
    }
    return NULL;
}

// Merge one input object, named name, into the output.
void Link_ElfMerge(Elf *out, Elf *in, const char *name)
{
    Err_Assert(in->elf_machine == out->elf_machine, ERR_LINK_MACHINE_MISMATCH, name, (unsigned) in->elf_machine, (unsigned) out->elf_machine);
    Err_Assert(in->elf_type == ELF_ET_REL, ERR_LINK_OBJECT_NOT_RELOCATABLE, name, (unsigned) in->elf_type);

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
            int64_t j = Link_ElfSectionIndex(in, sym->sym_sec);
            dsec  = secmap[j];
            value = secbase[j] + sym->sym_value;
        }

        if (sym->sym_bind == ELF_BIND_LOCAL) {
            symmap[i] = Elf_SymbolAdd(out, sym->sym_name, dsec, value, ELF_BIND_LOCAL, sym->sym_type);
            continue;
        }

        Elf_Sym *existing = Link_ElfFindGlobal(out, sym->sym_name);
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
            Elf_Rela *rela = Elf_RelaAt(sec, r);
            int64_t k = Link_ElfSymbolIndex(in, rela->rela_sym);
            if (k < 0) {
                continue;
            }
            Elf_RelaAdd(secmap[i], secbase[i] + rela->rela_offset, symmap[k], rela->rela_type, rela->rela_addend);
        }
    }

    free(secmap);
    free(secbase);
    free(symmap);
}

// Read archive bytes, refusing an archive that is malformed.
Lib_Ar *Link_ArRead(const char *path, const uint8_t *bytes, size_t len)
{
    Lib_ArStatus status = LIB_AR_STATUS_OK;
    Lib_Ar *ar = Lib_ArReadMem(bytes, len, &status);
    Err_Assert(status != LIB_AR_STATUS_MEMBER_TRUNCATED, ERR_LINK_MEMBER_TRUNCATED, path);
    Err_Assert(status != LIB_AR_STATUS_HEADER_MALFORMED, ERR_LINK_HEADER_MALFORMED, path);
    Err_Assert(status != LIB_AR_STATUS_NAME_NOT_FOUND, ERR_LINK_NAME_NOT_FOUND, path);
    return ar;
}

// True if a member defines a symbol out references but has not defined.
bool Link_ArMemberNeeded(Elf *out, const Lib_ArMember *member)
{
    for (const char **iter = member->lam_globals; *iter; iter++) {
        Elf_Sym *sym = Link_ElfFindGlobal(out, *iter);
        if (sym && ! sym->sym_sec) {
            return true;
        }
    }
    return false;
}

// Read one archive member as an object and merge it into out.
void Link_ArMergeMember(Elf *out, const char *path, const Lib_ArMember *member, const Link_Options *opts)
{
    Elf *in = Elf_ReadMem(member->lam_data, member->lam_size);
    Err_Assert(in, ERR_LINK_MEMBER_NOT_READABLE, member->lam_name, path);
    if (opts->lo_trace >= LINK_TRACE_MEMBERS) {
        printf("(%s)%s\n", path, member->lam_name);
    }
    Link_ElfMerge(out, in, member->lam_name);
    Elf_Free(in);
}

// Merge each member out needs, scanning again until a pass pulls in none.
void Link_ArMerge(Elf *out, const char *path, const Lib_Ar *ar, const Link_Options *opts)
{
    bool pulled = true;
    while (pulled) {
        pulled = false;
        for (size_t i = 0; i < Lib_ArMemberCount(ar); i++) {
            Lib_ArMember *member = Lib_ArMemberAt(ar, i);
            if (Link_ArMemberNeeded(out, member)) {
                Link_ArMergeMember(out, path, member, opts);
                pulled = true;
            }
        }
    }
}

// Record a -place request, growing the list to hold it.
void Link_PlaceAdd(Link_Options *opts, const char *name, uint64_t addr)
{
    opts->lo_places = realloc(opts->lo_places, (opts->lo_nplaces + 1) * sizeof(*opts->lo_places));
    opts->lo_places[opts->lo_nplaces].lp_name = name;
    opts->lo_places[opts->lo_nplaces].lp_addr = addr;
    opts->lo_nplaces++;
}

// Load address requested for a section by name.
uint64_t Link_PlaceAddr(const Link_Options *opts, const char *name, bool *placed)
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
void Link_PlaceSections(Elf *elf, const Link_Options *opts)
{
    uint64_t next = ELF_BASE + ELF_PAGE;
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        Elf_Sec *sec = Elf_SectionAt(elf, i);
        if (! (sec->sec_flags & ELF_SHF_ALLOC)) {
            continue;
        }
        bool placed;
        uint64_t addr = Link_PlaceAddr(opts, sec->sec_name, &placed);
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
void Link_ExecCheckDefined(Elf *elf)
{
    for (size_t i = 0; i < Elf_SectionCount(elf); i++) {
        Elf_Sec *sec = Elf_SectionAt(elf, i);
        for (size_t r = 0; r < Elf_RelaCount(sec); r++) {
            Elf_Sym *sym = Elf_RelaAt(sec, r)->rela_sym;
            Err_Assert(sym && sym->sym_sec, ERR_LINK_SYMBOL_NOT_DEFINED, sym ? sym->sym_name : "?");
        }
    }
}

// Finalize an in-memory object into a static executable.
void Link_ExecFinalize(Elf *elf, const Link_Options *opts)
{
    const char *entry = opts->lo_entry ? opts->lo_entry : "_start";

    Link_PlaceSections(elf, opts);
    Link_ExecCheckDefined(elf);

    Elf_Sym *sym = Elf_SymbolFind(elf, entry);
    Err_Assert(sym && sym->sym_sec, ERR_LINK_ENTRY_NOT_DEFINED, entry);
    Elf_SetEntry(elf, sym->sym_sec->sec_addr + sym->sym_value);

    Link_ElfApplyRelas(elf);
    Elf_SetType(elf, ELF_ET_EXEC);
}

// Merge each object into out, and of each archive the members out needs.
void Link_MergeFiles(Elf *out, const char *const *paths, size_t npaths, const Link_Options *opts)
{
    for (size_t i = 0; i < npaths; i++) {
        size_t len = 0;
        uint8_t *bytes = Elf_ReadBytes(paths[i], &len);
        Err_Assert(bytes, ERR_LINK_INPUT_NOT_READABLE, paths[i], strerror(errno));
        if (opts->lo_trace >= LINK_TRACE_FILES) {
            printf("%s\n", paths[i]);
        }

        if (Lib_ArReadMagic(bytes, len)) {
            Lib_Ar *ar = Link_ArRead(paths[i], bytes, len);
            Link_ArMerge(out, paths[i], ar, opts);
            Lib_ArFree(ar);
        } else {
            Elf *in = Elf_ReadMem(bytes, len);
            Err_Assert(in, ERR_LINK_OBJECT_NOT_READABLE, paths[i]);
            Link_ElfMerge(out, in, paths[i]);
            Elf_Free(in);
        }
        free(bytes);
    }
}

// Read and link the given objects and archives into one Elf.
Elf *Link_Build(const char *const *paths, size_t npaths, const Link_Options *opts)
{
    Elf *out = Elf_New(ELF_ET_REL, ELF_EM_X86_64);
    Link_MergeFiles(out, paths, npaths, opts);

    if (! opts->lo_relocatable) {
        Link_ExecFinalize(out, opts);
    }
    return out;
}
