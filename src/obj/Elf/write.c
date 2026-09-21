#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "obj/Elf/buf.h"
#include "obj/Elf/write.h"

// Append name and a NUL to a string table, returning name's start offset.
uint32_t Elf_Write_Str(Elf_Buffer *strtab, const char *name)
{
    uint32_t off = (uint32_t) strtab->eb_len;
    Elf_Buffer_Data(strtab, name, strlen(name) + 1);
    return off;
}

// Position of a section within the object, used to fill in section indices.
uint32_t Elf_Write_SectionIndex(const Elf *elf, const Elf_Sec *sec, const uint32_t *secidx)
{
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        if (elf->elf_secs[i] == sec) {
            return secidx[i];
        }
    }
    return 0;
}

// Build the .symtab and .strtab bodies, locals before globals, recording indices in slot[].
void Elf_Write_Symtab(const Elf *elf, const uint32_t *secidx, Elf_Buffer *symtab, Elf_Buffer *strtab, uint32_t *slot, uint32_t *first_global)
{
    Elf64_Sym null = {0};
    Elf_Buffer_Data(symtab, &null, sizeof(null));
    Elf_Buffer_Byte(strtab, 0);

    uint32_t si = 1;
    for (int pass = 0; pass < 2; pass++) {
        uint8_t want = pass == 0 ? ELF_BIND_LOCAL : ELF_BIND_GLOBAL;
        if (pass == 1) {
            *first_global = si;
        }
        for (size_t i = 0; i < elf->elf_nsyms; i++) {
            Elf_Sym *sym = elf->elf_syms[i];
            uint8_t bind = sym->sym_bind == ELF_BIND_LOCAL ? ELF_BIND_LOCAL
                                                           : ELF_BIND_GLOBAL;
            if (bind != want) {
                continue;
            }
            slot[i] = si++;
            Elf64_Sym out = {
                .st_name  = Elf_Write_Str(strtab, sym->sym_name),
                .st_info  = ELF_ST_INFO(sym->sym_bind, sym->sym_type),
                .st_other = sym->sym_other,
                .st_size  = sym->sym_size
            };
            if (sym->sym_sec) {
                out.st_shndx = Elf_Write_SectionIndex(elf, sym->sym_sec, secidx);
                out.st_value = sym->sym_value;
            } else {
                out.st_shndx = ELF_SHN_UNDEF;
            }
            Elf_Buffer_Data(symtab, &out, sizeof(out));
        }
    }
}

// Build one .rela.* body from a section's relocations, using final indices.
void Elf_Write_Relas(const Elf_Sec *sec, const uint32_t *slot, const Elf *elf, Elf_Buffer *out)
{
    for (size_t r = 0; r < sec->sec_nrelas; r++) {
        uint32_t symi = 0;
        Elf_Rela *rel = &sec->sec_relas[r];
        for (size_t i = 0; i < elf->elf_nsyms; i++) {
            if (elf->elf_syms[i] == rel->rel_sym) {
                symi = slot[i];
                break;
            }
        }
        Elf64_Rela disk = {
            .r_offset = rel->rel_offset,
            .r_info   = ELF_R_INFO(symi, rel->rel_type),
            .r_addend = rel->rel_addend
        };
        Elf_Buffer_Data(out, &disk, sizeof(disk));
    }
}

