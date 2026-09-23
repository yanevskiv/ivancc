#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/file.h"
#include "util/log.h"
#include "util/str.h"
#include "obj/elf.h"
#include "arch/x86_64/rel.h"

// Bytes of stack reserved above the image; nothing here grows one on demand.
#define LOAD_STACK_SIZE 0x100000

// Alignment the SysV ABI requires of %rsp at a call boundary.
#define LOAD_STACK_ALIGN 16

// Initialize an empty byte buffer.
void Elf_Buffer_Init(Elf_Buffer *buf)
{
    buf->eb_data = NULL;
    buf->eb_len  = 0;
    buf->eb_cap  = 0;
}

// Free a buffer's storage and clear it.
void Elf_Buffer_Free(Elf_Buffer *buf)
{
    free(buf->eb_data);
    buf->eb_data = NULL;
    buf->eb_len  = 0;
    buf->eb_cap  = 0;
}

// Grow a buffer so it can hold at least n more bytes.
void Elf_Buffer_Reserve(Elf_Buffer *buf, size_t n)
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
void *Elf_Buffer_At(Elf_Buffer *buf, size_t off)
{
    return buf->eb_data + off;
}

// Append one byte, returning the offset it began at.
size_t Elf_Buffer_Byte(Elf_Buffer *buf, uint8_t value)
{
    size_t off = buf->eb_len;
    Elf_Buffer_Reserve(buf, 1);
    buf->eb_data[buf->eb_len++] = value;
    return off;
}

// Append n raw bytes, returning the offset they began at.
size_t Elf_Buffer_Data(Elf_Buffer *buf, const void *data, size_t n)
{
    size_t off = buf->eb_len;
    Elf_Buffer_Reserve(buf, n);
    memcpy(buf->eb_data + buf->eb_len, data, n);
    buf->eb_len += n;
    return off;
}

// Append a little-endian 16-bit value, returning its offset.
size_t Elf_Buffer_U16(Elf_Buffer *buf, uint16_t value)
{
    uint8_t bytes[2] = { value & 0xFF, (value >> 8) & 0xFF };
    return Elf_Buffer_Data(buf, bytes, 2);
}

