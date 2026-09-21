#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util/log.h"
#include "obj/Elf/buf.h"
#include "obj/Elf/elf.h"
#include "obj/Elf/sec.h"
#include "obj/Elf/sym.h"
#include "obj/Elf/rela.h"
#include "obj/Elf/read.h"
#include "obj/Elf/link.h"
#include "arch/x86_64/rel.h"

// Virtual address an executable is loaded at, and the page segments align to.
#define LINK_BASE 0x400000
#define LINK_PAGE 0x1000

// Index of a section within an object, or -1 if it holds none.
long Elf_Link_SectionIndex(const Elf *elf, const Elf_Sec *target)
{
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        if (Elf_Section_At(elf, i) == target) {
            return (long) i;
        }
    }
    return -1;
}

// Index of a symbol within an object, or -1 if it holds none.
long Elf_Link_SymbolIndex(const Elf *elf, const Elf_Sym *target)
{
    for (size_t i = 0; i < Elf_Symbol_Count(elf); i++) {
        if (Elf_Symbol_At(elf, i) == target) {
            return (long) i;
        }
    }
    return -1;
}

// Find an existing global symbol by name, or return NULL.
Elf_Sym *Elf_Link_FindGlobal(Elf *elf, const char *name)
{
    for (size_t i = 0; i < Elf_Symbol_Count(elf); i++) {
        Elf_Sym *sym = Elf_Symbol_At(elf, i);
        if (sym->sym_bind != ELF_BIND_LOCAL && strcmp(sym->sym_name, name) == 0) {
            return sym;
        }
    }
    return NULL;
}

// Merge one input object into the output, unifying globals and rebasing relocations.
void Elf_Link_Merge(Elf *out, Elf *in)
{
    size_t nsec = Elf_Section_Count(in);
    size_t nsym = Elf_Symbol_Count(in);

    Elf_Sec **secmap = calloc(nsec ? nsec : 1, sizeof(*secmap));
    Elf_Sym **symmap = calloc(nsym ? nsym : 1, sizeof(*symmap));
    uint64_t *secbase = calloc(nsec ? nsec : 1, sizeof(*secbase));

    // Phase: merge section bytes, recording each input section's new base.
    for (size_t i = 0; i < nsec; i++) {
        Elf_Sec *sec   = Elf_Section_At(in, i);
        Elf_Sec *dst = Elf_Section_Get(out, sec->sec_name, sec->sec_type, sec->sec_flags);
        Elf_Buffer *db  = Elf_Section_Data(dst);
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
        Elf_Sym *sym   = Elf_Symbol_At(in, i);
        Elf_Sec *dsec  = NULL;
        uint64_t value = 0;
        if (sym->sym_sec) {
            long j = Elf_Link_SectionIndex(in, sym->sym_sec);
            dsec  = secmap[j];
            value = secbase[j] + sym->sym_value;
        }

        if (sym->sym_bind == ELF_BIND_LOCAL) {
            symmap[i] = Elf_Symbol_Add(out, sym->sym_name, dsec, value, ELF_BIND_LOCAL, sym->sym_type);
            continue;
        }

        Elf_Sym *existing = Elf_Link_FindGlobal(out, sym->sym_name);
        if (! existing) {
            symmap[i] = Elf_Symbol_Add(out, sym->sym_name, dsec, value, sym->sym_bind, sym->sym_type);
            continue;
        }
        if (dsec) {
            if (existing->sym_sec) {
                Log_ShowError("multiple definition of '%s'", sym->sym_name);
            }
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
            long k = Elf_Link_SymbolIndex(in, rel->rel_sym);
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
void Elf_Link_MergeFiles(Elf *out, const char *const *paths, int npaths)
{
    for (int i = 0; i < npaths; i++) {
        Elf *in = Elf_Read_Path(paths[i]);
        if (! in) {
            Log_ShowError("cannot read object '%s'", paths[i]);
        }
        Elf_Link_Merge(out, in);
        Elf_Free(in);
    }
}

// Record a -place request, growing the list to hold it.
void Elf_Link_AddPlace(Elf_LinkOptions *opts, const char *name, uint64_t addr)
{
    opts->lo_places = realloc(opts->lo_places, (opts->lo_nplaces + 1) * sizeof(*opts->lo_places));
    opts->lo_places[opts->lo_nplaces].lp_name = name;
    opts->lo_places[opts->lo_nplaces].lp_addr = addr;
    opts->lo_nplaces++;
}

// Load address requested for a section by name, or 0 if it is unplaced.
uint64_t Elf_Link_PlacedAddr(const Elf_LinkOptions *opts, const char *name, int *placed)
{
    for (int i = 0; i < opts->lo_nplaces; i++) {
        if (strcmp(opts->lo_places[i].lp_name, name) == 0) {
            *placed = 1;
            return opts->lo_places[i].lp_addr;
        }
    }
    *placed = 0;
    return 0;
}

// Assign each allocatable section its -place address, else the next free page.
void Elf_Link_PlaceSections(Elf *elf, const Elf_LinkOptions *opts)
{
    uint64_t next = LINK_BASE + LINK_PAGE;
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        Elf_Sec *sec = Elf_Section_At(elf, i);
        if (! (sec->sec_flags & ELF_SHF_ALLOC)) {
            continue;
        }
        int placed;
        uint64_t addr = Elf_Link_PlacedAddr(opts, sec->sec_name, &placed);
        if (! placed) {
            addr = next;
        }
        Elf_Section_Addr(sec, addr);
        uint64_t end = addr + sec->sec_data.eb_len;
        if (end > next) {
            next = (end + LINK_PAGE - 1) / LINK_PAGE * LINK_PAGE;
        }
    }
}

// Abort if any relocation references a symbol that was never defined.
void Elf_Link_CheckDefined(Elf *elf)
{
    for (size_t i = 0; i < Elf_Section_Count(elf); i++) {
        Elf_Sec *sec = Elf_Section_At(elf, i);
        for (size_t r = 0; r < Elf_Rela_Count(sec); r++) {
            Elf_Sym *sym = Elf_Rela_At(sec, r)->rel_sym;
            if (! sym || ! sym->sym_sec) {
                Log_ShowError("undefined symbol '%s'", sym ? sym->sym_name : "?");
            }
        }
    }
}

// Finalize an in-memory object into a static executable.
void Elf_Link_Exec(Elf *elf, const Elf_LinkOptions *opts)
{
    const char *entry = opts->lo_entry ? opts->lo_entry : "_start";

    Elf_Link_PlaceSections(elf, opts);
    Elf_Link_CheckDefined(elf);

    Elf_Sym *sym = Elf_Symbol_Find(elf, entry);
    if (! sym || ! sym->sym_sec) {
        Log_ShowError("undefined entry symbol '%s'", entry);
    }
    Elf_SetEntry(elf, sym->sym_sec->sec_addr + sym->sym_value);

    Rel_x86_64_Apply(elf);
    Elf_SetType(elf, ELF_ET_EXEC);
}

// Read and link the given objects into one Elf.
Elf *Elf_Link_Run(const char *const *paths, int npaths, const Elf_LinkOptions *opts)
{
    Elf *out = Elf_New(ELF_ET_REL, ELF_EM_X86_64);
    Elf_Link_MergeFiles(out, paths, npaths);

    if (! opts->lo_relocatable) {
        Elf_Link_Exec(out, opts);
    }
    return out;
}