// Serialize a relocatable object (ET_REL): sections, .symtab/.strtab, .rela.* and .shstrtab.
int Elf_Write_Rel(const Elf *elf, FILE *out)
{
    size_t nuser = elf->elf_nsecs;

    // Phase: assign a section-header index to every output section.
    uint32_t *secidx = calloc(nuser ? nuser : 1, sizeof(*secidx));
    uint32_t idx = 1;
    for (size_t i = 0; i < nuser; i++) {
        secidx[i] = idx++;
    }
    uint32_t idx_symtab = idx++;
    uint32_t idx_strtab = idx++;
    uint32_t *relaidx = calloc(nuser ? nuser : 1, sizeof(*relaidx));
    for (size_t i = 0; i < nuser; i++) {
        if (elf->elf_secs[i]->sec_nrelas) {
            relaidx[i] = idx++;
        }
    }
    uint32_t idx_shstrtab = idx++;
    uint32_t shnum = idx;

    // Phase: build the symbol table and per-section relocation bodies.
    uint32_t first_global = 1;
    Elf_Buffer symtab, strtab, shstr;
    uint32_t *slot = calloc(elf->elf_nsyms ? elf->elf_nsyms : 1, sizeof(*slot));
    Elf_Buffer_Init(&symtab);
    Elf_Buffer_Init(&strtab);
    Elf_Buffer_Init(&shstr);
    Elf_Buffer_Byte(&shstr, 0);
    Elf_Write_Symtab(elf, secidx, &symtab, &strtab, slot, &first_global);

    Elf_Buffer *relas = calloc(nuser ? nuser : 1, sizeof(*relas));
    for (size_t i = 0; i < nuser; i++) {
        Elf_Buffer_Init(&relas[i]);
        if (relaidx[i]) {
            Elf_Write_Relas(elf->elf_secs[i], slot, elf, &relas[i]);
        }
    }

    // Phase: lay out section headers and their bodies.
    Elf64_Shdr *shdrs = calloc(shnum, sizeof(*shdrs));
    uint64_t *sizes = calloc(shnum, sizeof(*sizes));
    const void **bodies = calloc(shnum, sizeof(*bodies));

    for (size_t i = 0; i < nuser; i++) {
        Elf_Sec *sec = elf->elf_secs[i];
        shdrs[secidx[i]] = (Elf64_Shdr) {
            .sh_name      = Elf_Write_Str(&shstr, sec->sec_name),
            .sh_type      = sec->sec_type,
            .sh_flags     = sec->sec_flags,
            .sh_size      = sec->sec_data.eb_len,
            .sh_addralign = sec->sec_addralign,
            .sh_entsize   = sec->sec_entsize
        };
        bodies[secidx[i]] = sec->sec_data.eb_data;
        sizes[secidx[i]]  = sec->sec_data.eb_len;
    }

    shdrs[idx_symtab] = (Elf64_Shdr) {
        .sh_name      = Elf_Write_Str(&shstr, ".symtab"),
        .sh_type      = ELF_SHT_SYMTAB,
        .sh_size      = symtab.eb_len,
        .sh_link      = idx_strtab,
        .sh_info      = first_global,
        .sh_addralign = 8,
        .sh_entsize   = sizeof(Elf64_Sym)
    };
    bodies[idx_symtab] = symtab.eb_data;
    sizes[idx_symtab]  = symtab.eb_len;

    shdrs[idx_strtab] = (Elf64_Shdr) {
        .sh_name      = Elf_Write_Str(&shstr, ".strtab"),
        .sh_type      = ELF_SHT_STRTAB,
        .sh_size      = strtab.eb_len,
        .sh_addralign = 1
    };
    bodies[idx_strtab] = strtab.eb_data;
    sizes[idx_strtab]  = strtab.eb_len;

    for (size_t i = 0; i < nuser; i++) {
        if (! relaidx[i]) {
            continue;
        }
        char name[64];
        snprintf(name, sizeof(name), ".rela%s", elf->elf_secs[i]->sec_name);
        shdrs[relaidx[i]] = (Elf64_Shdr) {
            .sh_name      = Elf_Write_Str(&shstr, name),
            .sh_type      = ELF_SHT_RELA,
            .sh_size      = relas[i].eb_len,
            .sh_link      = idx_symtab,
            .sh_info      = secidx[i],
            .sh_addralign = 8,
            .sh_entsize   = sizeof(Elf64_Rela)
        };
        bodies[relaidx[i]] = relas[i].eb_data;
        sizes[relaidx[i]]  = relas[i].eb_len;
    }

    shdrs[idx_shstrtab] = (Elf64_Shdr) {
        .sh_name      = Elf_Write_Str(&shstr, ".shstrtab"),
        .sh_type      = ELF_SHT_STRTAB,
        .sh_addralign = 1
    };
    bodies[idx_shstrtab] = shstr.eb_data;
    sizes[idx_shstrtab]  = shstr.eb_len;
    shdrs[idx_shstrtab].sh_size = shstr.eb_len;

    // Phase: assign file offsets, then write header, bodies and shdr table.
    uint64_t off = sizeof(Elf64_Ehdr);
    for (uint32_t i = 1; i < shnum; i++) {
        uint64_t align = shdrs[i].sh_addralign ? shdrs[i].sh_addralign : 1;
        off = (off + align - 1) / align * align;
        shdrs[i].sh_offset = off;
        off += sizes[i];
    }
    off = (off + 7) / 8 * 8;
    uint64_t shoff = off;

    Elf64_Ehdr ehdr = {
        .e_ident     = { 0x7F, 'E', 'L', 'F', ELF_CLASS64, ELF_DATA2LSB, ELF_VERSION },
        .e_type      = ELF_ET_REL,
        .e_machine   = elf->elf_machine,
        .e_version   = ELF_VERSION,
        .e_shoff     = shoff,
        .e_ehsize    = sizeof(Elf64_Ehdr),
        .e_shentsize = sizeof(Elf64_Shdr),
        .e_shnum     = (uint16_t) shnum,
        .e_shstrndx  = (uint16_t) idx_shstrtab
    };

    long pos = 0;
    fwrite(&ehdr, sizeof(ehdr), 1, out);
    pos += sizeof(ehdr);
    for (uint32_t i = 1; i < shnum; i++) {
        while (pos < (long) shdrs[i].sh_offset) {
            fputc(0, out);
            pos++;
        }
        if (sizes[i]) {
            fwrite(bodies[i], 1, sizes[i], out);
        }
        pos += sizes[i];
    }
    while (pos < (long) shoff) {
        fputc(0, out);
        pos++;
    }
    fwrite(shdrs, sizeof(Elf64_Shdr), shnum, out);

    Elf_Buffer_Free(&symtab);
    Elf_Buffer_Free(&strtab);
    Elf_Buffer_Free(&shstr);
    for (size_t i = 0; i < nuser; i++) {
        Elf_Buffer_Free(&relas[i]);
    }
    free(relas);
    free(secidx);
    free(relaidx);
    free(slot);
    free(shdrs);
    free(bodies);
    free(sizes);
    return 0;
}

