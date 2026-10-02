/*
 * C source file for ELF objects and executables.
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
#include "util/object/elf.h"

// Initialize an empty byte buffer.
void Elf_BufferInit(Elf_Buffer *buf)
{
    buf->eb_data = NULL;
    buf->eb_len  = 0;
    buf->eb_cap  = 0;
}

// Free a buffer's storage and clear it.
void Elf_BufferFree(Elf_Buffer *buf)
{
    free(buf->eb_data);
    buf->eb_data = NULL;
    buf->eb_len  = 0;
    buf->eb_cap  = 0;
}

// Grow a buffer so it can hold at least n more bytes.
void Elf_BufferReserve(Elf_Buffer *buf, size_t n)
{
    if (buf->eb_len + n <= buf->eb_cap) {
        return;
    }
    size_t cap = buf->eb_cap ? buf->eb_cap : 256;
    while (buf->eb_len + n > cap) {
        cap *= 2;
    }
    buf->eb_data = realloc(buf->eb_data, cap);
    buf->eb_cap  = cap;
}

// Return a pointer to byte off within a buffer, for in-place patching.
void *Elf_BufferAt(Elf_Buffer *buf, size_t off)
{
    return buf->eb_data + off;
}

// Append one byte, returning the offset it began at.
size_t Elf_BufferByte(Elf_Buffer *buf, uint8_t value)
{
    size_t off = buf->eb_len;
    Elf_BufferReserve(buf, 1);
    buf->eb_data[buf->eb_len++] = value;
    return off;
}

// Append n raw bytes, returning the offset they began at.
size_t Elf_BufferData(Elf_Buffer *buf, const void *data, size_t n)
{
    size_t off = buf->eb_len;
    Elf_BufferReserve(buf, n);
    memcpy(buf->eb_data + buf->eb_len, data, n);
    buf->eb_len += n;
    return off;
}

// Append a little-endian 16-bit value, returning its offset.
size_t Elf_BufferU16(Elf_Buffer *buf, uint16_t value)
{
    uint8_t bytes[2] = { value & 0xFF, (value >> 8) & 0xFF };
    return Elf_BufferData(buf, bytes, 2);
}

// Append a little-endian 32-bit value, returning its offset.
size_t Elf_BufferU32(Elf_Buffer *buf, uint32_t value)
{
    uint8_t bytes[4];
    for (size_t i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Elf_BufferData(buf, bytes, 4);
}

// Append a little-endian 64-bit value, returning its offset.
size_t Elf_BufferU64(Elf_Buffer *buf, uint64_t value)
{
    uint8_t bytes[8];
    for (size_t i = 0; i < sizeof(bytes); i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Elf_BufferData(buf, bytes, 8);
}

// Append n zero bytes, returning the offset they began at.
size_t Elf_BufferZero(Elf_Buffer *buf, size_t n)
{
    size_t off = buf->eb_len;
    Elf_BufferReserve(buf, n);
    memset(buf->eb_data + buf->eb_len, 0, n);
    buf->eb_len += n;
    return off;
}

// Pad the buffer with zeros up to a multiple of align, returning new length.
size_t Elf_BufferAlign(Elf_Buffer *buf, size_t align)
{
    if (align > 1) {
        while (buf->eb_len % align != 0) {
            Elf_BufferByte(buf, 0);
        }
    }
    return buf->eb_len;
}

// Intern a name into the object's string pool, returning an owned copy.
const char *Elf_Intern(Elf *elf, const char *name)
{
    if (! name) {
        name = "";
    }
    char *copy = malloc(strlen(name) + 1);
    strcpy(copy, name);
    if (elf->elf_npool == elf->elf_cappool) {
        elf->elf_cappool = elf->elf_cappool ? elf->elf_cappool * 2 : 16;
        elf->elf_pool = realloc(elf->elf_pool, elf->elf_cappool * sizeof(*elf->elf_pool));
    }
    elf->elf_pool[elf->elf_npool++] = copy;
    return copy;
}

// Create an empty ELF object of the given type and machine.
Elf *Elf_New(uint16_t type, uint16_t machine)
{
    Elf *elf = calloc(1, sizeof(*elf));
    elf->elf_type    = type;
    elf->elf_machine = machine;
    return elf;
}

// Free an ELF object and everything it owns.
void Elf_Free(Elf *elf)
{
    if (! elf) {
        return;
    }
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        Elf_BufferFree(&elf->elf_secs[i]->sec_data);
        free(elf->elf_secs[i]->sec_relas);
        free(elf->elf_secs[i]);
    }
    free(elf->elf_secs);
    for (size_t i = 0; i < elf->elf_nsyms; i++) {
        free(elf->elf_syms[i]);
    }
    free(elf->elf_syms);
    for (size_t i = 0; i < elf->elf_npool; i++) {
        free(elf->elf_pool[i]);
    }
    free(elf->elf_pool);
    free(elf);
}

// Set the entry virtual address (ET_EXEC).
void Elf_SetEntry(Elf *elf, uint64_t vaddr)
{
    elf->elf_entry = vaddr;
}

// Set the object type (ELF_ET_*).
void Elf_SetType(Elf *elf, uint16_t type)
{
    elf->elf_type = type;
}

// Return the object type (ELF_ET_*).
uint16_t Elf_GetType(const Elf *elf)
{
    return elf->elf_type;
}

// Return the last error message recorded on the object.
const char *Elf_Error(const Elf *elf)
{
    return elf->elf_err;
}

// Append a new section and return it.
Elf_Sec *Elf_SectionAdd(Elf *elf, const char *name, uint32_t type, uint64_t flags)
{
    Elf_Sec *sec = calloc(1, sizeof(*sec));
    sec->sec_name      = Elf_Intern(elf, name);
    sec->sec_type      = type;
    sec->sec_flags     = flags;
    sec->sec_addralign = 1;
    Elf_BufferInit(&sec->sec_data);

    if (elf->elf_nsecs == elf->elf_capsecs) {
        elf->elf_capsecs = elf->elf_capsecs ? elf->elf_capsecs * 2 : 8;
        elf->elf_secs = realloc(elf->elf_secs, elf->elf_capsecs * sizeof(*elf->elf_secs));
    }
    elf->elf_secs[elf->elf_nsecs++] = sec;
    return sec;
}

// Find a section by name.
Elf_Sec *Elf_SectionFind(Elf *elf, const char *name)
{
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        if (strcmp(elf->elf_secs[i]->sec_name, name) == 0) {
            return elf->elf_secs[i];
        }
    }
    return NULL;
}

// Find a section by name, creating it with the given type and flags if absent.
Elf_Sec *Elf_SectionGet(Elf *elf, const char *name, uint32_t type, uint64_t flags)
{
    Elf_Sec *sec = Elf_SectionFind(elf, name);
    if (sec) {
        return sec;
    }
    return Elf_SectionAdd(elf, name, type, flags);
}

// Return the number of sections.
size_t Elf_SectionCount(const Elf *elf)
{
    return elf->elf_nsecs;
}

// Return section i.
Elf_Sec *Elf_SectionAt(const Elf *elf, size_t i)
{
    return elf->elf_secs[i];
}

// Return the byte buffer a section's contents are appended to.
Elf_Buffer *Elf_SectionData(Elf_Sec *sec)
{
    return &sec->sec_data;
}

// Place a section at a load address.
void Elf_SectionAddr(Elf_Sec *sec, uint64_t addr)
{
    sec->sec_addr = addr;
}

// Append a symbol and return it.
Elf_Sym *Elf_SymbolAdd(Elf *elf, const char *name, Elf_Sec *sec, uint64_t value, uint8_t bind, uint8_t type)
{
    Elf_Sym *sym = calloc(1, sizeof(*sym));
    sym->sym_name  = Elf_Intern(elf, name);
    sym->sym_sec   = sec;
    sym->sym_value = value;
    sym->sym_bind  = bind;
    sym->sym_type  = type;

    if (elf->elf_nsyms == elf->elf_capsyms) {
        elf->elf_capsyms = elf->elf_capsyms ? elf->elf_capsyms * 2 : 16;
        elf->elf_syms = realloc(elf->elf_syms, elf->elf_capsyms * sizeof(*elf->elf_syms));
    }
    elf->elf_syms[elf->elf_nsyms++] = sym;
    return sym;
}

// Find a symbol by name.
Elf_Sym *Elf_SymbolFind(Elf *elf, const char *name)
{
    for (size_t i = 0; i < elf->elf_nsyms; i++) {
        if (strcmp(elf->elf_syms[i]->sym_name, name) == 0) {
            return elf->elf_syms[i];
        }
    }
    return NULL;
}

// Return the number of symbols.
size_t Elf_SymbolCount(const Elf *elf)
{
    return elf->elf_nsyms;
}

// Return symbol i.
Elf_Sym *Elf_SymbolAt(const Elf *elf, size_t i)
{
    return elf->elf_syms[i];
}

// Append a relocation to the section it patches and return it.
Elf_Rela *Elf_RelaAdd(Elf_Sec *target, uint64_t offset, Elf_Sym *sym, uint32_t type, int64_t addend)
{
    if (target->sec_nrelas == target->sec_caprelas) {
        target->sec_caprelas = target->sec_caprelas ? target->sec_caprelas * 2 : 8;
        target->sec_relas = realloc(target->sec_relas, target->sec_caprelas * sizeof(*target->sec_relas));
    }
    Elf_Rela *rela = &target->sec_relas[target->sec_nrelas++];
    rela->rela_offset = offset;
    rela->rela_sym    = sym;
    rela->rela_type   = type;
    rela->rela_addend = addend;
    return rela;
}

// Return the number of relocations patching a section.
size_t Elf_RelaCount(const Elf_Sec *target)
{
    return target->sec_nrelas;
}

// Return relocation i of a section.
Elf_Rela *Elf_RelaAt(const Elf_Sec *target, size_t i)
{
    return (Elf_Rela *) &target->sec_relas[i];
}

// Validate the file header and return it.
const Elf64_Ehdr *Elf_ReadEhdr(const uint8_t *data, size_t n)
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

// Parse ELF bytes into a new object.
Elf *Elf_ReadMem(const void *buf, size_t n)
{
    const uint8_t    *data = buf;
    const Elf64_Ehdr *eh = Elf_ReadEhdr(data, n);
    if (! eh) {
        return NULL;
    }
    uint16_t shnum = eh->e_shnum;
    const Elf64_Shdr *sh = (const Elf64_Shdr *) (data + eh->e_shoff);
    const char *shstr = (const char *) (data + sh[eh->e_shstrndx].sh_offset);

    Elf *elf = Elf_New(eh->e_type, eh->e_machine);
    elf->elf_entry = eh->e_entry;

    // Phase: reconstruct the model's own sections (skip the synthesized ones).
    Elf_Sec **secmap = calloc(shnum ? shnum : 1, sizeof(*secmap));
    for (uint16_t i = 1; i < shnum; i++) {
        if (sh[i].sh_type != ELF_SHT_PROGBITS && sh[i].sh_type != ELF_SHT_NOBITS) {
            continue;
        }
        const char *name = shstr + sh[i].sh_name;
        Elf_Sec *sec = Elf_SectionAdd(elf, name, sh[i].sh_type, sh[i].sh_flags);
        sec->sec_addr      = sh[i].sh_addr;
        sec->sec_addralign = sh[i].sh_addralign ? sh[i].sh_addralign : 1;
        sec->sec_entsize   = sh[i].sh_entsize;
        if (sh[i].sh_type == ELF_SHT_NOBITS) {
            Elf_BufferZero(&sec->sec_data, sh[i].sh_size);
        } else {
            Elf_BufferData(&sec->sec_data, data + sh[i].sh_offset, sh[i].sh_size);
        }
        secmap[i] = sec;
    }

    // Phase: rebuild the symbol table.
    size_t nsyms = 0;
    const char *symstr = NULL;
    const Elf64_Sym *syms = NULL;
    for (uint16_t i = 0; i < shnum; i++) {
        if (sh[i].sh_type == ELF_SHT_SYMTAB) {
            syms   = (const Elf64_Sym *) (data + sh[i].sh_offset);
            nsyms  = sh[i].sh_size / sizeof(Elf64_Sym);
            symstr = (const char *) (data + sh[sh[i].sh_link].sh_offset);
            break;
        }
    }

    Elf_Sym **symmap = calloc(nsyms ? nsyms : 1, sizeof(*symmap));
    for (size_t i = 1; i < nsyms; i++) {
        const Elf64_Sym *sym = &syms[i];
        const char      *name = symstr + sym->st_name;
        Elf_Sec *sec = (sym->st_shndx != ELF_SHN_UNDEF && sym->st_shndx < shnum)
                                    ? secmap[sym->st_shndx] : NULL;
        symmap[i] = Elf_SymbolAdd(elf, name, sec, sym->st_value, ELF_ST_BIND(sym->st_info), ELF_ST_TYPE(sym->st_info));
        symmap[i]->sym_size  = sym->st_size;
        symmap[i]->sym_other = sym->st_other;
    }

    // Phase: rebuild relocations, attaching each to the section it patches.
    for (uint16_t i = 0; i < shnum; i++) {
        if (sh[i].sh_type != ELF_SHT_RELA) {
            continue;
        }
        Elf_Sec *target = (sh[i].sh_info < (uint32_t) shnum) ? secmap[sh[i].sh_info] : NULL;
        if (! target) {
            continue;
        }
        size_t nrel = sh[i].sh_size / sizeof(Elf64_Rela);
        const Elf64_Rela *rela = (const Elf64_Rela *) (data + sh[i].sh_offset);
        for (size_t r = 0; r < nrel; r++) {
            uint32_t si = ELF_R_SYM(rela[r].r_info);
            Elf_Sym *sym = (si < nsyms) ? symmap[si] : NULL;
            Elf_RelaAdd(target, rela[r].r_offset, sym, ELF_R_TYPE(rela[r].r_info), rela[r].r_addend);
        }
    }

    free(secmap);
    free(symmap);
    return elf;
}

// Copy out section header i when it and its contents lie within the object.
bool Elf_ReadShdr(const uint8_t *data, size_t n, size_t i, Elf64_Shdr *sh)
{
    const Elf64_Ehdr *eh = Elf_ReadEhdr(data, n);
    if (! eh || eh->e_ident[ELF_EI_CLASS] != ELF_CLASS64 || eh->e_shentsize != sizeof(*sh) || i >= eh->e_shnum) {
        return false;
    }
    if (eh->e_shoff > n || eh->e_shnum > (n - eh->e_shoff) / sizeof(*sh)) {
        return false;
    }
    memcpy(sh, data + eh->e_shoff + i * sizeof(*sh), sizeof(*sh));
    return sh->sh_offset <= n && sh->sh_size <= n - sh->sh_offset;
}

// Find the symbol table of an object.
bool Elf_ReadSymtab(const uint8_t *data, size_t n, Elf64_Shdr *symtab)
{
    const Elf64_Ehdr *eh = Elf_ReadEhdr(data, n);
    size_t nsecs = eh ? eh->e_shnum : 0;
    for (size_t i = 0; i < nsecs; i++) {
        if (Elf_ReadShdr(data, n, i, symtab) && symtab->sh_type == ELF_SHT_SYMTAB) {
            return true;
        }
    }
    return false;
}

// List the names of the global and weak symbols an object defines.
const char **Elf_ReadGlobals(const uint8_t *data, size_t n)
{
    size_t count = 0;
    Elf64_Shdr symtab;
    Elf64_Shdr strtab;

    if (! Elf_ReadSymtab(data, n, &symtab) || ! Elf_ReadShdr(data, n, symtab.sh_link, &strtab)) {
        return calloc(1, sizeof(const char *));
    }

    size_t nsyms = symtab.sh_size / sizeof(Elf64_Sym);
    const char *strs = (const char *) data + strtab.sh_offset;
    const char **names = calloc(nsyms + 1, sizeof(*names));
    for (size_t i = 1; i < nsyms; i++) {
        Elf64_Sym sym;
        memcpy(&sym, data + symtab.sh_offset + i * sizeof(sym), sizeof(sym));
        uint8_t bind = ELF_ST_BIND(sym.st_info);
        if ((bind != ELF_BIND_GLOBAL && bind != ELF_BIND_WEAK) || sym.st_shndx == ELF_SHN_UNDEF) {
            continue;
        }
        if (sym.st_name >= strtab.sh_size || ! memchr(strs + sym.st_name, '\0', strtab.sh_size - sym.st_name)) {
            continue;
        }
        names[count++] = strs + sym.st_name;
    }
    return names;
}

// Read the whole file at path.
uint8_t *Elf_ReadBytes(const char *path, size_t *len)
{
    FILE *file = fopen(path, "rb");
    if (! file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size < 0) {
        fclose(file);
        return NULL;
    }

    uint8_t *buf = malloc((size_t) size);

    if (fread(buf, 1, (size_t) size, file) != (size_t) size) {
        free(buf);
        fclose(file);
        return NULL;
    }
    fclose(file);

    *len = (size_t) size;
    return buf;
}

// Parse an ELF file into a new object.
Elf *Elf_ReadPath(const char *path)
{
    size_t size = 0;
    uint8_t *buf = Elf_ReadBytes(path, &size);

    if (! buf) {
        return NULL;
    }

    Elf *elf = Elf_ReadMem(buf, size);

    free(buf);
    return elf;
}

// Append name and a NUL to a string table, returning name's start offset.
uint32_t Elf_WriteStr(Elf_Buffer *strtab, const char *name)
{
    uint32_t off = (uint32_t) strtab->eb_len;
    Elf_BufferData(strtab, name, strlen(name) + 1);
    return off;
}

// Position of a section within the object, used to fill in section indices.
uint32_t Elf_WriteSectionIndex(const Elf *elf, const Elf_Sec *sec, const uint32_t *secidx)
{
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        if (elf->elf_secs[i] == sec) {
            return secidx[i];
        }
    }
    return 0;
}

// Build the .symtab and .strtab bodies, locals before globals.
void Elf_WriteSymtab(const Elf *elf, const uint32_t *secidx, Elf_Buffer *symtab, Elf_Buffer *strtab, uint32_t *slot, uint32_t *first_global)
{
    Elf64_Sym null = {0};
    Elf_BufferData(symtab, &null, sizeof(null));
    Elf_BufferByte(strtab, 0);

    uint32_t si = 1;
    for (int32_t pass = 0; pass < 2; pass++) {
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
                .st_name  = Elf_WriteStr(strtab, sym->sym_name),
                .st_info  = ELF_ST_INFO(sym->sym_bind, sym->sym_type),
                .st_other = sym->sym_other,
                .st_size  = sym->sym_size
            };
            if (sym->sym_sec) {
                out.st_shndx = Elf_WriteSectionIndex(elf, sym->sym_sec, secidx);
                out.st_value = sym->sym_value;
            } else {
                out.st_shndx = ELF_SHN_UNDEF;
            }
            Elf_BufferData(symtab, &out, sizeof(out));
        }
    }
}

// Build one .rela.* body from a section's relocations, using final indices.
void Elf_WriteRelas(const Elf_Sec *sec, const uint32_t *slot, const Elf *elf, Elf_Buffer *out)
{
    for (size_t r = 0; r < sec->sec_nrelas; r++) {
        uint32_t symi = 0;
        Elf_Rela *rela = &sec->sec_relas[r];
        for (size_t i = 0; i < elf->elf_nsyms; i++) {
            if (elf->elf_syms[i] == rela->rela_sym) {
                symi = slot[i];
                break;
            }
        }
        Elf64_Rela disk = {
            .r_offset = rela->rela_offset,
            .r_info   = ELF_R_INFO(symi, rela->rela_type),
            .r_addend = rela->rela_addend
        };
        Elf_BufferData(out, &disk, sizeof(disk));
    }
}

// Serialize a relocatable object (ET_REL).
bool Elf_WriteRel(const Elf *elf, FILE *out)
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
    Elf_BufferInit(&symtab);
    Elf_BufferInit(&strtab);
    Elf_BufferInit(&shstr);
    Elf_BufferByte(&shstr, 0);
    Elf_WriteSymtab(elf, secidx, &symtab, &strtab, slot, &first_global);

    Elf_Buffer *relas = calloc(nuser ? nuser : 1, sizeof(*relas));
    for (size_t i = 0; i < nuser; i++) {
        Elf_BufferInit(&relas[i]);
        if (relaidx[i]) {
            Elf_WriteRelas(elf->elf_secs[i], slot, elf, &relas[i]);
        }
    }

    // Phase: lay out section headers and their bodies.
    Elf64_Shdr *shdrs = calloc(shnum, sizeof(*shdrs));
    uint64_t *sizes = calloc(shnum, sizeof(*sizes));
    const void **bodies = calloc(shnum, sizeof(*bodies));

    for (size_t i = 0; i < nuser; i++) {
        Elf_Sec *sec = elf->elf_secs[i];
        shdrs[secidx[i]] = (Elf64_Shdr) {
            .sh_name      = Elf_WriteStr(&shstr, sec->sec_name),
            .sh_type      = sec->sec_type,
            .sh_flags     = sec->sec_flags,
            .sh_size      = sec->sec_data.eb_len,
            .sh_addralign = sec->sec_addralign,
            .sh_entsize   = sec->sec_entsize
        };
        bodies[secidx[i]] = sec->sec_data.eb_data;
        sizes[secidx[i]]  = sec->sec_type == ELF_SHT_NOBITS ? 0 : sec->sec_data.eb_len;
    }

    shdrs[idx_symtab] = (Elf64_Shdr) {
        .sh_name      = Elf_WriteStr(&shstr, ".symtab"),
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
        .sh_name      = Elf_WriteStr(&shstr, ".strtab"),
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
        char *name = malloc(strlen(".rela") + strlen(elf->elf_secs[i]->sec_name) + 1);
        strcpy(name, ".rela");
        strcat(name, elf->elf_secs[i]->sec_name);
        shdrs[relaidx[i]] = (Elf64_Shdr) {
            .sh_name      = Elf_WriteStr(&shstr, name),
            .sh_type      = ELF_SHT_RELA,
            .sh_size      = relas[i].eb_len,
            .sh_link      = idx_symtab,
            .sh_info      = secidx[i],
            .sh_addralign = 8,
            .sh_entsize   = sizeof(Elf64_Rela)
        };
        free(name);
        bodies[relaidx[i]] = relas[i].eb_data;
        sizes[relaidx[i]]  = relas[i].eb_len;
    }

    shdrs[idx_shstrtab] = (Elf64_Shdr) {
        .sh_name      = Elf_WriteStr(&shstr, ".shstrtab"),
        .sh_type      = ELF_SHT_STRTAB,
        .sh_addralign = 1
    };
    bodies[idx_shstrtab] = shstr.eb_data;
    sizes[idx_shstrtab]  = shstr.eb_len;
    shdrs[idx_shstrtab].sh_size = shstr.eb_len;

    // Phase: assign file offsets.
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

    uint64_t pos = 0;
    fwrite(&ehdr, 1, sizeof(ehdr), out);
    pos += sizeof(ehdr);
    for (uint32_t i = 1; i < shnum; i++) {
        while (pos < shdrs[i].sh_offset) {
            fputc(0, out);
            pos++;
        }
        if (sizes[i]) {
            fwrite(bodies[i], 1, sizes[i], out);
        }
        pos += sizes[i];
    }
    while (pos < shoff) {
        fputc(0, out);
        pos++;
    }
    fwrite(shdrs, 1, sizeof(Elf64_Shdr) * shnum, out);

    Elf_BufferFree(&symtab);
    Elf_BufferFree(&strtab);
    Elf_BufferFree(&shstr);
    for (size_t i = 0; i < nuser; i++) {
        Elf_BufferFree(&relas[i]);
    }
    free(relas);
    free(secidx);
    free(relaidx);
    free(slot);
    free(shdrs);
    free(bodies);
    free(sizes);
    return true;
}

// Return the segment permissions a section's flags call for.
uint32_t Elf_WriteSegFlags(const Elf_Sec *sec)
{
    uint32_t flags = ELF_PF_R;
    if (sec->sec_flags & ELF_SHF_EXECINSTR) {
        flags |= ELF_PF_X;
    }
    if (sec->sec_flags & ELF_SHF_WRITE) {
        flags |= ELF_PF_W;
    }
    return flags;
}

// Smallest file offset >= pos that is page-congruent with vaddr.
uint64_t Elf_WritePlaceOffset(uint64_t pos, uint64_t vaddr)
{
    return pos + (vaddr - pos) % ELF_PAGE;
}

// Serialize a static executable, one PT_LOAD per placed section.
bool Elf_WriteExec(const Elf *elf, FILE *out)
{
    // Phase: select the loadable sections.
    Elf_Sec **segs = calloc(elf->elf_nsecs ? elf->elf_nsecs : 1, sizeof(*segs));
    uint64_t *offs = calloc(elf->elf_nsecs ? elf->elf_nsecs : 1, sizeof(*offs));
    size_t nseg = 0;
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        Elf_Sec *sec = elf->elf_secs[i];
        if ((sec->sec_flags & ELF_SHF_ALLOC) && sec->sec_data.eb_len) {
            segs[nseg++] = sec;
        }
    }

    // Phase: assign page-congruent file offsets.
    uint64_t pos = sizeof(Elf64_Ehdr) + (uint64_t) nseg * sizeof(Elf64_Phdr);
    for (size_t i = 0; i < nseg; i++) {
        offs[i] = Elf_WritePlaceOffset(pos, segs[i]->sec_addr);
        if (segs[i]->sec_type != ELF_SHT_NOBITS) {
            pos = offs[i] + segs[i]->sec_data.eb_len;
        }
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
    fwrite(&ehdr, 1, sizeof(ehdr), out);
    for (size_t i = 0; i < nseg; i++) {
        bool nobits = segs[i]->sec_type == ELF_SHT_NOBITS;
        Elf64_Phdr phdr = {
            .p_type   = ELF_PT_LOAD,
            .p_flags  = Elf_WriteSegFlags(segs[i]),
            .p_offset = offs[i],
            .p_vaddr  = segs[i]->sec_addr,
            .p_paddr  = segs[i]->sec_addr,
            .p_filesz = nobits ? 0 : segs[i]->sec_data.eb_len,
            .p_memsz  = segs[i]->sec_data.eb_len,
            .p_align  = ELF_PAGE
        };
        fwrite(&phdr, 1, sizeof(phdr), out);
    }
    uint64_t pos2 = sizeof(Elf64_Ehdr) + (uint64_t) nseg * sizeof(Elf64_Phdr);
    for (size_t i = 0; i < nseg; i++) {
        if (segs[i]->sec_type == ELF_SHT_NOBITS) {
            continue;
        }
        while (pos2 < offs[i]) {
            fputc(0, out);
            pos2++;
        }
        fwrite(segs[i]->sec_data.eb_data, 1, segs[i]->sec_data.eb_len, out);
        pos2 += segs[i]->sec_data.eb_len;
    }

    free(segs);
    free(offs);
    return true;
}

// Serialize an object to an open stream.
bool Elf_WriteFile(const Elf *elf, FILE *out)
{
    if (elf->elf_type == ELF_ET_EXEC) {
        return Elf_WriteExec(elf, out);
    }
    return Elf_WriteRel(elf, out);
}

// Serialize an object to a file.
bool Elf_WritePath(const Elf *elf, const char *path)
{
    FILE *out = fopen(path, "wb");

    if (! out) {
        return false;
    }

    bool ok = Elf_WriteFile(elf, out);

    return fclose(out) == 0 && ok;
}