// Append a little-endian 32-bit value, returning its offset.
size_t Elf_Buffer_U32(Elf_Buffer *buf, uint32_t value)
{
    uint8_t bytes[4];
    for (int i = 0; i < 4; i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Elf_Buffer_Data(buf, bytes, 4);
}

// Append a little-endian 64-bit value, returning its offset.
size_t Elf_Buffer_U64(Elf_Buffer *buf, uint64_t value)
{
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return Elf_Buffer_Data(buf, bytes, 8);
}

// Append n zero bytes, returning the offset they began at.
size_t Elf_Buffer_Zero(Elf_Buffer *buf, size_t n)
{
    size_t off = buf->eb_len;
    Elf_Buffer_Reserve(buf, n);
    memset(buf->eb_data + buf->eb_len, 0, n);
    buf->eb_len += n;
    return off;
}

// Pad the buffer with zeros up to a multiple of align, returning new length.
size_t Elf_Buffer_Align(Elf_Buffer *buf, size_t align)
{
    if (align > 1) {
        while (buf->eb_len % align != 0) {
            Elf_Buffer_Byte(buf, 0);
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
        Elf_Buffer_Free(&elf->elf_secs[i]->sec_data);
        free(elf->elf_secs[i]->sec_relas);
        free(elf->elf_secs[i]);
    }
    free(elf->elf_secs);
    for (size_t i = 0; i < elf->elf_nsyms; i++) {
        free(elf->elf_syms[i]);
    }
    free(elf->elf_syms);
    for (size_t i = 0; i < elf->elf_npool; i++) {
        Str_Free(elf->elf_pool[i]);
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

// Return the last error message recorded on the object, or NULL.
const char *Elf_Error(const Elf *elf)
{
    return elf->elf_err;
}

// Append a new section and return it.
Elf_Sec *Elf_Section_Add(Elf *elf, const char *name, uint32_t type, uint64_t flags)
{
    Elf_Sec *sec = calloc(1, sizeof(*sec));
    sec->sec_name      = Elf_Intern(elf, name);
    sec->sec_type      = type;
    sec->sec_flags     = flags;
    sec->sec_addralign = 1;
    Elf_Buffer_Init(&sec->sec_data);

    if (elf->elf_nsecs == elf->elf_capsecs) {
        elf->elf_capsecs = elf->elf_capsecs ? elf->elf_capsecs * 2 : 8;
        elf->elf_secs = realloc(elf->elf_secs, elf->elf_capsecs * sizeof(*elf->elf_secs));
    }
    elf->elf_secs[elf->elf_nsecs++] = sec;
    return sec;
}

// Find a section by name, or return NULL.
Elf_Sec *Elf_Section_Find(Elf *elf, const char *name)
{
    for (size_t i = 0; i < elf->elf_nsecs; i++) {
        if (strcmp(elf->elf_secs[i]->sec_name, name) == 0) {
            return elf->elf_secs[i];
        }
    }
    return NULL;
}

// Find a section by name, creating it with the given type and flags if absent.
Elf_Sec *Elf_Section_Get(Elf *elf, const char *name, uint32_t type, uint64_t flags)
{
    Elf_Sec *sec = Elf_Section_Find(elf, name);
    if (sec) {
        return sec;
    }
    return Elf_Section_Add(elf, name, type, flags);
}

// Return the number of sections.
size_t Elf_Section_Count(const Elf *elf)
{
    return elf->elf_nsecs;
}

// Return section i.
Elf_Sec *Elf_Section_At(const Elf *elf, size_t i)
{
    return elf->elf_secs[i];
}

// Return the byte buffer a section's contents are appended to.
Elf_Buffer *Elf_Section_Data(Elf_Sec *sec)
{
    return &sec->sec_data;
}

// Place a section at a load address.
void Elf_Section_Addr(Elf_Sec *sec, uint64_t addr)
{
    sec->sec_addr = addr;
}

// Append a symbol and return it.  sec == NULL records an undefined reference.
Elf_Sym *Elf_Symbol_Add(Elf *elf, const char *name, Elf_Sec *sec, uint64_t value, uint8_t bind, uint8_t type)
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

// Find a symbol by name, or return NULL.
Elf_Sym *Elf_Symbol_Find(Elf *elf, const char *name)
{
    for (size_t i = 0; i < elf->elf_nsyms; i++) {
        if (strcmp(elf->elf_syms[i]->sym_name, name) == 0) {
            return elf->elf_syms[i];
        }
    }
    return NULL;
}

// Return the number of symbols.
size_t Elf_Symbol_Count(const Elf *elf)
{
    return elf->elf_nsyms;
}

// Return symbol i.
Elf_Sym *Elf_Symbol_At(const Elf *elf, size_t i)
{
    return elf->elf_syms[i];
}

// Append a relocation to the section it patches and return it.
Elf_Rela *Elf_Rela_Add(Elf_Sec *target, uint64_t offset, Elf_Sym *sym, uint32_t type, int64_t addend)
{
    if (target->sec_nrelas == target->sec_caprelas) {
        target->sec_caprelas = target->sec_caprelas ? target->sec_caprelas * 2 : 8;
        target->sec_relas = realloc(target->sec_relas, target->sec_caprelas * sizeof(*target->sec_relas));
    }
    Elf_Rela *rel = &target->sec_relas[target->sec_nrelas++];
    rel->rel_offset = offset;
    rel->rel_sym    = sym;
    rel->rel_type   = type;
    rel->rel_addend = addend;
    return rel;
}

// Return the number of relocations patching a section.
size_t Elf_Rela_Count(const Elf_Sec *target)
{
    return target->sec_nrelas;
}

// Return relocation i of a section.
Elf_Rela *Elf_Rela_At(const Elf_Sec *target, size_t i)
{
    return (Elf_Rela *) &target->sec_relas[i];
}

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
        sizes[secidx[i]]  = sec->sec_type == ELF_SHT_NOBITS ? 0 : sec->sec_data.eb_len;
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

// Return the segment permissions a section's flags call for.
uint32_t Elf_Write_SegFlags(const Elf_Sec *sec)
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

// Smallest file offset >= pos that is page-congruent with vaddr, as PT_LOAD requires.
uint64_t Elf_Write_PlaceOffset(uint64_t pos, uint64_t vaddr)
{
    return pos + (vaddr - pos) % ELF_PAGE;
}

// Serialize a static executable: one PT_LOAD per placed section, with the permissions it asks for.
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
    fwrite(&ehdr, sizeof(ehdr), 1, out);
    for (int i = 0; i < nseg; i++) {
        int nobits = segs[i]->sec_type == ELF_SHT_NOBITS;
        Elf64_Phdr phdr = {
            .p_type   = ELF_PT_LOAD,
            .p_flags  = Elf_Write_SegFlags(segs[i]),
            .p_offset = offs[i],
            .p_vaddr  = segs[i]->sec_addr,
            .p_paddr  = segs[i]->sec_addr,
            .p_filesz = nobits ? 0 : segs[i]->sec_data.eb_len,
            .p_memsz  = segs[i]->sec_data.eb_len,
            .p_align  = ELF_PAGE
        };
        fwrite(&phdr, sizeof(phdr), 1, out);
    }
    long pos2 = sizeof(Elf64_Ehdr) + (long) nseg * sizeof(Elf64_Phdr);
    for (int i = 0; i < nseg; i++) {
        if (segs[i]->sec_type == ELF_SHT_NOBITS) {
            continue;
        }
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
    uint64_t next = ELF_BASE + ELF_PAGE;
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
            next = (end + ELF_PAGE - 1) / ELF_PAGE * ELF_PAGE;
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

// Round addr down to a multiple of align.
uint64_t Elf_Load_AlignDown(uint64_t addr, uint64_t align)
{
    return addr - addr % align;
}

// Round addr up to a multiple of align.
uint64_t Elf_Load_AlignUp(uint64_t addr, uint64_t align)
{
    return Elf_Load_AlignDown(addr + align - 1, align);
}

// Read an ET_EXEC file into a flat image: every PT_LOAD at its address, a stack above, zeros elsewhere.
int Elf_Load_ReadExec(const char *path, Elf_LoadImage *img)
{
    long len = 0;
    char *file = File_GetContent(path, &len);
    if (! file) {
        return -1;
    }

    const uint8_t *data = (const uint8_t *) file;
    const Elf64_Ehdr *eh = Elf_Read_Ehdr(data, (size_t) len);
    if (! eh) {
        Str_Free(file);
        Log_ShowError("not an ELF file: '%s'", path);
    }
    if (eh->e_ident[4] != ELF_CLASS64 || eh->e_ident[5] != ELF_DATA2LSB) {
        Str_Free(file);
        Log_ShowError("not a 64-bit little-endian ELF file: '%s'", path);
    }
    if (eh->e_type != ELF_ET_EXEC) {
        Str_Free(file);
        Log_ShowError("not an executable: '%s'", path);
    }

    // Phase: the extent of every PT_LOAD, which the image has to cover.
    uint64_t lo = UINT64_MAX;
    uint64_t hi = 0;
    for (int i = 0; i < eh->e_phnum; i++) {
        const Elf64_Phdr *ph = (const Elf64_Phdr *) (data + eh->e_phoff + (uint64_t) i * eh->e_phentsize);
        if (ph->p_type != ELF_PT_LOAD) {
            continue;
        }
        if (ph->p_vaddr < lo) {
            lo = ph->p_vaddr;
        }
        if (ph->p_vaddr + ph->p_memsz > hi) {
            hi = ph->p_vaddr + ph->p_memsz;
        }
    }
    if (lo > hi) {
        Str_Free(file);
        Log_ShowError("no loadable segments in '%s'", path);
    }

    // The stack shares the allocation so that one range covers every access.
    img->li_base    = Elf_Load_AlignDown(lo, ELF_PAGE);
    img->li_size    = Elf_Load_AlignUp(hi, ELF_PAGE) - img->li_base + LOAD_STACK_SIZE;
    img->li_entry   = eh->e_entry;
    img->li_machine = eh->e_machine;
    img->li_stack   = Elf_Load_AlignDown(img->li_base + img->li_size, LOAD_STACK_ALIGN);
    img->li_mem     = calloc(img->li_size, 1);

    // Phase: the bytes themselves, leaving p_memsz beyond p_filesz zeroed.
    for (int i = 0; i < eh->e_phnum; i++) {
        const Elf64_Phdr *ph = (const Elf64_Phdr *) (data + eh->e_phoff + (uint64_t) i * eh->e_phentsize);
        // A segment with no file bytes, such as .bss, has nothing to copy and no file offset worth checking.
        if (ph->p_type != ELF_PT_LOAD || ph->p_filesz == 0) {
            continue;
        }
        if (ph->p_offset + ph->p_filesz > (uint64_t) len) {
            Str_Free(file);
            Log_ShowError("segment runs past the end of '%s'", path);
        }
        memcpy(img->li_mem + (ph->p_vaddr - img->li_base), data + ph->p_offset, ph->p_filesz);
    }

    Str_Free(file);
    return 0;
}

// Return a pointer to size bytes of the image at vaddr, or NULL when that range is not mapped.
void *Elf_Load_At(const Elf_LoadImage *img, uint64_t vaddr, uint64_t size)
{
    if (vaddr < img->li_base || size > img->li_size) {
        return NULL;
    }
    uint64_t off = vaddr - img->li_base;
    if (off > img->li_size - size) {
        return NULL;
    }
    return img->li_mem + off;
}

// Release an image's memory and leave it empty.
void Elf_Load_Free(Elf_LoadImage *img)
{
    free(img->li_mem);
    memset(img, 0, sizeof(*img));
}
