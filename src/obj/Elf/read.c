#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "obj/Elf/buf.h"
#include "obj/Elf/elf.h"
#include "obj/Elf/sec.h"
#include "obj/Elf/sym.h"
#include "obj/Elf/rela.h"
#include "obj/Elf/read.h"

// Validate the file header and return it, or NULL if it is not an ELF object.
const Elf64_Ehdr *Elf_Read_Ehdr(const uint8_t *data, size_t n)
{
    if (n < sizeof(Elf64_Ehdr)) {
        return NULL;
    }
    const Elf64_Ehdr *eh = (const Elf64_Ehdr *) data;
    if (eh->e_ident[0] != 0x7F || eh->e_ident[1] != 'E' ||
        eh->e_ident[2] != 'L' || eh->e_ident[3] != 'F') {
        return NULL;
    }
    return eh;
}

// Parse ELF bytes into a new object, or return NULL on error.
Elf *Elf_Read_Mem(const void *buf, size_t n)
{
    const uint8_t    *data = buf;
    const Elf64_Ehdr *eh   = Elf_Read_Ehdr(data, n);
    if (! eh) {
        return NULL;
    }
    int shnum = eh->e_shnum;
    const Elf64_Shdr *sh = (const Elf64_Shdr *) (data + eh->e_shoff);
    const char *shstr = (const char *) (data + sh[eh->e_shstrndx].sh_offset);

    Elf *elf = Elf_New(eh->e_type, eh->e_machine);
    elf->elf_entry = eh->e_entry;

    // Phase: reconstruct the model's own sections (skip the synthesized ones).
    Elf_Sec **secmap = calloc(shnum ? shnum : 1, sizeof(*secmap));
    for (int i = 1; i < shnum; i++) {
        if (sh[i].sh_type != ELF_SHT_PROGBITS && sh[i].sh_type != ELF_SHT_NOBITS) {
            continue;
        }
        const char *name = shstr + sh[i].sh_name;
        Elf_Sec *sec = Elf_Section_Add(elf, name, sh[i].sh_type, sh[i].sh_flags);
        sec->sec_addr      = sh[i].sh_addr;
        sec->sec_addralign = sh[i].sh_addralign ? sh[i].sh_addralign : 1;
        sec->sec_entsize   = sh[i].sh_entsize;
        // A NOBITS section carries no bytes, only the space it asks for.
        if (sh[i].sh_type == ELF_SHT_NOBITS) {
            Elf_Buffer_Zero(&sec->sec_data, sh[i].sh_size);
        } else {
            Elf_Buffer_Data(&sec->sec_data, data + sh[i].sh_offset, sh[i].sh_size);
        }
        secmap[i] = sec;
    }

    // Phase: rebuild the symbol table, resolving names and defining sections.
    int nsyms = 0;
    const char *symstr = NULL;
    const Elf64_Sym *syms = NULL;
    for (int i = 0; i < shnum; i++) {
        if (sh[i].sh_type == ELF_SHT_SYMTAB) {
            syms   = (const Elf64_Sym *) (data + sh[i].sh_offset);
            nsyms  = sh[i].sh_size / sizeof(Elf64_Sym);
            symstr = (const char *) (data + sh[sh[i].sh_link].sh_offset);
            break;
        }
    }

    Elf_Sym **symmap = calloc(nsyms ? nsyms : 1, sizeof(*symmap));
    for (int i = 1; i < nsyms; i++) {
        const Elf64_Sym *sym  = &syms[i];
        const char      *name = symstr + sym->st_name;
        Elf_Sec         *sec  = (sym->st_shndx != ELF_SHN_UNDEF && sym->st_shndx < shnum)
                                    ? secmap[sym->st_shndx] : NULL;
        symmap[i] = Elf_Symbol_Add(elf, name, sec, sym->st_value, ELF_ST_BIND(sym->st_info), ELF_ST_TYPE(sym->st_info));
        symmap[i]->sym_size  = sym->st_size;
        symmap[i]->sym_other = sym->st_other;
    }

    // Phase: rebuild relocations, attaching each to the section it patches.
    for (int i = 0; i < shnum; i++) {
        if (sh[i].sh_type != ELF_SHT_RELA) {
            continue;
        }
        Elf_Sec *target = (sh[i].sh_info < (uint32_t) shnum) ? secmap[sh[i].sh_info] : NULL;
        if (! target) {
            continue;
        }
        int nrel = sh[i].sh_size / sizeof(Elf64_Rela);
        const Elf64_Rela *rela = (const Elf64_Rela *) (data + sh[i].sh_offset);
        for (int r = 0; r < nrel; r++) {
            uint32_t si = ELF_R_SYM(rela[r].r_info);
            Elf_Sym *sym = (si < (uint32_t) nsyms) ? symmap[si] : NULL;
            Elf_Rela_Add(target, rela[r].r_offset, sym, ELF_R_TYPE(rela[r].r_info), rela[r].r_addend);
        }
    }

    free(secmap);
    free(symmap);
    return elf;
}

// Parse an ELF file into a new object, or return NULL on error.
Elf *Elf_Read_Path(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (! file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *buf = malloc(size > 0 ? size : 1);
    if (fread(buf, 1, size, file) != (size_t) size) {
        free(buf);
        fclose(file);
        return NULL;
    }
    fclose(file);

    Elf *elf = Elf_Read_Mem(buf, size);
    free(buf);
    return elf;
}
