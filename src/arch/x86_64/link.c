// C source file for the x86-64 static linker.

// Take every include from the module's header.
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
    uint8_t *at = Elf_Buffer_At(Elf_Section_Data(sec), offset);
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
            Err_Raise(ERR_LINK_UNSUPPORTED_RELOCATION, rel->rel_type);
        }
    }
}

// Apply every relocation in a placed object, patching each section's bytes.
void Link_x86_64_RelApply(Elf *elf)
{
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        Elf_Sec *sec = Elf_Section_At(elf, i);
        for (size_t r = 0; r < Elf_Rela_Count(sec); r++) {
            Link_x86_64_RelApplyOne(sec, Elf_Rela_At(sec, r));
        }
    }
}

// Index of a section within an object.
int64_t Link_x86_64_SectionIndex(const Elf *elf, const Elf_Sec *target)
{
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        if (Elf_Section_At(elf, i) == target) {
            return (int64_t) i;
        }
    }
    return -1;
}

// Index of a symbol within an object.
int64_t Link_x86_64_SymbolIndex(const Elf *elf, const Elf_Sym *target)
{
    for (size_t i = 0; i < Elf_Symbol_Count(elf); i++) {
        if (Elf_Symbol_At(elf, i) == target) {
            return (int64_t) i;
        }
    }
    return -1;
}

// Find an existing global symbol by name.
Elf_Sym *Link_x86_64_FindGlobal(Elf *elf, const char *name)
{
    for (size_t i = 0; i < Elf_Symbol_Count(elf); i++) {
        Elf_Sym *sym = Elf_Symbol_At(elf, i);
        if (sym->sym_bind != ELF_BIND_LOCAL && strcmp(sym->sym_name, name) == 0) {
            return sym;
        }
    }
    return NULL;
}

// Merge one input object into the output.
void Link_x86_64_Merge(Elf *out, Elf *in)
{
    size_t nsec = Elf_Section_Count(in);
    size_t nsym = Elf_Symbol_Count(in);

    Elf_Sec **secmap = calloc(nsec ? nsec : 1, sizeof(*secmap));
    Elf_Sym **symmap = calloc(nsym ? nsym : 1, sizeof(*symmap));
    uint64_t *secbase = calloc(nsec ? nsec : 1, sizeof(*secbase));

    // Phase: merge section bytes, recording each input section's new base.
    for (size_t i = 0; i < nsec; i++) {
        Elf_Sec *sec = Elf_Section_At(in, i);
        Elf_Sec *dst = Elf_Section_Get(out, sec->sec_name, sec->sec_type, sec->sec_flags);
        Elf_Buffer *db = Elf_Section_Data(dst);
        if (sec->sec_addralign > dst->sec_addralign) {
            dst->sec_addralign = sec->sec_addralign;
        }
        Elf_Buffer_Align(db, sec->sec_addralign);
        secbase[i] = db->eb_len;
        Elf_Buffer_Data(db, sec->sec_data.eb_data, sec->sec_data.eb_len);
        secmap[i] = dst;
    }

    // Phase: copy symbols, unifying globals and resolving undefined references.
    for (size_t i = 0; i < nsym; i++) {
        Elf_Sym *sym = Elf_Symbol_At(in, i);
        Elf_Sec *dsec = NULL;
        uint64_t value = 0;
        if (sym->sym_sec) {
            int64_t j = Link_x86_64_SectionIndex(in, sym->sym_sec);
            dsec  = secmap[j];
            value = secbase[j] + sym->sym_value;
        }

        if (sym->sym_bind == ELF_BIND_LOCAL) {
            symmap[i] = Elf_Symbol_Add(out, sym->sym_name, dsec, value, ELF_BIND_LOCAL, sym->sym_type);
            continue;
        }

        Elf_Sym *existing = Link_x86_64_FindGlobal(out, sym->sym_name);
        if (! existing) {
            symmap[i] = Elf_Symbol_Add(out, sym->sym_name, dsec, value, sym->sym_bind, sym->sym_type);
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
        Elf_Sec *sec = Elf_Section_At(in, i);
        for (size_t r = 0; r < Elf_Rela_Count(sec); r++) {
            Elf_Rela *rel = Elf_Rela_At(sec, r);
            int64_t k = Link_x86_64_SymbolIndex(in, rel->rel_sym);
            if (k < 0) {
                continue;
            }
            Elf_Rela_Add(secmap[i], secbase[i] + rel->rel_offset, symmap[k], rel->rel_type, rel->rel_addend);
        }
    }

    free(secmap);
    free(secbase);
    free(symmap);
}

// Read each object file and merge it into out.
void Link_x86_64_MergeFiles(Elf *out, const char *const *paths, size_t npaths)
{
    for (size_t i = 0; i < npaths; i++) {
        Elf *in = Elf_Read_Path(paths[i]);
        Err_Assert(in, ERR_LINK_OBJECT_UNREADABLE, paths[i]);
        Link_x86_64_Merge(out, in);
        Elf_Free(in);
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
        if (strcmp(opts->lo_places[i].lp_name, name) == 0) {
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
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        Elf_Sec *sec = Elf_Section_At(elf, i);
        if (! (sec->sec_flags & ELF_SHF_ALLOC)) {
            continue;
        }
        bool placed;
        uint64_t addr = Link_x86_64_PlacedAddr(opts, sec->sec_name, &placed);
        if (! placed) {
            addr = next;
        }
        Elf_Section_Addr(sec, addr);
        uint64_t end = addr + sec->sec_data.eb_len;
        if (end > next) {
            next = (end + ELF_PAGE - 1) / ELF_PAGE * ELF_PAGE;
        }
    }
}

// Abort if any relocation references a symbol that was never defined.
void Link_x86_64_CheckDefined(Elf *elf)
{
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        Elf_Sec *sec = Elf_Section_At(elf, i);
        for (size_t r = 0; r < Elf_Rela_Count(sec); r++) {
            Elf_Sym *sym = Elf_Rela_At(sec, r)->rel_sym;
            Err_Assert(sym && sym->sym_sec, ERR_LINK_UNDEFINED_SYMBOL, sym ? sym->sym_name : "?");
        }
    }
}

// Finalize an in-memory object into a static executable.
void Link_x86_64_Exec(Elf *elf, const Link_x86_64_Options *opts)
{
    const char *entry = opts->lo_entry ? opts->lo_entry : "_start";

    Link_x86_64_PlaceSections(elf, opts);
    Link_x86_64_CheckDefined(elf);

    Elf_Sym *sym = Elf_Symbol_Find(elf, entry);
    Err_Assert(sym && sym->sym_sec, ERR_LINK_UNDEFINED_ENTRY, entry);
    Elf_SetEntry(elf, sym->sym_sec->sec_addr + sym->sym_value);

    Link_x86_64_RelApply(elf);
    Elf_SetType(elf, ELF_ET_EXEC);
}

// Read and link the given objects into one Elf.
Elf *Link_x86_64_Run(const char *const *paths, size_t npaths, const Link_x86_64_Options *opts)
{
    Elf *out = Elf_New(ELF_ET_REL, ELF_EM_X86_64);
    Link_x86_64_MergeFiles(out, paths, npaths);

    if (! opts->lo_relocatable) {
        Link_x86_64_Exec(out, opts);
    }
    return out;
}