// Smallest file offset >= pos that is page-congruent with vaddr, as PT_LOAD requires.
uint64_t Elf_Write_PlaceOffset(uint64_t pos, uint64_t vaddr)
{
    return pos + (vaddr - pos) % ELF_PAGE;
}

// Serialize a static executable (ET_EXEC): one R+X PT_LOAD per placed section.
int Elf_Write_Exec(const Elf *elf, FILE *out)
{
    // Phase: select the loadable sections.
    Elf_Sec **segs = calloc(elf->elf_nsecs ? elf->elf_nsecs : 1, sizeof(*segs));
    uint64_t *offs = calloc(elf->elf_nsecs ? elf->elf_nsecs : 1, sizeof(*offs));
    int nseg = 0;
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        Elf_Sec *sec = elf->elf_secs[i];
        if ((sec->sec_flags & ELF_SHF_ALLOC) && sec->sec_data.eb_len) {
            segs[nseg++] = sec;
        }
    }

    // Phase: assign page-congruent file offsets.
    uint64_t pos = sizeof(Elf64_Ehdr) + (uint64_t) nseg * sizeof(Elf64_Phdr);
    for (int i = 0; i < nseg; i++) {
        offs[i] = Elf_Write_PlaceOffset(pos, segs[i]->sec_addr);
        pos = offs[i] + segs[i]->sec_data.eb_len;
    }

    Elf64_Ehdr ehdr = {
        .e_ident     = { 0x7F, 'E', 'L', 'F', ELF_CLASS64, ELF_DATA2LSB, ELF_VERSION },
        .e_type      = ELF_ET_EXEC,
        .e_machine   = elf->elf_machine,
        .e_version   = ELF_VERSION,
        .e_entry     = elf->elf_entry,
        .e_phoff     = sizeof(Elf64_Ehdr),
        .e_ehsize    = sizeof(Elf64_Ehdr),
        .e_phentsize = sizeof(Elf64_Phdr),
        .e_phnum     = (uint16_t) nseg
    };
    fwrite(&ehdr, sizeof(ehdr), 1, out);
    for (int i = 0; i < nseg; i++) {
        Elf64_Phdr phdr = {
            .p_type   = ELF_PT_LOAD,
            .p_flags  = ELF_PF_R | ELF_PF_X,
            .p_offset = offs[i],
            .p_vaddr  = segs[i]->sec_addr,
            .p_paddr  = segs[i]->sec_addr,
            .p_filesz = segs[i]->sec_data.eb_len,
            .p_memsz  = segs[i]->sec_data.eb_len,
            .p_align  = ELF_PAGE
        };
        fwrite(&phdr, sizeof(phdr), 1, out);
    }
    long pos2 = sizeof(Elf64_Ehdr) + (long) nseg * sizeof(Elf64_Phdr);
    for (int i = 0; i < nseg; i++) {
        while (pos2 < (long) offs[i]) {
            fputc(0, out);
            pos2++;
        }
        fwrite(segs[i]->sec_data.eb_data, 1, segs[i]->sec_data.eb_len, out);
        pos2 += segs[i]->sec_data.eb_len;
    }

    free(segs);
    free(offs);
    return 0;
}

// Serialize an object to an open stream, returning 0 on success or -1.
int Elf_Write_File(const Elf *elf, FILE *out)
{
    if (elf->elf_type == ELF_ET_EXEC) {
        return Elf_Write_Exec(elf, out);
    }
    return Elf_Write_Rel(elf, out);
}

// Serialize an object to a file, returning 0 on success or -1 on error.
int Elf_Write_Path(const Elf *elf, const char *path)
{
    FILE *out = fopen(path, "wb");
    if (! out) {
        return -1;
    }
    int rc = Elf_Write_File(elf, out);
    fclose(out);
    return rc;
}
